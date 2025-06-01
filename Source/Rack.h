/**
 * @file Rack.h
 * @brief Header file for the Rack class, which manages a collection of rack slots for gear items.
 *
 * The Rack class is responsible for:
 * - Managing a collection of rack slots
 * - Handling drag and drop operations from the gear library
 * - Managing gear item schemas and images
 * - Supporting instance management for gear items
 * - Coordinating slot interactions and reordering
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"

/**
 * @class Rack
 * @brief Represents a rack that can contain multiple gear items in slots.
 *
 * The Rack class manages a collection of RackSlot components, each capable of
 * holding a gear item. It handles drag and drop operations from the gear library,
 * manages gear item schemas and images, and supports instance management.
 */
class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    /**
     * @brief Constructs a new Rack.
     *
     * Initializes the rack with:
     * - A viewport for scrolling
     * - A container for rack slots
     * - The specified number of slots
     */
    Rack();

    /**
     * @brief Destructor for Rack.
     */
    ~Rack() override;

    /**
     * @brief Paints the rack and its contents.
     * @param g The graphics context to paint with.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Called when the component is resized.
     * Updates the layout of internal components.
     */
    void resized() override;

    /**
     * @brief Checks if this rack is interested in a drag source.
     * @param dragSourceDetails Details about the drag source.
     * @return true if this rack can accept the drag source.
     */
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation enters this rack.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation moves within this rack.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation exits this rack.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation is dropped on this rack.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Swaps items between two slots.
     * @param sourceSlotIndex The index of the source slot.
     * @param targetSlotIndex The index of the target slot.
     */
    void rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex);

    /**
     * @brief Finds the nearest slot to a position.
     * @param position The position to check.
     * @return Pointer to the nearest slot, or nullptr if none found.
     */
    RackSlot *findNearestSlot(const juce::Point<int> &position);

    /**
     * @brief Gets the number of slots in the rack.
     * @return The number of slots.
     */
    int getNumSlots() const { return slots.size(); }

    /**
     * @brief Gets a specific slot by index.
     * @param index The index of the slot to get.
     * @return Pointer to the slot, or nullptr if index is invalid.
     */
    RackSlot *getSlot(int index) const { return (index >= 0 && index < slots.size()) ? slots[index] : nullptr; }

    /**
     * @brief Sets the gear library reference.
     * @param library Pointer to the gear library.
     */
    void setGearLibrary(GearLibrary *library) { gearLibrary = library; }

    /**
     * @brief Fetches the schema for a gear item.
     * @param item The gear item to fetch the schema for.
     */
    void fetchSchemaForGearItem(GearItem *item);

    /**
     * @brief Parses a schema for a gear item.
     * @param schemaData The schema data to parse.
     * @param item The gear item to parse the schema for.
     */
    void parseSchema(const juce::String &schemaData, GearItem *item);

    /**
     * @brief Fetches the faceplate image for a gear item.
     * @param item The gear item to fetch the faceplate for.
     */
    void fetchFaceplateImage(GearItem *item);

    /**
     * @brief Fetches the knob image for a control.
     * @param item The gear item containing the control.
     * @param controlIndex The index of the control.
     */
    void fetchKnobImage(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the fader image for a control.
     * @param item The gear item containing the control.
     * @param controlIndex The index of the control.
     */
    void fetchFaderImage(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the switch sprite sheet for a control.
     * @param item The gear item containing the control.
     * @param controlIndex The index of the control.
     */
    void fetchSwitchSpriteSheet(GearItem *item, int controlIndex);

    /**
     * @brief Fetches the button sprite sheet for a control.
     * @param item The gear item containing the control.
     * @param controlIndex The index of the control.
     */
    void fetchButtonSpriteSheet(GearItem *item, int controlIndex);

    /**
     * @brief Creates an instance of the gear item in a slot.
     * @param slotIndex The index of the slot to create the instance in.
     */
    void createInstance(int slotIndex);

    /**
     * @brief Resets a gear item to its source state.
     * @param slotIndex The index of the slot containing the item to reset.
     */
    void resetToSource(int slotIndex);

    /**
     * @brief Checks if a gear item is an instance.
     * @param slotIndex The index of the slot to check.
     * @return true if the gear item is an instance.
     */
    bool isInstance(int slotIndex) const;

    /**
     * @brief Gets the instance ID of a gear item.
     * @param slotIndex The index of the slot to get the instance ID from.
     * @return The instance ID, or empty string if not an instance.
     */
    juce::String getInstanceId(int slotIndex) const;

    /**
     * @brief Resets all instances to their source states.
     */
    void resetAllInstances();

    /**
     * @class RackContainer
     * @brief Internal container class for rack slots.
     *
     * The RackContainer class provides a container for the rack slots,
     * allowing them to be scrolled within a viewport.
     */
    class RackContainer : public juce::Component
    {
    public:
        /**
         * @brief Constructs a new RackContainer.
         */
        RackContainer() { setComponentID("RackContainer"); }

        /**
         * @brief Paints the container.
         * @param g The graphics context to paint with.
         */
        void paint(juce::Graphics &g) override { g.fillAll(juce::Colours::black); }

        Rack *rack = nullptr; ///< Pointer to the parent rack.
    };

private:
    int numSlots = 16;    ///< Number of slots in the rack
    int slotSpacing = 10; ///< Spacing between slots

    std::unique_ptr<juce::Viewport> rackViewport; ///< Viewport for scrolling
    std::unique_ptr<RackContainer> rackContainer; ///< Container for rack slots
    juce::OwnedArray<RackSlot> slots;             ///< Array of rack slots

    GearLibrary *gearLibrary = nullptr; ///< Reference to the gear library

    /**
     * @brief Gets the height of a slot based on its contents.
     * @param slotIndex The index of the slot.
     * @return The height of the slot.
     */
    int getSlotHeight(int slotIndex) const;

    /**
     * @brief Gets the default height for a slot.
     * @return The default slot height.
     */
    int getDefaultSlotHeight() const { return 150; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
};