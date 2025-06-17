#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GearItem.h"
#include "PluginEditor.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class PluginProcessorTests : public juce::UnitTest
{
public:
    PluginProcessorTests() : UnitTest("PluginProcessorTests") {}

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

    // Helper function to set up all mock responses
    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        mockFetcher.reset();

        // Create a simple JPEG image for testing
        juce::Image testImage(juce::Image::RGB, 24, 24, true);
        {
            juce::Graphics g(testImage);
            g.fillAll(juce::Colours::darkgrey);
            g.setColour(juce::Colours::white);
            g.drawText("Test", testImage.getBounds(), juce::Justification::centred, true);
        }

        // Convert to JPEG format
        juce::MemoryOutputStream stream;
        juce::JPEGImageFormat jpegFormat;
        jpegFormat.setQuality(0.8f);
        jpegFormat.writeImageToStream(testImage, stream);
        juce::MemoryBlock imageData(stream.getData(), stream.getDataSize());

        // Set up mock response for the units index
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json",
            R"({
                "units": [
                    {
                        "unitId": "la2a-compressor",
                        "name": "LA-2A Tube Compressor",
                        "manufacturer": "Universal Audio",
                        "category": "compressor",
                        "version": "1.0.0",
                        "schemaPath": "units/la2a-compressor-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                        "tags": [
                            "compressor",
                            "tube",
                            "optical",
                            "vintage",
                            "hardware"
                        ]
                    }
                ]
            })");

        // Set up mock responses for images using binary data
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
            imageData);

        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            imageData);

        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
            imageData);

        // Set up mock response for the compressor schema
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/la2a-compressor-1.0.0.json",
            R"({
                    "unitId": "la2a-compressor",
                    "name": "LA-2A Tube Compressor",
                    "manufacturer": "Universal Audio",
                    "tags": [
                        "compressor",
                        "tube",
                        "optical",
                        "vintage",
                        "hardware"
                    ],
                    "version": "1.0.0",
                    "category": "compressor",
                    "formFactor": "19-inch-rack",
                    "faceplateImage": "assets/faceplates/la2a-compressor-1.0.0.jpg",
                    "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                    "width": 1900,
                    "height": 525,
                    "controls": [
                        {
                            "id": "peak-reduction",
                            "label": "Peak Reduction",
                            "type": "knob",
                            "position": {
                            "x": 0.68,
                            "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        },
                        {
                            "id": "gain",
                            "label": "Gain",
                            "type": "knob",
                            "position": {
                                "x": 0.257,
                                "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        }
                    ]
                    })");
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();

        beginTest("Construction");
        {
            setUpMocks(mockFetcher);
            AnalogIQProcessor processor(mockFetcher);
            expectEquals(processor.getName(), juce::String("AnalogIQ"),
                         "Processor name should be AnalogIQ, but got: " + processor.getName());
        }

        beginTest("Plugin State Management");
        {
            setUpMocks(mockFetcher);
            AnalogIQProcessor processor(mockFetcher);

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
            setUpMocks(mockFetcher);
            AnalogIQProcessor processor(mockFetcher);

            // Create editor and get rack
            auto *editor = dynamic_cast<AnalogIQEditor *>(processor.createEditor());
            expect(editor != nullptr, "Editor should be created");

            if (editor != nullptr)
            {
                auto *rack = editor->getRack();
                expect(rack != nullptr, "Rack should exist");

                if (rack != nullptr)
                {
                    // Create a gear item using the constructor
                    GearItem testGear(
                        "la2a-compressor",
                        "LA-2A Tube Compressor",
                        "Universal Audio",
                        "compressor",
                        "1.0.0",
                        "units/la2a-compressor-1.0.0.json",
                        "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                        juce::StringArray({"compressor", "tube", "optical", "vintage", "hardware"}),
                        mockFetcher,
                        GearType::Rack19Inch,
                        GearCategory::Compressor);
                    testGear.createInstance(testGear.unitId);

                    // Add the LA-2A controls
                    testGear.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
                    testGear.controls.add(GearControl(GearControl::Type::Knob, "Gain", juce::Rectangle<float>(60, 0, 50, 50)));

                    // Set control values
                    auto &peakReduction = testGear.controls.getReference(0);
                    peakReduction.value = 200;
                    auto &gain = testGear.controls.getReference(1);
                    gain.value = 70;

                    // Set the gear item in the slot
                    if (auto *slot = rack->getSlot(0))
                    {
                        slot->setGearItem(&testGear);

                        // Create instance in the slot
                        slot->createInstance();

                        // Save the instance state
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
                            expectEquals(controlTree.getProperty("value").toString(), juce::String("200.0"),
                                         "Peak Reduction value should be 200");
                            expectEquals(controlTree.getProperty("initialValue").toString(), juce::String("200.0"),
                                         "Peak Reduction initial value should be 200");
                        }

                        // Verify control 1 values
                        auto control1Tree = controlsTree.getChildWithName("control_1");
                        expect(control1Tree.isValid(), "Control 1 tree should exist in state");
                        if (controlTree.isValid())
                        {
                            expectEquals(control1Tree.getProperty("value").toString(), juce::String("70.0"),
                                         "Gain value should be 70");
                            expectEquals(control1Tree.getProperty("initialValue").toString(), juce::String("70.0"),
                                         "Gain initial value should be 70");
                        }
                    }
                }

                // Clean up
                delete editor;
            }
        }

        beginTest("Gear Load Instance");
        {
            setUpMocks(mockFetcher);
            // Create processor and editor
            AnalogIQProcessor processor(mockFetcher);
            std::unique_ptr<AnalogIQEditor> editor(static_cast<AnalogIQEditor *>(processor.createEditor()));
            auto *rack = editor->getRack();

            // Create a gear item using the constructor
            GearItem testGear(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                juce::StringArray({"compressor", "tube", "optical", "vintage", "hardware"}),
                mockFetcher,
                GearType::Rack19Inch,
                GearCategory::Compressor);
            testGear.createInstance(testGear.unitId);

            // Add the LA-2A controls
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Gain", juce::Rectangle<float>(60, 0, 50, 50)));

            // Set control values
            auto &peakReduction = testGear.controls.getReference(0);
            peakReduction.value = 0.5f;
            auto &gain = testGear.controls.getReference(1);
            gain.value = 0.75f;

            // Set the test gear in slot 0
            if (auto *slot = rack->getSlot(0))
            {
                slot->setGearItem(&testGear);
            }

            // Create an instance
            rack->createInstance(0);

            // Save the instance state
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
            processor.saveInstanceStateFromRack(rack, instanceTree);

            // Clear the slot
            if (auto *slot = rack->getSlot(0))
            {
                slot->clearGearItem();
            }

            // Load the instance state
            processor.loadInstanceState(rack);

            // Verify the instance was restored correctly
            if (auto *slot = rack->getSlot(0))
            {
                expect(slot->getGearItem() != nullptr, "Slot should have a gear item after loading");
                if (auto *item = slot->getGearItem())
                {
                    expect(item->isInstance, "Restored item should be an instance");
                    expect(item->instanceId.isNotEmpty(), "Restored instance should have an ID");
                    expectEquals(item->controls.size(), 2, "Restored instance should have 2 controls");

                    // Verify control values were restored
                    expectEquals(item->controls[0].value, 0.5f, "Peak Reduction value should be restored");
                    expectEquals(item->controls[1].value, 0.75f, "Gain value should be restored");
                }
            }
        }

        beginTest("Gear Reset Instance");
        {
            setUpMocks(mockFetcher);
            // Create processor and editor
            AnalogIQProcessor processor(mockFetcher);
            auto editor = std::unique_ptr<AnalogIQEditor>(dynamic_cast<AnalogIQEditor *>(processor.createEditor()));
            expect(editor != nullptr, "Editor should be created");

            // Get the rack
            auto *rack = editor->getRack();
            expect(rack != nullptr, "Rack should exist");

            // Create two gear items using the constructor
            GearItem testGear1(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                juce::StringArray({"compressor", "tube", "optical", "vintage", "hardware"}),
                mockFetcher,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to first gear
            testGear1.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            auto &peakReduction1 = testGear1.controls.getReference(0);
            peakReduction1.value = 180;

            // Create instance after setting value
            testGear1.createInstance(testGear1.unitId);

            GearItem testGear2(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                juce::StringArray({"compressor", "tube", "optical", "vintage", "hardware"}),
                mockFetcher,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to second gear
            testGear2.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            auto &peakReduction2 = testGear2.controls.getReference(0);
            peakReduction2.value = 180;

            // Create instance after setting value
            testGear2.createInstance(testGear2.unitId);

            // Set the gear items in slots 0 and 1
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->setGearItem(&testGear1);
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                slot1->setGearItem(&testGear2);
            }

            // Verify initial values
            if (auto *slot0 = rack->getSlot(0))
            {
                if (auto *item = slot0->getGearItem())
                {
                    expectEquals(item->controls[0].value, 180.0f, "First gear Peak Reduction should have initial value 180");
                }
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                if (auto *item = slot1->getGearItem())
                {
                    expectEquals(item->controls[0].value, 180.0f, "Second gear Peak Reduction should have initial value 180");
                }
            }

            // Change control values
            if (auto *slot0 = rack->getSlot(0))
            {
                if (auto *item = slot0->getGearItem())
                {
                    auto &control = item->controls.getReference(0);
                    control.value = 200;
                }
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                if (auto *item = slot1->getGearItem())
                {
                    auto &control = item->controls.getReference(0);
                    control.value = 100;
                }
            }

            // Reset all instances using the processor's method
            processor.resetAllInstances();

            // Verify values are reset
            if (auto *slot0 = rack->getSlot(0))
            {
                if (auto *item = slot0->getGearItem())
                {
                    expectEquals(item->controls[0].value, 180.0f, "First gear Peak Reduction should be reset to 180");
                }
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                if (auto *item = slot1->getGearItem())
                {
                    expectEquals(item->controls[0].value, 180.0f, "Second gear Peak Reduction should be reset to 180");
                }
            }
        }
    }
};

static PluginProcessorTests pluginProcessorTests;