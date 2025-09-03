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
    : networkFetcher(networkFetcher), fileSystem(fileSystem), cacheManager(cacheManager), presetManager(presetManager), gearLibrary(gearLibrary), numSlots(0), slotWidth(200), slotHeight(150), slotSpacing(10), backgroundColor(juce::Colours::darkgrey), slotBackgroundColor(juce::Colours::darkgrey.darker(0.7f)), slotBorderColor(juce::Colours::black), showSlotNumbers(true), showGrid(false)
{
    // Initialize the rack
    initializeRack();

    // Set component ID for debugging
    setComponentID("Rack");

    // Note: We no longer use the global library callback for repainting
    // Individual slots will handle their own repaints via callbacks
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

    // Start with empty rack - no initial slots created
    // Slots will be created dynamically when items are dropped

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

    // Set up individual faceplate loaded callback for this slot
    slot->setFaceplateLoadedCallback([this, slotIndex]()
                                     {
        juce::Logger::writeToLog("Rack: Faceplate loaded for slot " + juce::String(slotIndex) + ", repainting single slot");
        repaintSingleSlot(slotIndex); });

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
    int totalHeight = slotSpacing; // Start with top spacing
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        totalHeight += getSlotHeight(static_cast<int>(i)) + slotSpacing;
    }

    // Size the container to fit all slots with spacing
    rackContainer->setSize(containerWidth, totalHeight);

    updateSlotPositions();
}

void Rack::updateSlotPositions()
{
    // Calculate the slot width based on container width minus margins and padding
    int slotPadding = 2; // 2px padding on left and right
    int effectiveSlotWidth = getWidth() - (2 * slotSpacing) - (2 * slotPadding);

    // Position the slots within the container in a single vertical column
    int currentY = slotSpacing;
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            int slotHeight = getSlotHeight(static_cast<int>(i));

            rackSlots[i]->setBounds(
                slotSpacing + slotPadding, // Left margin + padding
                currentY,                  // Current Y position
                effectiveSlotWidth,        // Full width minus margins and padding
                slotHeight                 // Dynamic height for this slot
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
    juce::Logger::writeToLog("Rack: isInterestedInDragSource called");
    juce::Logger::writeToLog("Rack: Drag description: '" + dragSourceDetails.description.toString() + "'");
    juce::Logger::writeToLog("Rack: Source component: " + (dragSourceDetails.sourceComponent ? dragSourceDetails.sourceComponent->getComponentID() : "null"));

    // Check if the drag source contains gear data
    juce::String description = dragSourceDetails.description.toString();
    bool interested = description.startsWith("gear:");

    juce::Logger::writeToLog("Rack: Interested in drag source: " + juce::String(interested ? "YES" : "NO"));
    return interested;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::Logger::writeToLog("Rack: itemDragEnter called");
    juce::Logger::writeToLog("Rack: Drag description: '" + dragSourceDetails.description.toString() + "'");

    // Highlight the rack to show it's a valid drop target
    repaint();
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::Logger::writeToLog("Rack: itemDragMove called at position: " + dragSourceDetails.localPosition.toString());

    // Update visual feedback during drag
    repaint();
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::Logger::writeToLog("Rack: itemDragExit called");

    // Remove drag feedback
    repaint();
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::Logger::writeToLog("Rack: itemDropped called");
    juce::Logger::writeToLog("Rack: Drop position: " + dragSourceDetails.localPosition.toString());
    juce::Logger::writeToLog("Rack: Drag description: '" + dragSourceDetails.description.toString() + "'");
    juce::Logger::writeToLog("Rack: Source component: " + (dragSourceDetails.sourceComponent ? dragSourceDetails.sourceComponent->getComponentID() : "null"));

    // Extract gear ID from drag description
    juce::String description = dragSourceDetails.description.toString();
    juce::Logger::writeToLog("Rack: Full description: '" + description + "'");

    if (description.startsWith("gear:"))
    {
        juce::String gearId = description.substring(5); // Remove "gear:" prefix
        juce::Logger::writeToLog("Rack: Extracted gear ID: '" + gearId + "'");

        // Convert drop position to slot index
        juce::Point<int> dropPos = dragSourceDetails.localPosition;
        int slotIndex = getSlotIndexFromPosition(dropPos);
        juce::Logger::writeToLog("Rack: Calculated slot index: " + juce::String(slotIndex));

        // For fluid rack, accept any non-negative slot index
        if (slotIndex >= 0)
        {
            juce::Logger::writeToLog("Rack: Slot index is valid for fluid rack");

            // Check if we can drop the gear in this slot
            if (canDropGearInSlot(slotIndex, gearId))
            {
                juce::Logger::writeToLog("Rack: Can drop gear in slot, calling addGearToSlot");
                bool success = addGearToSlot(slotIndex, gearId);
                juce::Logger::writeToLog("Rack: addGearToSlot result: " + juce::String(success ? "SUCCESS" : "FAILED"));

                if (success)
                {
                    // Add to recently used
                    cacheManager.addToRecentlyUsed(gearId);
                    juce::Logger::writeToLog("Rack: Added gear to recently used: " + gearId);
                }
            }
            else
            {
                juce::Logger::writeToLog("Rack: Cannot drop gear in slot");
            }
        }
        else
        {
            juce::Logger::writeToLog("Rack: Invalid slot index (negative): " + juce::String(slotIndex));
        }
    }
    else
    {
        juce::Logger::writeToLog("Rack: Description does not start with 'gear:', ignoring drop");
    }

    repaint();
}

int Rack::getSlotIndexFromPosition(juce::Point<int> position) const
{
    juce::Logger::writeToLog("Rack: getSlotIndexFromPosition called with position: " + position.toString());

    if (!rackContainer)
    {
        juce::Logger::writeToLog("Rack: No rack container, returning 0 (empty rack)");
        return 0; // Return 0 for empty rack
    }

    // Convert to container coordinates
    juce::Point<int> containerPos = rackContainer->getLocalPoint(this, position);
    juce::Logger::writeToLog("Rack: Container position: " + containerPos.toString());

    // If rack is empty, return 0 to create first slot
    if (rackSlots.empty())
    {
        juce::Logger::writeToLog("Rack: Empty rack, returning 0 for first slot");
        return 0;
    }

    // For vertical layout, find insertion point based on Y position
    int currentY = slotSpacing;
    for (int i = 0; i < static_cast<int>(rackSlots.size()); ++i)
    {
        int slotHeight = getSlotHeight(i);
        int slotBottom = currentY + slotHeight;
        int slotMiddle = currentY + (slotHeight / 2);

        juce::Logger::writeToLog("Rack: Checking slot " + juce::String(i) + " - Y range: " + juce::String(currentY) + " to " + juce::String(slotBottom) + " (middle: " + juce::String(slotMiddle) + ")");

        // Check if X position is within slot bounds
        if (containerPos.x >= slotSpacing && containerPos.x < (getWidth() - slotSpacing))
        {
            juce::Logger::writeToLog("Rack: X position " + juce::String(containerPos.x) + " is within slot bounds");
            if (containerPos.y < slotBottom)
            {
                // Drop anywhere on this slot - always insert before this slot (above it)
                juce::Logger::writeToLog("Rack: Drop on slot " + juce::String(i) + " (Y=" + juce::String(containerPos.y) + " < bottom=" + juce::String(slotBottom) + "), inserting before it");
                return i;
            }
            else
            {
                juce::Logger::writeToLog("Rack: Drop below slot " + juce::String(i) + " (Y=" + juce::String(containerPos.y) + " > bottom=" + juce::String(slotBottom) + "), continuing to next slot");
            }
        }
        else
        {
            juce::Logger::writeToLog("Rack: X position " + juce::String(containerPos.x) + " is outside slot bounds (spacing=" + juce::String(slotSpacing) + ", width=" + juce::String(getWidth()) + ")");
        }

        currentY = slotBottom + slotSpacing;
    }

    // If we get here, drop is below all existing slots
    juce::Logger::writeToLog("Rack: Drop below all slots, appending at end");
    return static_cast<int>(rackSlots.size());
}

bool Rack::canDropGearInSlot(int slotIndex, const juce::String &gearId) const
{
    juce::Logger::writeToLog("Rack: canDropGearInSlot called for slot " + juce::String(slotIndex) + " and gear ID: '" + gearId + "'");

    // For fluid rack, we can create slots at any valid index
    if (slotIndex < 0)
    {
        juce::Logger::writeToLog("Rack: Invalid slot index (negative): " + juce::String(slotIndex));
        return false;
    }

    // For fluid rack, we can always create slots at any index
    // The only constraint is that the gear must exist in the library
    juce::Logger::writeToLog("Rack: Slot index " + juce::String(slotIndex) + " is valid for fluid rack");

    // Check if gear exists in library
    bool gearExists = gearLibrary.gearItemExists(gearId);
    juce::Logger::writeToLog("Rack: Gear exists in library: " + juce::String(gearExists ? "YES" : "NO"));

    return gearExists;
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

void Rack::insertRackSlot(int slotIndex)
{
    if (slotIndex < 0)
        return;

    juce::Logger::writeToLog("Rack: insertRackSlot called for index: " + juce::String(slotIndex));

    // Create a new slot at the specified index
    auto newSlot = std::make_unique<RackSlot>(fileSystem, cacheManager, presetManager, gearLibrary, slotIndex);
    newSlot->setComponentID("RackSlot_" + juce::String(slotIndex));
    newSlot->addComponentListener(this);

    // Set up individual faceplate loaded callback for this slot
    newSlot->setFaceplateLoadedCallback([this, slotIndex]()
                                        {
        juce::Logger::writeToLog("Rack: Faceplate loaded for slot " + juce::String(slotIndex) + ", repainting single slot");
        repaintSingleSlot(slotIndex); });

    // Set the slot's background color
    newSlot->setSlotBackgroundColor(slotBackgroundColor);

    // Insert the slot at the specified position
    if (static_cast<size_t>(slotIndex) >= rackSlots.size())
    {
        // Append to end
        rackSlots.push_back(std::move(newSlot));
        juce::Logger::writeToLog("Rack: Appended slot at end, total slots: " + juce::String(rackSlots.size()));
    }
    else
    {
        // Insert at specified position
        rackSlots.insert(rackSlots.begin() + slotIndex, std::move(newSlot));
        juce::Logger::writeToLog("Rack: Inserted slot at position " + juce::String(slotIndex) + ", total slots: " + juce::String(rackSlots.size()));

        // Reindex all slots after the insertion point
        for (size_t i = static_cast<size_t>(slotIndex) + 1; i < rackSlots.size(); ++i)
        {
            if (rackSlots[i])
            {
                rackSlots[i]->setIndex(static_cast<int>(i));
                rackSlots[i]->setComponentID("RackSlot_" + juce::String(static_cast<int>(i)));
            }
        }
    }

    // Add the new slot to the container
    if (rackSlots[static_cast<size_t>(slotIndex)])
    {
        rackContainer->addAndMakeVisible(rackSlots[static_cast<size_t>(slotIndex)].get());
    }

    // Don't call layoutSlots() here - it will be called after faceplates finish loading
    // This prevents the race condition where slots are positioned with incorrect heights
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
    juce::Logger::writeToLog("Rack: addGearToSlot called for slot " + juce::String(slotIndex) + " and gear ID: '" + gearId + "'");

    if (slotIndex < 0)
        return false;

    // Get gear item from library
    auto gearItem = gearLibrary.getGearItem(gearId);
    if (!gearItem)
    {
        juce::Logger::writeToLog("Rack: ERROR - Gear item not found in library: " + gearId);
        return false;
    }

    // If slot doesn't exist, create it
    if (static_cast<size_t>(slotIndex) >= rackSlots.size())
    {
        juce::Logger::writeToLog("Rack: Slot doesn't exist, creating new slot at index " + juce::String(slotIndex));
        insertRackSlot(slotIndex);
    }
    else if (isSlotOccupied(slotIndex))
    {
        // Slot exists but is occupied - insert a new slot at this position
        juce::Logger::writeToLog("Rack: Slot is occupied, inserting new slot at position " + juce::String(slotIndex));
        insertRackSlot(slotIndex);
        // After insertion, the new slot is at slotIndex, and the original occupied slot is now at slotIndex + 1
    }

    // Now add gear to the slot (which should be the newly created/available slot)
    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
    {
        juce::Logger::writeToLog("Rack: ERROR - Failed to get slot after creation");
        return false;
    }

    // Add gear to slot
    slot->setGearItem(gearItem);
    juce::Logger::writeToLog("Rack: Successfully added gear to slot " + juce::String(slotIndex));

    // Load schema and faceplate for the gear item
    if (gearLibrary.loadGearSchema(gearItem))
    {
        juce::Logger::writeToLog("Rack: Successfully loaded schema for " + gearItem->unitId);

        // Increment pending faceplate loads counter
        pendingFaceplateLoads++;
        juce::Logger::writeToLog("Rack: Incremented pendingFaceplateLoads to " + juce::String(pendingFaceplateLoads));

        // Load faceplate asynchronously with slot-specific callback
        auto slotCallback = [this, slotIndex]()
        {
            juce::Logger::writeToLog("Rack: Faceplate loaded for slot " + juce::String(slotIndex) + ", triggering slot callback");
            repaintSingleSlot(slotIndex);

            // Decrement pending faceplate loads counter
            pendingFaceplateLoads--;
            juce::Logger::writeToLog("Rack: Decremented pendingFaceplateLoads to " + juce::String(pendingFaceplateLoads));

            // If all faceplates are loaded, recalculate layout with correct heights
            if (pendingFaceplateLoads == 0)
            {
                juce::Logger::writeToLog("Rack: All faceplates loaded, recalculating layout with correct heights");
                layoutSlots();
            }
        };

        gearLibrary.loadGearFaceplateAsync(gearItem, slotCallback);
        juce::Logger::writeToLog("Rack: Started async faceplate loading for " + gearItem->unitId + " with slot callback");
    }
    else
    {
        juce::Logger::writeToLog("Rack: Failed to load schema for " + gearItem->unitId);
        // If schema loading failed, we still need to layout the slot
        layoutSlots();
    }

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

void Rack::repaintAllSlots()
{
    // Repaint all rack slots to show updated faceplates
    for (auto &slot : rackSlots)
    {
        if (slot)
        {
            slot->repaint();
        }
    }

    // Also repaint the rack container to ensure proper display
    if (rackContainer)
    {
        rackContainer->repaint();
    }
}

void Rack::repaintSingleSlot(int slotIndex)
{
    // Repaint only the specified slot for efficient updates
    if (slotIndex >= 0 && static_cast<size_t>(slotIndex) < rackSlots.size())
    {
        auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
        if (slot)
        {
            juce::Logger::writeToLog("Rack: Repainting single slot " + juce::String(slotIndex));
            slot->repaint();
        }
    }
    else
    {
        juce::Logger::writeToLog("Rack: Invalid slot index for repaint: " + juce::String(slotIndex));
    }
}
