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
#include "../Model/GearItem.h"

/**
 * @brief Constructs a new Rack instance.
 *
 * Initializes the rack with the specified number of slots and sets up the viewport
 * and container for managing the rack layout.
 *
 * @param networkFetcher Reference to the network fetcher
 * @param fileSystem Reference to the file system
 * @param cacheManager Reference to the cache manager
 * @param presetManager Reference to the preset manager
 * @param gearLibrary Pointer to the gear library
 */
Rack::Rack(INetworkFetcher &networkFetcher,
           IFileSystem &fileSystem,
           ICacheManager &cacheManager,
           PresetManager &presetManager,
           GearLibrary &gearLibrary)
    : networkFetcher(networkFetcher),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(gearLibrary)
{
    // Set component ID for debugging
    setComponentID("Rack");

    // Create the viewport and container
    rackViewport = std::make_unique<juce::Viewport>();
    rackContainer = std::make_unique<RackContainer>();

    // Set up the container
    rackContainer->rack = this;

    // Create rack slots
    for (int i = 0; i < numSlots; ++i)
    {
        auto *slot = new RackSlot(fileSystem, cacheManager, presetManager, gearLibrary, i);
        slots.add(slot);
        rackContainer->addAndMakeVisible(slot);
    }

    // Set up the viewport
    rackViewport->setViewedComponent(rackContainer.get());
    rackViewport->setScrollBarsShown(true, false); // Vertical scrollbar only

    // Add components to this rack
    addAndMakeVisible(rackViewport.get());
}

/**
 * @brief Destructor for the Rack class.
 *
 * Cleans up resources and ensures all images are properly released.
 */
Rack::~Rack()
{
    // The unique_ptrs and OwnedArray will clean up automatically
}

/**
 * @brief Paints the rack's background.
 *
 * Fills the background with a dark color to represent the rack chassis.
 *
 * @param g The graphics context to paint with
 */
void Rack::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey);
}

/**
 * @brief Handles resizing of the rack component.
 *
 * Adjusts the layout of the viewport, container, and all rack slots
 * based on the new dimensions.
 */
void Rack::resized()
{
    auto area = getLocalBounds();

    // Position the viewport to fill the entire rack area
    rackViewport->setBounds(area);

    // Calculate the total height needed for all slots
    int totalHeight = 0;
    for (int i = 0; i < slots.size(); ++i)
    {
        totalHeight += getSlotHeight(i) + slotSpacing;
    }

    // Remove spacing from the last slot
    if (totalHeight > 0)
        totalHeight -= slotSpacing;

    // Set the container size
    rackContainer->setSize(area.getWidth(), totalHeight);

    // Position each slot
    int currentY = 0;
    for (int i = 0; i < slots.size(); ++i)
    {
        auto *slot = slots[i];
        int slotHeight = getSlotHeight(i);

        slot->setBounds(0, currentY, area.getWidth(), slotHeight);
        currentY += slotHeight + slotSpacing;
    }
}

// DragAndDropTarget methods
bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // For now, accept all drag sources - this can be refined later
    return true;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Visual feedback can be added here later
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Visual feedback can be added here later
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Visual feedback can be added here later
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // For now, just log the drop - full implementation will come later
    // when we implement gear item creation and management
    std::cout << "[Rack] Item dropped at position: " << dragSourceDetails.localPosition.x
              << ", " << dragSourceDetails.localPosition.y << std::endl;
}

/**
 * @brief Gets the height of a specific rack slot.
 *
 * @param slotIndex The index of the slot to get the height for
 * @return The height of the slot in pixels
 */
int Rack::getSlotHeight(int slotIndex) const
{
    // For now, all slots have the same height
    // This can be customized later based on gear item requirements
    return getDefaultSlotHeight();
}
