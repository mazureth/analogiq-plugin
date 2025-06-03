#include <JuceHeader.h>
#include "../../Source/GearItem.h"

class GearItemTests : public juce::UnitTest
{
public:
    GearItemTests() : UnitTest("GearItemTests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            GearItem item;
            expect(item.name.isEmpty());
            expect(item.type == GearType::Other);
            expect(item.manufacturer.isEmpty());
        }

        beginTest("Property Assignment");
        {
            GearItem item;
            item.name = "Test Gear";
            item.type = GearType::Series500;
            item.manufacturer = "Test Co";
            item.category = GearCategory::EQ;
            item.categoryString = "equalizer";
            item.version = "1.0";
            item.slotSize = 1;

            expectEquals(item.name, juce::String("Test Gear"));
            expect(item.type == GearType::Series500);
            expectEquals(item.manufacturer, juce::String("Test Co"));
            expect(item.category == GearCategory::EQ);
            expectEquals(item.categoryString, juce::String("equalizer"));
            expectEquals(item.version, juce::String("1.0"));
            expect(item.slotSize == 1);
        }

        beginTest("Instance Creation");
        {
            GearItem item;
            item.name = "Original Gear";
            item.unitId = "test.unit.1";
            item.createInstance(item.unitId);
            expect(item.isInstance);
            expect(item.sourceUnitId == item.unitId);
            expect(item.name == "Original Gear");
            expect(!item.instanceId.isEmpty());
        }

        beginTest("Instance Checking");
        {
            GearItem item;
            item.unitId = "test.unit.1";
            expect(!item.isInstanceOf("test.unit.1"));
            expect(!item.isInstanceOf("test.unit.2"));
            item.createInstance("test.unit.1");
            expect(item.isInstanceOf("test.unit.1"));
            expect(!item.isInstanceOf("test.unit.2"));
        }
    }
};

static GearItemTests gearItemTests;