#include <JuceHeader.h>
#include "../../Source/PluginProcessor.h"
#include "../../Source/GearItem.h"
#include "../../Source/PluginEditor.h"

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

        beginTest("Gear Save Instance");
        {
            AnalogIQProcessor processor;

            // Create test gear instance with control values
            testGear = createTestGearInstance();
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Control 0", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Control 1", juce::Rectangle<float>(60, 0, 50, 50)));

            // Set control values
            auto &control0 = testGear.controls.getReference(0);
            control0.value = 0.5f;
            control0.initialValue = 0.0f;
            auto &control1 = testGear.controls.getReference(1);
            control1.value = 1.0f;
            control1.initialValue = 1.0f;

            // Create editor and get rack
            auto *editor = dynamic_cast<AnalogIQEditor *>(processor.createEditor());
            expect(editor != nullptr, "Editor should be created");

            if (editor != nullptr)
            {
                auto *rack = editor->getRack();
                expect(rack != nullptr, "Rack should exist");

                if (rack != nullptr)
                {
                    // Set the gear item in the slot
                    if (auto *slot = rack->getSlot(0))
                    {
                        slot->setGearItem(&testGear);

                        // Create instance in the slot
                        slot->createInstance();

                        // Save the instance state
                        logMessage("Saved instance state");
                        auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
                        processor.saveInstanceStateFromRack(rack, instanceTree);

                        // Verify instance state was saved correctly
                        auto &state = processor.getState();

                        auto instanceTreeVerify = state.state.getChildWithName("instances");
                        expect(instanceTreeVerify.isValid(), "Instance tree should exist in state");
                        expect(instanceTreeVerify.getNumChildren() > 0, "Instance tree should have children");

                        // Verify slot tree exists and has correct instance ID
                        auto slotTree = instanceTreeVerify.getChildWithName("slot_0");
                        expect(slotTree.isValid(), "Slot tree should exist in state");
                        expectEquals(slotTree.getProperty("instanceId").toString(), testGear.instanceId,
                                     "Slot tree should have the correct instance ID");

                        // Verify controls tree exists and has correct values
                        auto controlsTree = slotTree.getChildWithName("controls");
                        expect(controlsTree.isValid(), "Controls tree should exist in state");
                        expect(controlsTree.getNumChildren() == 2, "Controls tree should have 2 children");

                        // Verify control values
                        auto controlTree = controlsTree.getChildWithName("control_0");
                        expect(controlTree.isValid(), "Control tree should exist in state");
                        if (controlTree.isValid())
                        {
                            expectEquals(controlTree.getProperty("value").toString(), juce::String("0.5"),
                                         "Control value should be 0.5");
                            expectEquals(controlTree.getProperty("initialValue").toString(), juce::String("0.5"),
                                         "Control initial value should match current value");
                        }

                        // Verify control 1 values
                        auto control1Tree = controlsTree.getChildWithName("control_1");
                        expect(control1Tree.isValid(), "Control 1 tree should exist in state");
                        expectEquals(static_cast<double>(control1Tree.getProperty("value")), 1.0,
                                     "Control 1 value should be 1.0");
                        expectEquals(static_cast<double>(control1Tree.getProperty("initialValue")), 1.0,
                                     "Control 1 initial value should be 1.0");
                    }
                }

                // Clean up
                delete editor;
            }
        }

        beginTest("Gear Load Instance");
        {
            // Create a test gear instance with control values
            GearItem testGear;
            testGear.unitId = "test.eq.1";
            testGear.name = "Test EQ";
            testGear.type = GearType::Series500;
            testGear.manufacturer = "Test Co";
            testGear.category = GearCategory::EQ;
            testGear.categoryString = "equalizer";
            testGear.version = "1.0";
            testGear.slotSize = 1;

            // Add some controls
            GearControl knob1;
            knob1.name = "Frequency";
            knob1.type = GearControl::Type::Knob;
            knob1.value = 0.5f;
            knob1.initialValue = 0.0f;
            testGear.controls.add(knob1);

            GearControl knob2;
            knob2.name = "Gain";
            knob2.type = GearControl::Type::Knob;
            knob2.value = 0.75f;
            knob2.initialValue = 0.0f;
            testGear.controls.add(knob2);

            // Create processor and editor
            AnalogIQProcessor processor;
            std::unique_ptr<AnalogIQEditor> editor(static_cast<AnalogIQEditor *>(processor.createEditor()));
            auto *rack = editor->getRack();

            // Set the test gear in slot 0
            logMessage("Set testGear in slot 0");
            if (auto *slot = rack->getSlot(0))
            {
                slot->setGearItem(&testGear);
            }

            // Create an instance
            logMessage("Called createInstance() on slot 0");
            rack->createInstance(0);

            // Save the instance state
            logMessage("Saved instance state");
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
            processor.saveInstanceStateFromRack(rack, instanceTree);

            // Clear the slot
            logMessage("Cleared slot");
            if (auto *slot = rack->getSlot(0))
            {
                slot->clearGearItem();
            }

            // Load the instance state
            logMessage("Loading instance state");
            processor.loadInstanceState(rack);

            // Verify the instance was restored correctly
            logMessage("Verifying restored instance");
            if (auto *slot = rack->getSlot(0))
            {
                expect(slot->getGearItem() != nullptr, "Slot should have a gear item after loading");
                if (auto *item = slot->getGearItem())
                {
                    expect(item->isInstance, "Restored item should be an instance");
                    expect(item->instanceId.isNotEmpty(), "Restored instance should have an ID");
                    expectEquals(item->controls.size(), 2, "Restored instance should have 2 controls");

                    // Verify control values were restored
                    expectEquals(item->controls[0].value, 0.5f, "First control value should be restored");
                    expectEquals(item->controls[1].value, 0.75f, "Second control value should be restored");
                }
            }
        }

        beginTest("Gear Reset Instance");
        {
            AnalogIQProcessor processor;
        }
    }

private:
    GearItem testGear; // Make the gear item persist for the duration of the test
};

static PluginProcessorTests pluginProcessorTests;