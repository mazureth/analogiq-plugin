/**
 * @file Rack.cpp
 * @brief Implementation of the Rack class.
 *
 * This file implements the Rack class which provides a visual interface for managing
 * audio gear items in a virtual rack. It handles the layout of gear items, drag-and-drop
 * functionality for adding and rearranging items, and manages the loading and display
 * of gear resources like faceplates and controls.
 */

#include "Rack.h"
#include "RackSlot.h"
#include "../Shared/IGearLibrary.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/IPresetManager.h"
#include "../Shared/IRackStateListener.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

Rack::Rack(RackModel &rackModel)
    : rackModel(rackModel), backgroundColor(juce::Colours::darkgrey), slotBackgroundColor(juce::Colours::darkgrey.darker(0.7f)), slotBorderColor(juce::Colours::black), showSlotNumbers(true), showGrid(false)
{
    // Initialize the rack
    initializeRack();

    // Set component ID for debugging
    setComponentID("Rack");
}

Rack::~Rack()
{
    // Clear all UI slots
    rackSlots.clear();
}

void Rack::initializeRack()
{
    // Create viewport
    viewport = std::make_unique<juce::Viewport>();
    addAndMakeVisible(viewport.get());

    // Create rack container
    rackContainer = std::make_unique<juce::Component>();
    rackContainer->setComponentID("RackContainer");
    viewport->setViewedComponent(rackContainer.get(), false);

    // Create UI slots based on RackModel data
    int slotCount = rackModel.getSlotCount();
    for (int i = 0; i < slotCount; ++i)
    {
        createSlot(i);
    }

    // Layout slots
    layoutSlots();
}

// getSlotHeight and getDefaultSlotHeight methods removed - Now handled by RackModel

void Rack::createSlot(int slotIndex)
{
    if (!rackModel.isValidSlotIndex(slotIndex))
        return;

    // Create RackSlot with RackModel reference
    auto slot = std::make_unique<RackSlot>(rackModel, slotIndex);
    slot->setComponentID("RackSlot_" + juce::String(slotIndex));
    slot->setRack(this); // Set reference to parent rack

    // Add as component listener to track changes
    slot->addComponentListener(this);

    // Update button states after setting rack reference
    slot->updateButtonStates();

    // Set up individual faceplate loaded callback for this slot
    slot->setFaceplateLoadedCallback([this, slotIndex]()
                                     { repaintSingleSlot(slotIndex); });

    // Set the slot's background color
    slot->setSlotBackgroundColor(slotBackgroundColor);

    rackContainer->addAndMakeVisible(slot.get());
    rackSlots.push_back(std::move(slot));
}

void Rack::layoutSlots()
{
    if (!rackContainer)
        return;

    // Use full available width for container
    int containerWidth = getWidth();

    // Safety check - don't layout if we don't have a valid width yet
    if (containerWidth <= 0)
        return;

    // Calculate total height needed for all slots with their dynamic heights
    int slotSpacing = rackModel.getSlotSpacing();
    int totalHeight = slotSpacing; // Start with top spacing
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        totalHeight += rackModel.getSlotHeight(static_cast<int>(i)) + slotSpacing;
    }

    // If rack is empty, use the full Rack component height so the placeholder text is fully visible
    if (rackSlots.empty())
    {
        totalHeight = getHeight();
    }

    // Size the container to fit all slots with spacing
    rackContainer->setSize(containerWidth, totalHeight);

    updateSlotPositions();
}

void Rack::updateSlotPositions()
{
    // Calculate the slot width based on container width minus margins (exactly like old system)
    int slotSpacing = rackModel.getSlotSpacing();
    int effectiveSlotWidth = rackModel.getEffectiveSlotWidth(getWidth());

    // Position the slots within the container in a single vertical column
    int currentY = slotSpacing;
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            int slotHeight = rackModel.getSlotHeight(static_cast<int>(i));

            rackSlots[i]->setBounds(
                slotSpacing,        // Left margin (exactly like old system)
                currentY,           // Current Y position
                effectiveSlotWidth, // Full width minus margins
                slotHeight          // Dynamic height for this slot
            );

            currentY += slotHeight + slotSpacing;
        }
    }
}

void Rack::paint(juce::Graphics &g)
{
    // Fill background
    g.fillAll(backgroundColor);

    // If rack is empty, show visual feedback
    if (rackSlots.empty())
    {
        g.setColour(juce::Colours::lightgrey.withAlpha(0.3f));
        g.setFont(16.0f);
        g.drawText("Drop gear items here to add them to the rack",
                   getLocalBounds(),
                   juce::Justification::centred);

        // Draw a subtle border to indicate drop area
        g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
        g.drawRect(getLocalBounds().reduced(10), 2);
    }
}

void Rack::resized()
{
    if (viewport)
    {
        viewport->setBounds(getLocalBounds());
    }

    // Now that we have proper dimensions, layout the slots
    if (rackContainer && getWidth() > 0)
    {
        layoutSlots();
    }
}

// Drag and Drop Implementation
bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{

    // Check if the drag source contains gear data
    juce::String description = dragSourceDetails.description.toString();
    bool interested = description.startsWith("gear:");

    return interested;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{

    // Highlight the rack to show it's a valid drop target
    repaint();
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{

    // Update visual feedback during drag
    repaint();
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{

    // Remove drag feedback
    repaint();
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{

    // Extract gear ID from drag description
    juce::String description = dragSourceDetails.description.toString();

    if (description.startsWith("gear:"))
    {
        juce::String gearId = description.substring(5); // Remove "gear:" prefix

        // Convert drop position to slot index
        juce::Point<int> dropPos = dragSourceDetails.localPosition;
        int slotIndex = getSlotIndexFromPosition(dropPos);

        // For fluid rack, accept any non-negative slot index
        if (slotIndex >= 0)
        {

            // Check if we can drop the gear in this slot
            if (canDropGearInSlot(slotIndex, gearId))
            {
                bool success = rackModel.addGearToSlot(slotIndex, gearId);

                if (success)
                {
                    // Update UI to reflect the change
                    layoutSlots();
                    repaint();
                }
            }
            else
            {
            }
        }
        else
        {
        }
    }
    else
    {
    }

    repaint();
}

int Rack::getSlotIndexFromPosition(juce::Point<int> position) const
{

    if (!rackContainer)
    {
        return 0; // Return 0 for empty rack
    }

    // Convert to container coordinates
    juce::Point<int> containerPos = rackContainer->getLocalPoint(this, position);

    // If rack is empty, return 0 to create first slot
    if (rackSlots.empty())
    {
        return 0;
    }

    // For vertical layout, find insertion point based on Y position
    int slotSpacing = rackModel.getSlotSpacing();
    int currentY = slotSpacing;
    for (int i = 0; i < static_cast<int>(rackSlots.size()); ++i)
    {
        int slotHeight = rackModel.getSlotHeight(i);
        int slotBottom = currentY + slotHeight;
        int slotMiddle = currentY + (slotHeight / 2);

        // Check if X position is within slot bounds
        if (containerPos.x >= slotSpacing && containerPos.x < (getWidth() - slotSpacing))
        {
            if (containerPos.y < slotBottom)
            {
                // Drop anywhere on this slot - always insert before this slot (above it)
                return i;
            }
            else
            {
            }
        }
        else
        {
        }

        currentY = slotBottom + slotSpacing;
    }

    // If we get here, drop is below all existing slots
    return static_cast<int>(rackSlots.size());
}

bool Rack::canDropGearInSlot(int slotIndex, const juce::String &gearId) const
{
    // Use RackModel to validate the slot index
    if (!rackModel.isValidSlotIndex(slotIndex))
    {
        return false;
    }

    // Check if gear exists in library (this would need to be passed through RackModel)
    // For now, we'll assume the gear exists if we get this far
    return true;
}

// Rack Management - Now handled by RackModel

// insertRackSlot - Now handled by RackModel

// removeRackSlot, clearAllSlots - Now handled by RackModel

// Rack slot validation methods - Now handled by RackModel

// All validation and data management methods - Now handled by RackModel

// Gear Management - Now handled by RackModel

// All gear management methods removed - Now handled by RackModel

// All data management methods removed - Now handled by RackModel

// All remaining data management methods removed - Now handled by RackModel

// All remaining data management methods removed - Now handled by RackModel

// State Persistence - Now handled by RackModel

// Preset serialization - Now handled by RackModel

// deserializeRackFromJSON - Now handled by RackModel
// All remaining data management methods removed - Now handled by RackModel

// Visual Customization
void Rack::setBackgroundColor(juce::Colour color)
{
    backgroundColor = color;
    repaint();
}

void Rack::setSlotBackgroundColor(juce::Colour color)
{
    slotBackgroundColor = color;

    // Update all existing slots with the new background color
    for (auto &slot : rackSlots)
    {
        if (slot)
        {
            slot->setSlotBackgroundColor(color);
        }
    }

    repaint();
}

void Rack::setSlotBorderColor(juce::Colour color)
{
    slotBorderColor = color;
    repaint();
}

void Rack::setShowSlotNumbers(bool show)
{
    showSlotNumbers = show;
    repaint();
}

void Rack::setShowGrid(bool show)
{
    showGrid = show;
    repaint();
}

// Event Handling - Now handled by RackModel
// All remaining data management methods removed - Now handled by RackModel

// All remaining data management methods removed - Now handled by RackModel

void Rack::repaintSingleSlot(int slotIndex)
{
    if (rackModel.isValidSlotIndex(slotIndex) && slotIndex < static_cast<int>(rackSlots.size()) && rackSlots[slotIndex])
    {
        rackSlots[slotIndex]->repaint();
    }
}

void Rack::componentMovedOrResized(juce::Component &component, bool wasMoved, bool wasResized)
{
    // Handle component changes - this is called when RackSlot components are moved or resized
    // For now, we don't need to do anything special here as the RackModel handles the data
    // and the UI updates are handled by the normal JUCE component system
}
