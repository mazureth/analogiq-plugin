/**
 * @file RackStateListener.h
 * @brief Header file for the RackStateListener interface.
 *
 * This file defines the RackStateListener interface, which provides a way for
 * components to be notified when the rack state changes. This is used by the
 * preset system to track when the rack has been modified.
 */

#pragma once

#include <JuceHeader.h>

// Forward declarations
class Rack;
class RackSlot;
class GearItem;

/**
 * @brief Interface for components that need to be notified of rack state changes.
 *
 * The RackStateListener interface provides methods that are called when various
 * aspects of the rack state change, such as gear items being added, removed,
 * or modified. This allows the preset system to track when the rack has been
 * modified and needs to be saved.
 */
class RackStateListener
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~RackStateListener() = default;

    /**
     * @brief Called when a gear item is added to a rack slot.
     *
     * @param rack Pointer to the rack that was modified
     * @param slotIndex The index of the slot that was modified
     * @param gearItem Pointer to the gear item that was added
     */
    virtual void onGearItemAdded(Rack *rack, int slotIndex, GearItem *gearItem) = 0;

    /**
     * @brief Called when a gear item is removed from a rack slot.
     *
     * @param rack Pointer to the rack that was modified
     * @param slotIndex The index of the slot that was modified
     */
    virtual void onGearItemRemoved(Rack *rack, int slotIndex) = 0;

    /**
     * @brief Called when a gear item's controls are modified.
     *
     * @param rack Pointer to the rack that was modified
     * @param slotIndex The index of the slot that was modified
     * @param gearItem Pointer to the gear item that was modified
     * @param controlIndex The index of the control that was modified
     */
    virtual void onGearControlChanged(Rack *rack, int slotIndex, GearItem *gearItem, int controlIndex) = 0;

    /**
     * @brief Called when gear items are rearranged in the rack.
     *
     * @param rack Pointer to the rack that was modified
     * @param sourceSlotIndex The index of the source slot
     * @param targetSlotIndex The index of the target slot
     */
    virtual void onGearItemsRearranged(Rack *rack, int sourceSlotIndex, int targetSlotIndex) = 0;

    /**
     * @brief Called when the entire rack state is reset or cleared.
     *
     * @param rack Pointer to the rack that was reset
     */
    virtual void onRackStateReset(Rack *rack) = 0;

    /**
     * @brief Called when a preset is loaded into the rack.
     *
     * @param rack Pointer to the rack that was loaded
     * @param presetName The name of the preset that was loaded
     */
    virtual void onPresetLoaded(Rack *rack, const juce::String &presetName) = 0;

    /**
     * @brief Called when a preset is saved from the rack.
     *
     * @param rack Pointer to the rack that was saved
     * @param presetName The name of the preset that was saved
     */
    virtual void onPresetSaved(Rack *rack, const juce::String &presetName) = 0;
};