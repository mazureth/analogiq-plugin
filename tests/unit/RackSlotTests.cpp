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
            auto item = std::make_unique<GearItem>();
            item->name = "Test Gear";
            item->type = GearType::Series500;
            item->category = GearCategory::EQ;
            slot.setGearItem(item.get());
            slot.createInstance();
            expect(slot.isInstance(), "Slot should be an instance");
            slot.resetToSource();
            expect(!slot.isInstance(), "Slot should not be an instance");
        }
    }
};

static RackSlotTests rackSlotTestsInstance;