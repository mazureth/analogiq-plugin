/**
 * @file MockRackStateListener.h
 * @brief Mock implementation of RackStateListener for testing.
 */

#pragma once

#include "RackStateListener.h"

/**
 * @brief Mock implementation of RackStateListener for testing.
 */
class MockRackStateListener : public RackStateListener
{
public:
    MockRackStateListener() = default;
    ~MockRackStateListener() override = default;

    void onGearItemAdded(Rack *rack, int slotIndex, GearItem *gearItem) override
    {
        // Mock implementation - does nothing
    }

    void onGearItemRemoved(Rack *rack, int slotIndex) override
    {
        // Mock implementation - does nothing
    }

    void onGearControlChanged(Rack *rack, int slotIndex, GearItem *gearItem, int controlIndex) override
    {
        // Mock implementation - does nothing
    }

    void onGearItemsRearranged(Rack *rack, int sourceSlotIndex, int targetSlotIndex) override
    {
        // Mock implementation - does nothing
    }

    void onRackStateReset(Rack *rack) override
    {
        // Mock implementation - does nothing
    }

    void onPresetLoaded(Rack *rack, const juce::String &presetName) override
    {
        // Mock implementation - does nothing
    }

    void onPresetSaved(Rack *rack, const juce::String &presetName) override
    {
        // Mock implementation - does nothing
    }
};