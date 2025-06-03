#include <JuceHeader.h>
#include "Rack.h"
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"

class RackTests : public juce::UnitTest
{
public:
    RackTests() : UnitTest("RackTests") {}

    void runTest() override
    {
        beginTest("Initial State");
        {
            Rack rack;
            expect(rack.getNumSlots() == 0);
        }

        beginTest("Slot Management");
        {
            Rack rack;
            auto gearItem = std::make_unique<GearItem>();
            gearItem->unitId = "test_gear";
            gearItem->name = "Test Gear";
            gearItem->manufacturer = "Test Manufacturer";
            gearItem->categoryString = "Effects";
            gearItem->type = GearType::Series500;
            gearItem->category = GearCategory::EQ;
            auto slot = rack.getSlot(0);
            if (slot != nullptr)
            {
                slot->setGearItem(gearItem.get());
                expect(!slot->isAvailable());
                expect(slot->getGearItem()->name == "Test Gore");
            }
        }

        beginTest("Instance Management");
        {
            Rack rack;
            auto gearItem = std::make_unique<GearItem>();
            gearItem->unitId = "test_gear";
            gearItem->name = "Test Gear";
            gearItem->manufacturer = "Test Manufacturer";
            gearItem->categoryString = "Effects";
            gearItem->type = GearType::Series500;
            gearItem->category = GearCategory::EQ;
            auto slot = rack.getSlot(0);
            if (slot != nullptr)
            {
                slot->setGearItem(gearItem.get());
                rack.createInstance(0);
                expect(rack.isInstance(0));
                expect(!rack.getInstanceId(0).isEmpty());
                rack.resetToSource(0);
                expect(!rack.isInstance(0));
            }
        }

        beginTest("Multiple Slots");
        {
            Rack rack;
            auto gearItem1 = std::make_unique<GearItem>();
            gearItem1->unitId = "test_gear_1";
            gearItem1->name = "Test Gear 1";
            gearItem1->manufacturer = "Test Manufacturer";
            gearItem1->categoryString = "Effects";
            gearItem1->type = GearType::Series500;
            gearItem1->category = GearCategory::EQ;
            auto gearItem2 = std::make_unique<GearItem>();
            gearItem2->unitId = "test_gear_2";
            gearItem2->name = "Test Gear 2";
            gearItem2->manufacturer = "Test Manufacturer";
            gearItem2->categoryString = "Effects";
            gearItem2->type = GearType::Series500;
            gearItem2->category = GearCategory::EQ;
            auto slot1 = rack.getSlot(0);
            auto slot2 = rack.getSlot(1);
            if (slot1 != nullptr && slot2 != nullptr)
            {
                slot1->setGearItem(gearItem1.get());
                slot2->setGearItem(gearItem2.get());
                rack.createInstance(0);
                rack.createInstance(1);
                expect(rack.isInstance(0));
                expect(rack.isInstance(1));
                rack.resetAllInstances();
                expect(!rack.isInstance(0));
                expect(!rack.isInstance(1));
            }
        }
    }
};

static RackTests rackTestsInstance;