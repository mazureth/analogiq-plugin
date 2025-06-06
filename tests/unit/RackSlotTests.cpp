#include <JuceHeader.h>
#include "../../Source/RackSlot.h"
#include "GearItem.h"

class RackSlotTests : public juce::UnitTest
{
public:
    RackSlotTests() : UnitTest("RackSlotTests") {}

    void runTest() override
    {
        beginTest("Initial State");
        {
            RackSlot slot(0);
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Gear Item Management");
        {
            RackSlot slot(0);
            auto item = std::make_unique<GearItem>();
            item->name = "Test Gear";
            item->type = GearType::Series500;
            item->category = GearCategory::EQ;
            slot.setGearItem(item.get());
            expect(!slot.isAvailable(), "Slot should not be available");
            expect(slot.getGearItem()->name == "Test Gear", "Gear item name should match");
        }

        beginTest("Clear Gear Item");
        {
            RackSlot slot(0);
            auto item = std::make_unique<GearItem>();
            item->name = "Test Gear";
            item->type = GearType::Series500;
            item->category = GearCategory::EQ;
            slot.setGearItem(item.get());
            slot.clearGearItem();
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Instance Management");
        {
            RackSlot slot(0);
            auto gearItem = std::make_unique<GearItem>();
            gearItem->unitId = "test_gear";
            gearItem->name = "Test Gear";
            gearItem->manufacturer = "Test Manufacturer";
            gearItem->categoryString = "Effects";
            gearItem->type = GearType::Series500;
            gearItem->category = GearCategory::EQ;
            slot.setGearItem(gearItem.get());
            slot.createInstance();
            expect(slot.isInstance(), "Slot should be an instance");
            expect(!slot.getInstanceId().isEmpty(), "Instance ID should not be empty");

            // Store the instance ID before reset
            auto instanceId = slot.getInstanceId();

            // Reset the instance
            slot.resetToSource();

            // Verify instance state is preserved
            expect(slot.isInstance(), "Slot should still be an instance after reset");
            expect(slot.getInstanceId() == instanceId, "Instance ID should be preserved after reset");
        }
    }
};

static RackSlotTests rackSlotTestsInstance;