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
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

Rack::Rack(INetworkFetcher &networkFetcher,
           IFileSystem &fileSystem,
           ICacheManager &cacheManager,
           PresetManager &presetManager,
           GearLibrary &gearLibrary)
    : networkFetcher(networkFetcher), fileSystem(fileSystem), cacheManager(cacheManager), presetManager(presetManager), gearLibrary(gearLibrary), numSlots(16), slotWidth(200), slotHeight(150), slotSpacing(10), backgroundColor(juce::Colours::darkgrey), slotBackgroundColor(juce::Colours::lightgrey), slotBorderColor(juce::Colours::black), showSlotNumbers(true), showGrid(false)
{
    // Initialize the rack
    initializeRack();

    // Set component ID for debugging
    setComponentID("Rack");
}

Rack::~Rack()
{
    // Remove all state listeners
    stateListeners.clear();

    // Clear all slots
    clearAllSlots();
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

    // Create initial slots
    for (int i = 0; i < numSlots; ++i)
    {
        createSlot(i);
    }

    // Don't layout slots yet - wait for resized() to be called
    // This ensures the Rack component has proper dimensions

    // Load saved rack state
    loadRackState();
}

int Rack::getSlotHeight(int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(rackSlots.size()))
        return getDefaultSlotHeight();

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (slot == nullptr || slot->isEmpty())
        return getDefaultSlotHeight();

    // If the slot has a gear item with a faceplate image, use the image's height plus padding
    auto gearItem = slot->getGearItem();
    if (gearItem != nullptr && gearItem->faceplateImage.isValid())
    {
        // Calculate a reasonable height based on the faceplate image
        // Use aspect ratio of the image, but constrained to reasonable bounds
        int imageHeight = gearItem->faceplateImage.getHeight();
        int imageWidth = gearItem->faceplateImage.getWidth();

        if (imageHeight > 0 && imageWidth > 0)
        {
            // Calculate what the height would be if the width matched the slot width
            // Add extra padding for controls and slot UI elements
            int effectiveSlotWidth = getWidth() - (2 * slotSpacing);
            int scaledHeight = (imageHeight * effectiveSlotWidth) / imageWidth;

            // Add padding for slot UI elements (buttons, labels, etc.)
            int paddedHeight = scaledHeight + 40; // 20px padding top and bottom

            // Constrain to reasonable bounds
            return juce::jlimit(100, 400, paddedHeight);
        }
    }

    // Default height if no special considerations apply
    return getDefaultSlotHeight();
}

int Rack::getDefaultSlotHeight() const
{
    return 150; // Default height if not overridden
}

void Rack::createSlot(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= numSlots)
        return;

    auto slot = std::make_unique<RackSlot>(fileSystem, cacheManager, presetManager, gearLibrary, slotIndex);
    slot->setComponentID("RackSlot_" + juce::String(slotIndex));

    // Add as component listener to track changes
    slot->addComponentListener(this);

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
    int totalHeight = slotSpacing; // Start with top spacing
    for (int i = 0; i < numSlots; ++i)
    {
        totalHeight += getSlotHeight(i) + slotSpacing;
    }

    // Size the container to fit all slots with spacing
    rackContainer->setSize(containerWidth, totalHeight);

    updateSlotPositions();
}

void Rack::updateSlotPositions()
{
    // Calculate the slot width based on container width minus margins
    int effectiveSlotWidth = getWidth() - (2 * slotSpacing);

    // Position the slots within the container in a single vertical column
    int currentY = slotSpacing;
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            int slotHeight = getSlotHeight(static_cast<int>(i));

            rackSlots[i]->setBounds(
                slotSpacing,        // Left margin
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

    // No grid drawing needed for single-column vertical layout
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
    return description.startsWith("gear:");
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
    juce::String gearId = description.substring(5); // Remove "gear:" prefix

    // Convert drop position to slot index
    juce::Point<int> dropPos = dragSourceDetails.localPosition;
    int slotIndex = getSlotIndexFromPosition(dropPos);

    if (slotIndex >= 0 && static_cast<size_t>(slotIndex) < rackSlots.size())
    {
        // Check if we can drop the gear in this slot
        if (canDropGearInSlot(slotIndex, gearId))
        {
            addGearToSlot(slotIndex, gearId);
        }
    }

    repaint();
}

int Rack::getSlotIndexFromPosition(juce::Point<int> position) const
{
    if (!rackContainer)
        return -1;

    // Convert to container coordinates
    juce::Point<int> containerPos = rackContainer->getLocalPoint(this, position);

    // For vertical layout, find slot based on Y position
    int currentY = slotSpacing;
    for (int i = 0; i < static_cast<int>(rackSlots.size()); ++i)
    {
        int slotHeight = getSlotHeight(i);
        int slotBottom = currentY + slotHeight;

        if (containerPos.y >= currentY && containerPos.y < slotBottom)
        {
            // Check if X position is within slot bounds
            if (containerPos.x >= slotSpacing && containerPos.x < (getWidth() - slotSpacing))
            {
                return i;
            }
        }

        currentY = slotBottom + slotSpacing;
    }

    return -1;
}

bool Rack::canDropGearInSlot(int slotIndex, const juce::String &gearId) const
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return false;

    // Check if slot is empty
    if (isSlotOccupied(slotIndex))
        return false;

    // Check if gear exists in library
    return gearLibrary.gearItemExists(gearId);
}

// Rack Management
void Rack::addRackSlot(int slotIndex)
{
    if (slotIndex < 0)
        return;

    // Ensure we have enough capacity
    while (rackSlots.size() <= static_cast<size_t>(slotIndex))
    {
        createSlot(static_cast<int>(rackSlots.size()));
    }

    layoutSlots();
}

void Rack::removeRackSlot(int slotIndex)
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return;

    // Remove gear from slot first
    removeGearFromSlot(slotIndex);

    // Remove the slot
    rackSlots.erase(rackSlots.begin() + slotIndex);

    // Update slot indices
    for (size_t i = static_cast<size_t>(slotIndex); i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            rackSlots[i]->setIndex(static_cast<int>(i));
        }
    }

    layoutSlots();
}

void Rack::clearAllSlots()
{
    // Remove all gear from slots
    for (auto &slot : rackSlots)
    {
        if (slot)
        {
            slot->clearGearItem();
        }
    }

    // Clear the slots array
    rackSlots.clear();
}

int Rack::getSlotCount() const
{
    return static_cast<int>(rackSlots.size());
}

RackSlot *Rack::getSlot(int slotIndex) const
{
    if (slotIndex >= 0 && static_cast<size_t>(slotIndex) < rackSlots.size())
    {
        return rackSlots[static_cast<size_t>(slotIndex)].get();
    }
    return nullptr;
}

// Gear Management
bool Rack::addGearToSlot(int slotIndex, const juce::String &gearId)
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return false;

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
        return false;

    // Check if slot is already occupied
    if (isSlotOccupied(slotIndex))
        return false;

    // Get gear item from library
    auto gearItem = gearLibrary.getGearItem(gearId);
    if (!gearItem)
        return false;

    // Add gear to slot
    slot->setGearItem(gearItem);

    // Save rack state
    saveRackState();

    // Notify listeners
    notifyStateChanged();

    return true;
}

bool Rack::removeGearFromSlot(int slotIndex)
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return false;

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
        return false;

    slot->clearGearItem();

    // Save rack state
    saveRackState();

    // Notify listeners
    notifyStateChanged();

    return true;
}

bool Rack::moveGearBetweenSlots(int fromSlot, int toSlot)
{
    if (fromSlot < 0 || static_cast<size_t>(fromSlot) >= rackSlots.size() ||
        toSlot < 0 || static_cast<size_t>(toSlot) >= rackSlots.size())
        return false;

    if (fromSlot == toSlot)
        return true;

    // Get gear from source slot
    juce::String gearId = getGearInSlot(fromSlot);
    if (gearId.isEmpty())
        return false;

    // Check if destination slot is empty
    if (isSlotOccupied(toSlot))
        return false;

    // Remove from source slot
    if (!removeGearFromSlot(fromSlot))
        return false;

    // Add to destination slot
    return addGearToSlot(toSlot, gearId);
}

juce::String Rack::getGearInSlot(int slotIndex) const
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return "";

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
        return "";

    auto gearItem = slot->getGearItem();
    if (gearItem)
        return gearItem->unitId;
    return "";
}

// State Persistence
void Rack::saveRackState()
{
    // Create rack state ValueTree
    rackState = juce::ValueTree("Rack");

    // Save slot configuration
    rackState.setProperty("numSlots", numSlots, nullptr);
    rackState.setProperty("slotWidth", slotWidth, nullptr);
    rackState.setProperty("slotHeight", slotHeight, nullptr);

    // Save gear in each slot
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        auto slot = rackSlots[i].get();
        if (slot && isSlotOccupied(static_cast<int>(i)))
        {
            juce::String gearId = getGearInSlot(static_cast<int>(i));
            if (!gearId.isEmpty())
            {
                rackState.setProperty("slot_" + juce::String(static_cast<int>(i)), gearId, nullptr);
            }
        }
    }
}

void Rack::loadRackState()
{
    if (!rackState.isValid())
        return;

    // Load slot configuration
    numSlots = rackState.getProperty("numSlots", 16);
    slotWidth = rackState.getProperty("slotWidth", 200);
    slotHeight = rackState.getProperty("slotHeight", 150);

    // Recreate slots if configuration changed
    if (rackSlots.size() != static_cast<size_t>(numSlots))
    {
        clearAllSlots();
        for (int i = 0; i < numSlots; ++i)
        {
            createSlot(i);
        }
    }

    // Load gear into slots
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        juce::String gearId = rackState.getProperty("slot_" + juce::String(static_cast<int>(i)), "");
        if (!gearId.isEmpty())
        {
            addGearToSlot(static_cast<int>(i), gearId);
        }
    }

    layoutSlots();
}

juce::ValueTree Rack::getRackState() const
{
    return rackState;
}

void Rack::setRackState(const juce::ValueTree &state)
{
    rackState = state;
    loadRackState();
}

// Layout Management
void Rack::setSlotLayout(int newNumSlots)
{
    if (numSlots != newNumSlots)
    {
        numSlots = newNumSlots;

        // Recreate slots
        clearAllSlots();
        for (int i = 0; i < numSlots; ++i)
        {
            createSlot(i);
        }

        layoutSlots();
        saveRackState();
    }
}

void Rack::setSlotSize(int width, int height)
{
    if (slotWidth != width || slotHeight != height)
    {
        slotWidth = width;
        slotHeight = height;
        layoutSlots();
        saveRackState();
    }
}

void Rack::setSlotSpacing(int spacing)
{
    if (slotSpacing != spacing)
    {
        slotSpacing = spacing;
        layoutSlots();
        saveRackState();
    }
}

// Visual Customization
void Rack::setBackgroundColor(juce::Colour color)
{
    backgroundColor = color;
    repaint();
}

void Rack::setSlotBackgroundColor(juce::Colour color)
{
    slotBackgroundColor = color;
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

// Event Handling
void Rack::addRackStateListener(juce::Component *listener)
{
    if (listener && !stateListeners.contains(listener))
    {
        stateListeners.add(listener);
    }
}

void Rack::removeRackStateListener(juce::Component *listener)
{
    stateListeners.removeFirstMatchingValue(listener);
}

void Rack::notifyStateChanged()
{
    // Notify all listeners that rack state has changed
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->repaint();
        }
    }
}

// Utility Methods
bool Rack::isSlotOccupied(int slotIndex) const
{
    if (slotIndex < 0 || static_cast<size_t>(slotIndex) >= rackSlots.size())
        return false;

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
        return false;

    return !slot->isEmpty();
}

int Rack::getFirstEmptySlot() const
{
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (!isSlotOccupied(static_cast<int>(i)))
            return static_cast<int>(i);
    }
    return -1;
}

int Rack::getLastOccupiedSlot() const
{
    for (int i = static_cast<int>(rackSlots.size()) - 1; i >= 0; --i)
    {
        if (isSlotOccupied(i))
            return i;
    }
    return -1;
}

void Rack::compactSlots()
{
    // Find all occupied slots
    std::vector<int> occupiedSlots;
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (isSlotOccupied(static_cast<int>(i)))
        {
            occupiedSlots.push_back(static_cast<int>(i));
        }
    }

    // Sort occupied slots by index
    std::sort(occupiedSlots.begin(), occupiedSlots.end());

    // Move gear to consecutive slots starting from 0
    for (size_t i = 0; i < occupiedSlots.size(); ++i)
    {
        int currentSlot = occupiedSlots[i];
        int targetSlot = static_cast<int>(i);

        if (currentSlot != targetSlot)
        {
            moveGearBetweenSlots(currentSlot, targetSlot);
        }
    }
}

// Component Listener Override
void Rack::componentMovedOrResized(juce::Component &component, bool wasMoved, bool wasResized)
{
    // Check if this is one of our rack slots
    for (auto &slot : rackSlots)
    {
        if (slot.get() == &component)
        {
            // Slot was moved or resized, update state
            saveRackState();
            notifyStateChanged();
            break;
        }
    }
}
