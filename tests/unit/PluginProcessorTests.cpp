#include <JuceHeader.h>
#include "../../Source/PluginProcessor.h"
#include "../../Source/GearItem.h"

class PluginProcessorTests : public juce::UnitTest
{
public:
    PluginProcessorTests() : UnitTest("PluginProcessorTests") {}

    // Helper function to create a test gear instance with known state
    GearItem createTestGearInstance()
    {
        GearItem item;
        item.name = "Test EQ";
        item.type = GearType::Series500;
        item.manufacturer = "Test Co";
        item.category = GearCategory::EQ;
        item.categoryString = "equalizer";
        item.version = "1.0";
        item.slotSize = 1;
        item.unitId = "test.eq.1";

        // Create an instance of this gear
        item.createInstance(item.unitId);

        return item;
    }

    // Helper function to verify a gear instance matches our test state
    void verifyTestGearInstance(const GearItem &item)
    {
        expectEquals(item.name, juce::String("Test EQ"),
                     "Gear name should be Test EQ, but got: " + item.name);
        expect(item.type == GearType::Series500,
               "Type should be Series500, but got: " + juce::String(static_cast<int>(item.type)));
        expectEquals(item.manufacturer, juce::String("Test Co"),
                     "Manufacturer should be Test Co, but got: " + item.manufacturer);
        expect(item.category == GearCategory::EQ,
               "Category should be EQ, but got: " + juce::String(static_cast<int>(item.category)));
        expectEquals(item.categoryString, juce::String("equalizer"),
                     "Category string should be equalizer, but got: " + item.categoryString);
        expectEquals(item.version, juce::String("1.0"),
                     "Version should be 1.0, but got: " + item.version);
        expectEquals(item.slotSize, 1,
                     "Slot size should be 1, but got: " + juce::String(item.slotSize));
        expect(item.isInstance,
               "Item should be an instance, but isInstance is: " + juce::String(item.isInstance ? "true" : "false"));
        expectEquals(item.unitId, juce::String("test.eq.1"),
                     "Unit ID should be test.eq.1, but got: " + item.unitId);
    }

    void runTest() override
    {
        beginTest("Construction");
        {
            AnalogIQProcessor processor;
            expectEquals(processor.getName(), juce::String("AnalogIQ"),
                         "Processor name should be AnalogIQ, but got: " + processor.getName());
        }

        beginTest("Plugin State Management");
        {
            AnalogIQProcessor processor;

            // Save initial state
            juce::MemoryBlock state;
            processor.getStateInformation(state);
            expect(state.getSize() > 0,
                   "State should have a size greater than 0, but got: " + juce::String(state.getSize()));

            // Load the state back
            processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));

            // Save state again and compare
            juce::MemoryBlock newState;
            processor.getStateInformation(newState);
            expect(state == newState,
                   "State should remain unchanged after save/load cycle");
        }

        beginTest("Gear Instance State Management");
        {
            AnalogIQProcessor processor;

            // Create and save initial test gear instance
            auto testGear = createTestGearInstance();
            processor.saveInstanceState();

            // Reset instances
            processor.resetAllInstances();

            // Load saved state
            processor.loadInstanceState();

            // Get the loaded gear instance and verify its state
            // TODO: Need to add a way to get the loaded gear instance from the processor
            // This might require adding a test helper method to the processor
        }

        beginTest("Instance Reset");
        {
            AnalogIQProcessor processor;

            // Create test gear instance
            auto testGear = createTestGearInstance();

            // Reset all instances
            processor.resetAllInstances();

            // Verify the gear instance is back to default state
            // TODO: Need to add a way to get the reset gear instance from the processor
            // This might require adding a test helper method to the processor
        }
    }
};

static PluginProcessorTests pluginProcessorTests;