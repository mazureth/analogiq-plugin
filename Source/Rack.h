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
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"
#include "INetworkFetcher.h"
#include "RackStateListener.h"
#include "IFileSystem.h"
#include "PresetManager.h"

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
     * @param gearLibrary Pointer to the gear library
     */
    Rack(INetworkFetcher &networkFetcher, IFileSystem &fileSystem, CacheManager &cacheManager, PresetManager &presetManager, GearLibrary *gearLibrary);

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
     * @brief Rearranges gear items in the rack using a sortable list.
     *
     * @param sourceSlotIndex The index of the source slot
     * @param targetSlotIndex The index of the target slot
     */
    void rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex);

    /**
     * @brief Finds the nearest rack slot to a given position.
     *
     * @param position The position to find the nearest slot for
     * @return Pointer to the nearest RackSlot, or nullptr if none found
     */
    RackSlot *findNearestSlot(const juce::Point<int> &position);

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
     * @brief Sets the gear library reference.
     *
     * @param library Pointer to the GearLibrary instance
     */
    void setGearLibrary(GearLibrary *library) { gearLibrary = library; }

    // Listener management
    /**
     * @brief Adds a listener for rack state changes.
     *
     * @param listener Pointer to the listener to add
     */
    void addRackStateListener(RackStateListener *listener);

    /**
     * @brief Removes a listener for rack state changes.
     *
     * @param listener Pointer to the listener to remove
     */
    void removeRackStateListener(RackStateListener *listener);

    /**
     * @brief Notifies all listeners of a gear item being added.
     *
     * @param slotIndex The index of the slot that was modified
     * @param gearItem Pointer to the gear item that was added
     */
    void notifyGearItemAdded(int slotIndex, GearItem *gearItem);

    /**
     * @brief Notifies all listeners of a gear item being removed.
     *
     * @param slotIndex The index of the slot that was modified
     */
    void notifyGearItemRemoved(int slotIndex);

    /**
     * @brief Notifies all listeners of a gear control being changed.
     *
     * @param slotIndex The index of the slot that was modified
     * @param gearItem Pointer to the gear item that was modified
     * @param controlIndex The index of the control that was modified
     */
    void notifyGearControlChanged(int slotIndex, GearItem *gearItem, int controlIndex);

    /**
     * @brief Notifies all listeners of gear items being rearranged.
     *
     * @param sourceSlotIndex The index of the source slot
     * @param targetSlotIndex The index of the target slot
     */
    void notifyGearItemsRearranged(int sourceSlotIndex, int targetSlotIndex);

    /**
     * @brief Notifies all listeners of the rack state being reset.
     */
    void notifyRackStateReset();

    /**
     * @brief Notifies all listeners of a preset being loaded.
     *
     * @param presetName The name of the preset that was loaded
     */
    void notifyPresetLoaded(const juce::String &presetName);

    /**
     * @brief Notifies all listeners of a preset being saved.
     *
     * @param presetName The name of the preset that was saved
     */
    void notifyPresetSaved(const juce::String &presetName);

    // Schema management
    /**
     * @brief Fetches the schema for a gear item.
     *
     * @param item The gear item to fetch the schema for
     * @param onComplete Optional callback to execute when schema loading is complete
     */
    void fetchSchemaForGearItem(GearItem *item, std::function<void()> onComplete = nullptr);

    /**
     * @brief Parses the schema data for a gear item.
     *
     * @param schemaData The JSON schema data to parse
     * @param item The gear item to update with the parsed schema
     * @param onComplete Optional callback to execute when parsing is complete
     */
    void parseSchema(const juce::String &schemaData, GearItem *item, std::function<void()> onComplete = nullptr);

    /**
     * @brief Fetches the faceplate image for a gear item.
     *
     * @param item The gear item to fetch the faceplate for
     */
    void fetchFaceplateImage(GearItem *item);

    /**
     * @brief Fetches the knob image for a gear control.
     *
     * @param item The gear item containing the control
     * @param controlIndex The index of the control
     */
    void fetchKnobImage(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the fader image for a gear control.
     *
     * @param item The gear item containing the control
     * @param controlIndex The index of the control
     */
    void fetchFaderImage(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the switch sprite sheet for a gear control.
     *
     * @param item The gear item containing the control
     * @param controlIndex The index of the control
     */
    void fetchSwitchSpriteSheet(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the button sprite sheet for a gear control.
     *
     * @param item The gear item containing the control
     * @param controlIndex The index of the control
     */
    void fetchButtonSpriteSheet(GearItem *item, int controlIndex);

    // Instance management
    /**
     * @brief Creates a new instance of a gear item in a slot.
     *
     * @param slotIndex The index of the slot to create the instance in
     */
    void createInstance(int slotIndex);

    /**
     * @brief Resets a slot to its source gear item.
     *
     * @param slotIndex The index of the slot to reset
     */
    void resetToSource(int slotIndex);

    /**
     * @brief Checks if a slot contains an instance.
     *
     * @param slotIndex The index of the slot to check
     * @return true if the slot contains an instance
     */
    bool isInstance(int slotIndex) const;

    /**
     * @brief Gets the instance ID for a slot.
     *
     * @param slotIndex The index of the slot
     * @return The instance ID as a string
     */
    juce::String getInstanceId(int slotIndex) const;

    /**
     * @brief Resets all instances in the rack to their source gear items.
     */
    void resetAllInstances();

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
        void paint(juce::Graphics &g) override { g.fillAll(juce::Colours::black); }

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
    GearLibrary *gearLibrary; ///< Pointer to the gear library

    // Reference to the network fetcher
    INetworkFetcher &networkFetcher; ///< Reference to the network fetcher

    // Reference to the file system
    IFileSystem &fileSystem;

    // Reference to the cache manager
    CacheManager &cacheManager;

    // Reference to the preset manager
    PresetManager &presetManager;

    // Listener management
    juce::Array<RackStateListener *> rackStateListeners; ///< Array of rack state listeners

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