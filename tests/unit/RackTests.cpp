#include <JuceHeader.h>
#include "Rack.h"
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class RackTests : public juce::UnitTest
{
public:
    RackTests() : UnitTest("RackTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        mockFetcher.reset();

        beginTest("Initial State");
        {
            Rack rack(mockFetcher);
            expectEquals(rack.getNumSlots(), 16, "Rack should have zero slots");
        }

        beginTest("Slot Management");
        {
            Rack rack(mockFetcher);
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
                expect(!slot->isAvailable(), "Slot should not be available");
                expect(slot->getGearItem()->name == "Test Gear", "Gear item name should match");
            }
        }

        beginTest("Instance Management");
        {
            Rack rack(mockFetcher);
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
                expect(rack.isInstance(0), "Slot should be an instance");
                expect(!rack.getInstanceId(0).isEmpty(), "Instance ID should not be empty");

                // Store the instance ID before reset
                auto instanceId = rack.getInstanceId(0);

                // Reset the instance
                rack.resetToSource(0);

                // Verify instance state is preserved
                expect(rack.isInstance(0), "Slot should still be an instance after reset");
                expect(rack.getInstanceId(0) == instanceId, "Instance ID should be preserved after reset");
            }
        }

        beginTest("Multiple Slots");
        {
            Rack rack(mockFetcher);
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
                expect(rack.isInstance(0), "Slot 1 should be an instance");
                expect(rack.isInstance(1), "Slot 2 should be an instance");

                // Store instance IDs before reset
                auto instanceId1 = rack.getInstanceId(0);
                auto instanceId2 = rack.getInstanceId(1);

                // Reset all instances
                rack.resetAllInstances();

                // Verify instance states are preserved
                expect(rack.isInstance(0), "Slot 1 should still be an instance after reset");
                expect(rack.isInstance(1), "Slot 2 should still be an instance after reset");
                expect(rack.getInstanceId(0) == instanceId1, "Slot 1 instance ID should be preserved after reset");
                expect(rack.getInstanceId(1) == instanceId2, "Slot 2 instance ID should be preserved after reset");
            }
        }
    }
};

static RackTests rackTestsInstance;