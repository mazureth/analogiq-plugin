#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"

class RackSlotTests : public juce::UnitTest
{
public:
    RackSlotTests() : UnitTest("RackSlot Tests") {}

    void runTest() override
    {
        beginTest("Initial State");
        {
            RackSlot slot(0);
            expect(slot.isAvailable());
        }

        beginTest("Gear Item Management");
        {
            RackSlot slot(0);
            auto item = std::make_unique<GearItem>();
            item->name = "Test Gear";
            item->type = GearType::Series500;
            item->category = GearCategory::EQ;
            slot.setGearItem(item.get());
            expect(!slot.isAvailable());
            expect(slot.getGearItem()->name == "Test Gear");
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
            expect(slot.isAvailable());
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
            expect(slot.isInstance());
            slot.resetToSource();
            expect(!slot.isInstance());
        }
    }
};

static RackSlotTests rackSlotTestsInstance;