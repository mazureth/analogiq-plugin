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
    int totalHeight = slotSpacing; // Start with top spacing
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        totalHeight += getSlotHeight(static_cast<int>(i)) + slotSpacing;
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
    int effectiveSlotWidth = getWidth() - (2 * slotSpacing);

    // Position the slots within the container in a single vertical column
    int currentY = slotSpacing;
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            int slotHeight = getSlotHeight(static_cast<int>(i));

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
                bool success = addGearToSlot(slotIndex, gearId);

                if (success)
                {
                    // Recently used is handled in addGearToSlot() to capture all gear additions
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
    int currentY = slotSpacing;
    for (int i = 0; i < static_cast<int>(rackSlots.size()); ++i)
    {
        int slotHeight = getSlotHeight(i);
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

    // For fluid rack, we can create slots at any valid index
    if (slotIndex < 0)
    {
        return false;
    }

    // For fluid rack, we can always create slots at any index
    // The only constraint is that the gear must exist in the library

    // Check if gear exists in library
    bool gearExists = gearLibrary.gearItemExists(gearId);

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

    // Create a new slot at the specified index
    auto newSlot = std::make_unique<RackSlot>(fileSystem, cacheManager, presetManager, gearLibrary, slotIndex);
    newSlot->setComponentID("RackSlot_" + juce::String(slotIndex));
    newSlot->setRack(this); // Set reference to parent rack
    newSlot->addComponentListener(this);

    // Update button states after setting rack reference
    newSlot->updateButtonStates();

    // Set up individual faceplate loaded callback for this slot
    newSlot->setFaceplateLoadedCallback([this, slotIndex]()
                                        { repaintSingleSlot(slotIndex); });

    // Set the slot's background color
    newSlot->setSlotBackgroundColor(slotBackgroundColor);

    // Insert the slot at the specified position
    if (static_cast<size_t>(slotIndex) >= rackSlots.size())
    {
        // Append to end
        rackSlots.push_back(std::move(newSlot));

        // Ensure gearItemInstances vector is large enough
        if (static_cast<size_t>(slotIndex) >= gearItemInstances.size())
        {
            gearItemInstances.resize(slotIndex + 1);
        }
    }
    else
    {
        // Insert at specified position
        rackSlots.insert(rackSlots.begin() + slotIndex, std::move(newSlot));

        // Insert a null entry in gearItemInstances at the same position
        gearItemInstances.insert(gearItemInstances.begin() + slotIndex, nullptr);

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
    // Validate slot index
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("Remove rack slot failed: invalid slot index " + juce::String(slotIndex));
        return;
    }

    // Remove gear from slot first
    removeGearFromSlot(slotIndex);

    // Remove the slot
    rackSlots.erase(rackSlots.begin() + slotIndex);

    // Remove the corresponding gear item instance
    if (static_cast<size_t>(slotIndex) < gearItemInstances.size())
    {
        gearItemInstances.erase(gearItemInstances.begin() + slotIndex);
    }

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

    // Clear the gear item instances array
    gearItemInstances.clear();
}

// Rack slot validation methods
bool Rack::isValidSlotIndex(int slotIndex) const
{
    return slotIndex >= 0 && static_cast<size_t>(slotIndex) < rackSlots.size();
}

bool Rack::validateRackState() const
{
    // Check if rack has valid number of slots
    if (rackSlots.size() != gearItemInstances.size())
    {
        juce::Logger::writeToLog("Rack validation failed: slot count mismatch");
        return false;
    }

    // Validate slot consistency
    if (!validateSlotConsistency())
    {
        return false;
    }

    // Validate gear-slot relationships
    if (!validateGearSlotRelationships())
    {
        return false;
    }

    return true;
}

bool Rack::validateSlotConsistency() const
{
    // Check that all slots exist and have valid indices
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (!rackSlots[i])
        {
            juce::Logger::writeToLog("Rack validation failed: null slot at index " + juce::String(i));
            return false;
        }

        // Check that slot index matches its position
        if (rackSlots[i]->getSlotIndex() != static_cast<int>(i))
        {
            juce::Logger::writeToLog("Rack validation failed: slot index mismatch at position " + juce::String(i));
            return false;
        }
    }

    return true;
}

bool Rack::validateGearSlotRelationships() const
{
    // Check that gear items are properly associated with slots
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            auto gearItem = rackSlots[i]->getGearItem();
            if (gearItem && static_cast<size_t>(i) < gearItemInstances.size())
            {
                // Check that the gear item instance matches what's in the slot
                if (gearItemInstances[i].get() != gearItem)
                {
                    juce::Logger::writeToLog("Rack validation failed: gear item mismatch at slot " + juce::String(i));
                    return false;
                }
            }
        }
    }

    return true;
}

void Rack::recoverFromInvalidState()
{
    juce::Logger::writeToLog("Rack recovery: Attempting to recover from invalid state");

    // Clear all slots and start fresh
    clearAllSlots();

    // Recreate a basic rack structure
    setSlotLayout(16); // Default to 16 slots

    juce::Logger::writeToLog("Rack recovery: Recovered to clean state with " + juce::String(rackSlots.size()) + " slots");
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
    // Basic validation - slot index must be non-negative
    if (slotIndex < 0)
    {
        juce::Logger::writeToLog("Add gear failed: negative slot index " + juce::String(slotIndex));
        return false;
    }

    // Get gear item template from library and create a unique instance
    auto gearItemTemplate = gearLibrary.getGearItem(gearId);
    if (!gearItemTemplate)
    {
        return false;
    }

    // If slot doesn't exist, create it
    if (static_cast<size_t>(slotIndex) >= rackSlots.size())
    {
        insertRackSlot(slotIndex);
    }
    else if (isSlotOccupied(slotIndex))
    {
        // Slot exists but is occupied - insert a new slot at this position
        insertRackSlot(slotIndex);
        // After insertion, the new slot is at slotIndex, and the original occupied slot is now at slotIndex + 1
    }

    // Now validate that the slot exists after creation
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("Add gear failed: slot creation failed for index " + juce::String(slotIndex));
        return false;
    }

    // Create a unique instance for this rack slot (AFTER slot insertion)
    auto gearItem = std::make_unique<GearItem>(gearItemTemplate->createInstance());

    // Ensure we have enough space in the gearItemInstances vector
    if (static_cast<size_t>(slotIndex) >= gearItemInstances.size())
    {
        gearItemInstances.resize(slotIndex + 1);
    }

    // Store the unique instance
    gearItemInstances[static_cast<size_t>(slotIndex)] = std::move(gearItem);
    auto *gearItemPtr = gearItemInstances[static_cast<size_t>(slotIndex)].get();

    // Now add gear to the slot (which should be the newly created/available slot)
    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
    {
        return false;
    }

    // Add gear to slot
    slot->setGearItem(gearItemPtr);

    // Load schema and faceplate for the gear item
    if (gearLibrary.loadGearSchema(gearItemPtr))
    {

        // Increment pending faceplate loads counter
        pendingFaceplateLoads++;

        // Load faceplate asynchronously with slot-specific callback
        auto slotCallback = [this, slotIndex]()
        {
            // Debug: Check what GearItem is in the slot now
            auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
            if (slot && slot->getGearItem())
            {
            }
            repaintSingleSlot(slotIndex);

            // Decrement pending faceplate loads counter
            pendingFaceplateLoads--;

            // If all faceplates are loaded, recalculate layout with correct heights
            if (pendingFaceplateLoads == 0)
            {
                layoutSlots();
            }
        };

        gearLibrary.loadGearFaceplateAsync(gearItemPtr, slotCallback);
    }
    else
    {
        // If schema loading failed, we still need to layout the slot
        layoutSlots();
    }

    // Save rack state
    saveRackState();

    // Notify listeners
    notifyStateChanged();

    // Add to recently used
    cacheManager.addToRecentlyUsed(gearId);

    // Notify specific listeners about gear item added
    notifyGearItemAdded(slotIndex, gearItemPtr);

    return true;
}

bool Rack::addGearToSlotWithCallback(int slotIndex, const juce::String &gearId, std::function<void()> callback)
{
    juce::Logger::writeToLog("addGearToSlotWithCallback called: slotIndex=" + juce::String(slotIndex) + ", gearId=" + gearId);

    // Basic validation - slot index must be non-negative
    if (slotIndex < 0)
    {
        juce::Logger::writeToLog("Add gear failed: negative slot index " + juce::String(slotIndex));
        return false;
    }

    // Get gear item template from library and create a unique instance
    // Strip instance suffix from gearId for library lookup
    juce::String baseGearId = gearId;
    int instPos = gearId.indexOf("_inst_");
    if (instPos != -1)
    {
        baseGearId = gearId.substring(0, instPos);
    }

    juce::Logger::writeToLog("Looking up gear item in library for gearId=" + gearId + " (baseGearId=" + baseGearId + ")");
    auto gearItemTemplate = gearLibrary.getGearItem(baseGearId);
    if (!gearItemTemplate)
    {
        juce::Logger::writeToLog("Add gear failed: gear item template not found for baseGearId=" + baseGearId);
        juce::Logger::writeToLog("Available gear items in library:");
        auto items = gearLibrary.getAllGearItems();
        for (int i = 0; i < items.size(); ++i)
        {
            juce::Logger::writeToLog("  - " + items[i]->unitId + " (" + items[i]->name + ")");
        }
        return false;
    }

    juce::Logger::writeToLog("Found gear item template: " + gearItemTemplate->name);

    // If slot doesn't exist, create it
    if (static_cast<size_t>(slotIndex) >= rackSlots.size())
    {
        insertRackSlot(slotIndex);
    }
    else if (isSlotOccupied(slotIndex))
    {
        // Slot exists but is occupied - insert a new slot at this position
        insertRackSlot(slotIndex);
        // After insertion, the new slot is at slotIndex, and the original occupied slot is now at slotIndex + 1
    }

    // Now validate that the slot exists after creation
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("Add gear failed: slot creation failed for index " + juce::String(slotIndex));
        return false;
    }

    // Create a unique instance for this rack slot (AFTER slot insertion)
    auto gearItem = std::make_unique<GearItem>(gearItemTemplate->createInstance());

    // Ensure we have enough space in the gearItemInstances vector
    if (static_cast<size_t>(slotIndex) >= gearItemInstances.size())
    {
        gearItemInstances.resize(slotIndex + 1);
    }

    // Store the unique instance
    gearItemInstances[static_cast<size_t>(slotIndex)] = std::move(gearItem);
    auto *gearItemPtr = gearItemInstances[static_cast<size_t>(slotIndex)].get();

    // Now add gear to the slot (which should be the newly created/available slot)
    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
    {
        return false;
    }

    // Add gear to slot
    slot->setGearItem(gearItemPtr);

    // Load schema and faceplate for the gear item
    if (gearLibrary.loadGearSchema(gearItemPtr))
    {
        // Increment pending faceplate loads counter
        pendingFaceplateLoads++;

        // Load faceplate asynchronously with slot-specific callback that includes the user callback
        auto slotCallback = [this, slotIndex, callback]()
        {
            // Debug: Check what GearItem is in the slot now
            auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
            if (slot && slot->getGearItem())
            {
            }
            repaintSingleSlot(slotIndex);

            // Decrement pending faceplate loads counter
            pendingFaceplateLoads--;

            // If all faceplates are loaded, recalculate layout with correct heights
            if (pendingFaceplateLoads == 0)
            {
                layoutSlots();
            }

            // Call the user-provided callback after the gear item is fully loaded
            if (callback)
            {
                callback();
            }
        };

        gearLibrary.loadGearFaceplateAsync(gearItemPtr, slotCallback);
    }
    else
    {
        // If schema loading failed, we still need to layout the slot
        layoutSlots();

        // Call the user-provided callback even if schema loading failed
        if (callback)
        {
            callback();
        }
    }

    // Save rack state
    saveRackState();

    // Notify listeners
    notifyStateChanged();

    // Add to recently used
    cacheManager.addToRecentlyUsed(gearId);

    // Notify specific listeners about gear item added
    notifyGearItemAdded(slotIndex, gearItemPtr);

    juce::Logger::writeToLog("addGearToSlotWithCallback completed successfully for slotIndex=" + juce::String(slotIndex));
    return true;
}

bool Rack::removeGearFromSlot(int slotIndex)
{
    // Validate slot index
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("Remove gear failed: invalid slot index " + juce::String(slotIndex));
        return false;
    }

    auto slot = rackSlots[static_cast<size_t>(slotIndex)].get();
    if (!slot)
    {
        juce::Logger::writeToLog("Remove gear failed: null slot at index " + juce::String(slotIndex));
        return false;
    }

    slot->clearGearItem();

    // Clear the gear item instance
    if (static_cast<size_t>(slotIndex) < gearItemInstances.size())
    {
        gearItemInstances[static_cast<size_t>(slotIndex)].reset();
    }

    // Save rack state
    saveRackState();

    // Notify listeners
    notifyStateChanged();

    // Notify specific listeners about gear item removed
    notifyGearItemRemoved(slotIndex);

    return true;
}

bool Rack::moveGearBetweenSlots(int fromSlot, int toSlot)
{
    // Validate slot indices
    if (!isValidSlotIndex(fromSlot) || !isValidSlotIndex(toSlot))
    {
        juce::Logger::writeToLog("Move gear failed: invalid slot indices - from: " + juce::String(fromSlot) + ", to: " + juce::String(toSlot));
        return false;
    }

    if (fromSlot == toSlot)
        return true;

    // Get the actual gear items from both slots
    auto fromSlotPtr = rackSlots[static_cast<size_t>(fromSlot)].get();
    auto toSlotPtr = rackSlots[static_cast<size_t>(toSlot)].get();

    if (!fromSlotPtr || !toSlotPtr)
        return false;

    // Get gear items directly from slots
    auto fromGearItem = fromSlotPtr->getGearItem();
    auto toGearItem = toSlotPtr->getGearItem();

    // If source slot is empty, nothing to move
    if (!fromGearItem)
        return false;

    // If destination slot is empty, just move the gear
    if (!toGearItem)
    {
        // Move gear from source to destination
        toSlotPtr->setGearItem(fromGearItem);
        fromSlotPtr->clearGearItem();

        // Update gear item instances
        gearItemInstances[static_cast<size_t>(toSlot)] = std::move(gearItemInstances[static_cast<size_t>(fromSlot)]);
        gearItemInstances[static_cast<size_t>(fromSlot)].reset();

        // Save rack state
        saveRackState();
        notifyStateChanged();

        return true;
    }
    else
    {
        // Both slots have gear - swap them
        // Store gear items temporarily
        auto tempFromGear = std::move(gearItemInstances[static_cast<size_t>(fromSlot)]);
        auto tempToGear = std::move(gearItemInstances[static_cast<size_t>(toSlot)]);

        // Clear both slots first
        fromSlotPtr->clearGearItem();
        toSlotPtr->clearGearItem();

        // Swap the gear items
        fromSlotPtr->setGearItem(tempToGear.get());
        toSlotPtr->setGearItem(tempFromGear.get());

        // Swap the gear item instances
        gearItemInstances[static_cast<size_t>(fromSlot)] = std::move(tempToGear);
        gearItemInstances[static_cast<size_t>(toSlot)] = std::move(tempFromGear);

        // Save rack state
        saveRackState();
        notifyStateChanged();

        return true;
    }
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

// Preset serialization
juce::String Rack::serializeRackToJSON() const
{
    juce::DynamicObject::Ptr rackObject = new juce::DynamicObject();

    // Add rack metadata
    rackObject->setProperty("version", "1.0");
    rackObject->setProperty("numSlots", static_cast<int>(rackSlots.size()));
    rackObject->setProperty("slotWidth", slotWidth);
    rackObject->setProperty("slotHeight", slotHeight);
    rackObject->setProperty("slotSpacing", slotSpacing);

    // Create array of slots
    juce::Array<juce::var> slotsArray;

    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        auto slot = rackSlots[i].get();
        if (slot && isSlotOccupied(static_cast<int>(i)))
        {
            juce::DynamicObject::Ptr slotObject = new juce::DynamicObject();
            slotObject->setProperty("slotIndex", static_cast<int>(i));

            auto gearItem = slot->getGearItem();
            if (gearItem)
            {
                slotObject->setProperty("gearId", gearItem->unitId);
                slotObject->setProperty("gearName", gearItem->name);

                // Serialize control values
                juce::DynamicObject::Ptr controlsObject = new juce::DynamicObject();
                for (size_t j = 0; j < gearItem->controls.size(); ++j)
                {
                    const auto &control = gearItem->controls[j];
                    controlsObject->setProperty(control.name, control.currentValue);
                }
                slotObject->setProperty("controls", controlsObject.get());
            }

            slotsArray.add(slotObject.get());
        }
    }

    rackObject->setProperty("slots", slotsArray);

    // Convert to JSON string
    juce::String jsonString = juce::JSON::toString(juce::var(rackObject.get()));
    juce::Logger::writeToLog("Serialized JSON: " + jsonString);
    return jsonString;
}

bool Rack::deserializeRackFromJSON(const juce::String &jsonString)
{
    juce::Logger::writeToLog("deserializeRackFromJSON called with JSON length: " + juce::String(jsonString.length()));

    // Debug: Check gear library state
    auto items = gearLibrary.getAllGearItems();
    juce::Logger::writeToLog("Gear library has " + juce::String(items.size()) + " items");
    for (int i = 0; i < items.size(); ++i)
    {
        juce::Logger::writeToLog("  - " + items[i]->unitId + " (" + items[i]->name + ")");
    }

    try
    {
        // Parse JSON
        juce::var parsedJson = juce::JSON::parse(jsonString);
        if (!parsedJson.isObject())
        {
            juce::Logger::writeToLog("deserializeRackFromJSON failed: JSON is not an object");
            return false;
        }

        juce::DynamicObject *rackObject = parsedJson.getDynamicObject();
        if (!rackObject)
            return false;

        // Clear existing rack state
        clearAllSlots();

        // Load rack configuration
        if (rackObject->hasProperty("numSlots"))
        {
            int numSlots = rackObject->getProperty("numSlots");
            setSlotLayout(numSlots);
        }

        if (rackObject->hasProperty("slotWidth"))
            slotWidth = rackObject->getProperty("slotWidth");
        if (rackObject->hasProperty("slotHeight"))
            slotHeight = rackObject->getProperty("slotHeight");
        if (rackObject->hasProperty("slotSpacing"))
            slotSpacing = rackObject->getProperty("slotSpacing");

        // Load slots
        if (rackObject->hasProperty("slots"))
        {
            juce::var slotsVar = rackObject->getProperty("slots");
            if (slotsVar.isArray())
            {
                juce::Array<juce::var> *slotsArray = slotsVar.getArray();
                juce::Logger::writeToLog("Found " + juce::String(slotsArray->size()) + " slots in JSON");

                for (int i = 0; i < slotsArray->size(); ++i)
                {
                    juce::var slotVar = slotsArray->getReference(i);
                    if (slotVar.isObject())
                    {
                        juce::DynamicObject *slotObject = slotVar.getDynamicObject();
                        if (slotObject)
                        {
                            int slotIndex = slotObject->getProperty("slotIndex");
                            juce::String gearId = slotObject->getProperty("gearId");

                            juce::Logger::writeToLog("Processing slot " + juce::String(i) + ": slotIndex=" + juce::String(slotIndex) + ", gearId=" + gearId);

                            if (!gearId.isEmpty())
                            {
                                juce::Logger::writeToLog("Deserializing gear: slotIndex=" + juce::String(slotIndex) + ", gearId=" + gearId);

                                // Store control values for later restoration
                                juce::var controlsVar;
                                if (slotObject->hasProperty("controls"))
                                {
                                    controlsVar = slotObject->getProperty("controls");
                                }

                                // Add gear to slot with callback to restore control values after loading
                                bool success = addGearToSlotWithCallback(slotIndex, gearId, [this, slotIndex, controlsVar]()
                                                                         {
                                    // This callback runs after the gear item is fully loaded
                                    auto slot = getSlot(slotIndex);
                                    if (slot)
                                    {
                                        auto gearItem = slot->getGearItem();
                                        if (gearItem && controlsVar.isObject())
                                        {
                                            juce::DynamicObject *controlsObject = controlsVar.getDynamicObject();
                                            if (controlsObject)
                                            {
                                                // Restore control values
                                                for (size_t j = 0; j < gearItem->controls.size(); ++j)
                                                {
                                                    auto &control = gearItem->controls.getReference(static_cast<int>(j));
                                                    if (controlsObject->hasProperty(control.name))
                                                    {
                                                        control.currentValue = controlsObject->getProperty(control.name);
                                                    }
                                                }

                                                // Notify that controls have changed
                                                slot->repaint();
                                            }
                                        }
                                    } });
                            }
                        }
                    }
                }
            }
        }

        // Layout the slots after loading
        layoutSlots();

        // Save the new state
        saveRackState();

        return true;
    }
    catch (const std::exception &e)
    {
        juce::Logger::writeToLog("Error deserializing rack from JSON: " + juce::String(e.what()));
        return false;
    }
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
void Rack::addRackStateListener(RackStateListener *listener)
{
    if (listener && !stateListeners.contains(listener))
    {
        stateListeners.add(listener);
    }
}

void Rack::removeRackStateListener(RackStateListener *listener)
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
            // RackStateListener doesn't inherit from Component, so we can't call repaint()
            // The specific notification methods (notifyGearItemAdded, etc.) handle UI updates
        }
    }
}

void Rack::notifyGearItemAdded(int slotIndex, const GearItem *gearItem)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemAdded(this, slotIndex, gearItem);
        }
    }
}

void Rack::notifyGearItemRemoved(int slotIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemRemoved(this, slotIndex);
        }
    }
}

void Rack::notifyGearControlChanged(int slotIndex, const GearItem *gearItem, int controlIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearControlChanged(this, slotIndex, gearItem, controlIndex);
        }
    }
}

void Rack::notifyGearItemsRearranged(int sourceSlotIndex, int targetSlotIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemsRearranged(this, sourceSlotIndex, targetSlotIndex);
        }
    }
}

void Rack::notifyRackStateReset()
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onRackStateReset(this);
        }
    }
}

void Rack::notifyPresetLoaded(const juce::String &presetName)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onPresetLoaded(this, presetName);
        }
    }
}

void Rack::notifyPresetSaved(const juce::String &presetName)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onPresetSaved(this, presetName);
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
            slot->repaint();
        }
    }
    else
    {
    }
}

void Rack::updateSlotIndices()
{
    // Update the index of each slot to match its position in the vector
    for (size_t i = 0; i < rackSlots.size(); ++i)
    {
        if (rackSlots[i])
        {
            rackSlots[i]->setIndex(static_cast<int>(i));
            // Button states are updated in setIndex() method
        }
    }

    // Validate rack state after updating indices
    if (!validateRackState())
    {
        juce::Logger::writeToLog("Rack state validation failed after updating slot indices - attempting recovery");
        recoverFromInvalidState();
    }
}
