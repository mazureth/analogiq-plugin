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
            expectEquals(item.name, juce::String(), "New item should have empty name");
            expect(item.type == GearType::Series500,
                   "New item should have GearType::Other, but got: " + juce::String(static_cast<int>(item.type)));
            expectEquals(item.manufacturer, juce::String(), "New item should have empty manufacturer");
            expect(item.category == GearCategory::EQ,
                   "New item should have GearCategory::Other, but got: " + juce::String(static_cast<int>(item.category)));
            expectEquals(item.categoryString, juce::String(), "New item should have empty category string");
            expectEquals(item.version, juce::String(), "New item should have empty version");
            expectEquals(item.slotSize, 0, "New item should have zero slot size");
            expect(!item.isInstance,
                   "New item should not be an instance, but isInstance is: " + juce::String(item.isInstance ? "true" : "false"));
            expectEquals(item.unitId, juce::String(), "New item should have empty unit ID");
            expectEquals(item.sourceUnitId, juce::String(), "New item should have empty source unit ID");
            expectEquals(item.instanceId, juce::String(), "New item should have empty instance ID");
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

            expectEquals(item.name, juce::String("Test Gear"), "Name should match assigned value");
            expect(item.type == GearType::Series500,
                   "Type should be Series500, but got: " + juce::String(static_cast<int>(item.type)));
            expectEquals(item.manufacturer, juce::String("Test Co"), "Manufacturer should match assigned value");
            expect(item.category == GearCategory::EQ,
                   "Category should be EQ, but got: " + juce::String(static_cast<int>(item.category)));
            expectEquals(item.categoryString, juce::String("equalizer"), "Category string should match assigned value");
            expectEquals(item.version, juce::String("1.0"), "Version should match assigned value");
            expectEquals(item.slotSize, 1, "Slot size should match assigned value");
        }

        beginTest("Instance Creation");
        {
            GearItem item;
            item.name = "Original Gear";
            item.unitId = "test.unit.1";
            item.createInstance(item.unitId);

            expect(item.isInstance,
                   "Item should be marked as instance after creation, but isInstance is: " + juce::String(item.isInstance ? "true" : "false"));
            expectEquals(item.sourceUnitId, juce::String("test.unit.1"), "Source unit ID should match original unit ID");
            expectEquals(item.name, juce::String("Original Gear"), "Name should remain unchanged");
            expect(item.instanceId.isNotEmpty(),
                   "Instance ID should not be empty, but got: " + item.instanceId);
        }

        beginTest("Instance Checking");
        {
            GearItem item;
            item.unitId = "test.unit.1";

            expect(!item.isInstanceOf("test.unit.1"),
                   "Non-instance should not be instance of test.unit.1, but isInstanceOf returned: " +
                       juce::String(item.isInstanceOf("test.unit.1") ? "true" : "false"));
            expect(!item.isInstanceOf("test.unit.2"),
                   "Non-instance should not be instance of test.unit.2, but isInstanceOf returned: " +
                       juce::String(item.isInstanceOf("test.unit.2") ? "true" : "false"));

            item.createInstance("test.unit.1");

            expect(item.isInstanceOf("test.unit.1"),
                   "Instance should be instance of its source unit, but isInstanceOf returned: " +
                       juce::String(item.isInstanceOf("test.unit.1") ? "true" : "false"));
            expect(!item.isInstanceOf("test.unit.2"),
                   "Instance should not be instance of different unit, but isInstanceOf returned: " +
                       juce::String(item.isInstanceOf("test.unit.2") ? "true" : "false"));
        }
    }
};

static GearItemTests gearItemTests;