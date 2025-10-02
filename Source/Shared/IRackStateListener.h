#pragma once

#include <juce_core/juce_core.h>

// Forward declarations
class Rack;
class GearItem;

/**
 * Interface for components needing rack state change notifications.
 * Implements observer pattern for rack state changes including gear item modifications, control changes, and preset operations.
 */
class RackStateListener
{
public:
    virtual ~RackStateListener() = default;

    // Gear item lifecycle events
    virtual void onGearItemAdded(Rack *rack, int slotIndex, const GearItem *gearItem) = 0;
    virtual void onGearItemRemoved(Rack *rack, int slotIndex) = 0;

    // Control modification events
    virtual void onGearControlChanged(Rack *rack, int slotIndex, const GearItem *gearItem, int controlIndex) = 0;

    // Rack structure events
    virtual void onGearItemsRearranged(Rack *rack, int sourceSlotIndex, int targetSlotIndex) = 0;

    // Rack state reset events
    virtual void onRackStateReset(Rack *rack) = 0;

    // Preset operation events
    virtual void onPresetLoaded(Rack *rack, const juce::String &presetName) = 0;
    virtual void onPresetSaved(Rack *rack, const juce::String &presetName) = 0;
};
