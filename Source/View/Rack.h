/**
 * @file Rack.h
 * @brief Header file for the Rack class which manages a virtual rack of audio gear.
 *
 * This file defines the Rack class which provides a visual interface for managing
 * audio gear items in a virtual rack system. It handles the layout, drag-and-drop
 * functionality, and resource management for gear items and their controls.
 */

#pragma once

#include <JuceHeader.h>
#include "../Shared/INetworkFetcher.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Model/PresetManager.h"
#include "../Model/GearLibrary.h"

// Forward declarations
class RackSlot;

/**
 * @class Rack
 * @brief A component that manages a virtual rack of audio gear items.
 *
 * The Rack class provides a visual interface for managing audio gear items in a virtual rack.
 * It handles the layout of gear items, drag-and-drop functionality for adding and rearranging
 * items, and manages the loading and display of gear resources like faceplates and controls.
 */
class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    /**
     * @brief Constructs a new Rack instance.
     *
     * @param networkFetcher Reference to the network fetcher
     * @param fileSystem Reference to the file system
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     * @param gearLibrary Reference to the gear library
     */
    Rack(INetworkFetcher &networkFetcher,
         IFileSystem &fileSystem,
         ICacheManager &cacheManager,
         PresetManager &presetManager,
         GearLibrary &gearLibrary);

    /**
     * @brief Destructor for the Rack class.
     *
     * Cleans up resources and ensures all images are properly released.
     */
    ~Rack() override;

    /**
     * @brief Paints the rack's background.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handles resizing of the rack component.
     *
     * Adjusts the layout of the viewport, container, and all rack slots
     * based on the new dimensions.
     */
    void resized() override;

    // DragAndDropTarget methods
    /**
     * @brief Checks if the rack is interested in a drag source.
     *
     * @param dragSourceDetails Details about the drag source
     * @return true if the rack accepts drops from this source
     */
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item enters the rack.
     *
     * @param dragSourceDetails Details about the drag source
     */
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item moves over the rack.
     *
     * @param dragSourceDetails Details about the drag source and position
     */
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item exits the rack.
     *
     * @param dragSourceDetails Details about the drag source
     */
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item is dropped onto the rack.
     *
     * @param dragSourceDetails Details about the drag source and drop position
     */
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Gets the number of slots in the rack.
     *
     * @return The number of slots
     */
    int getNumSlots() const { return slots.size(); }

    /**
     * @brief Gets a specific slot by index.
     *
     * @param index The index of the slot to get
     * @return Pointer to the RackSlot, or nullptr if index is invalid
     */
    RackSlot *getSlot(int index) const { return (index >= 0 && index < slots.size()) ? slots[index] : nullptr; }



    /**
     * @class RackContainer
     * @brief Internal container class for rack slots.
     *
     * This class provides a container component that holds all the rack slots
     * and is managed by the viewport.
     */
    class RackContainer : public juce::Component
    {
    public:
        /**
         * @brief Constructs a new RackContainer.
         *
         * Sets the component ID for debugging purposes.
         */
        RackContainer() { setComponentID("RackContainer"); }

        /**
         * @brief Paints the container's background.
         *
         * @param g The graphics context to paint with
         */
        void paint(juce::Graphics &g) override { g.fillAll(juce::Colours::darkgrey); }

        Rack *rack = nullptr; ///< Reference to the parent rack
    };

private:
    // Configuration
    int numSlots = 16;    ///< Number of slots in the rack
    int slotSpacing = 10; ///< Spacing between slots in pixels

    // UI Components
    std::unique_ptr<juce::Viewport> rackViewport; ///< Viewport for scrolling the rack
    std::unique_ptr<RackContainer> rackContainer; ///< Container for rack slots
    juce::OwnedArray<RackSlot> slots;             ///< Array of rack slots

    // Reference to the gear library (for drag and drop)
    GearLibrary &gearLibrary; ///< Reference to the gear library

    // Reference to the network fetcher
    INetworkFetcher &networkFetcher; ///< Reference to the network fetcher

    // Reference to the file system
    IFileSystem &fileSystem;

    // Reference to the cache manager
    ICacheManager &cacheManager;

    // Reference to the preset manager
    PresetManager &presetManager;

    /**
     * @brief Gets the height of a specific rack slot.
     *
     * @param slotIndex The index of the slot to get the height for
     * @return The height of the slot in pixels
     */
    int getSlotHeight(int slotIndex) const;

    /**
     * @brief Gets the default height for a slot.
     *
     * @return The default slot height in pixels
     */
    int getDefaultSlotHeight() const { return 150; } // Default height if not overridden

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
};
