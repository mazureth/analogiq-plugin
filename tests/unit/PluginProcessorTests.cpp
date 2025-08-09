#include <JuceHeader.h>
#include "AnalogIQProcessor.h"
#include "AnalogIQEditor.h"
#include "GearItem.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class AnalogIQProcessorTests : public juce::UnitTest
{
public:
    AnalogIQProcessorTests() : UnitTest("AnalogIQProcessorTests") {}

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
    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem)
    {
        mockFetcher.reset();
        mockFileSystem.reset();

        // Use static test image data to prevent JUCE leak detection
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

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
                "category": "compressor",
                    "version": "1.0.0",
                    "faceplateImage": "assets/faceplates/la2a-compressor-1.0.0.jpg",
                    "controls": [
                        {
                        "id": "peak_reduction",
                            "label": "Peak Reduction",
                            "type": "knob",
                        "position": {"x": 0, "y": 0, "width": 50, "height": 50}
                        },
                        {
                            "id": "gain",
                            "label": "Gain",
                            "type": "knob",
                        "position": {"x": 60, "y": 0, "width": 50, "height": 50}
                    },
                    {
                        "id": "input",
                        "label": "Input",
                        "type": "switch",
                        "position": {"x": 120, "y": 0, "width": 50, "height": 50}
                    },
                    {
                        "id": "comp_limit",
                        "label": "Comp/Limit",
                        "type": "button",
                        "position": {"x": 180, "y": 0, "width": 50, "height": 50}
                        }
                    ]
                    })");
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        // Create local instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Construction");
        {
            setUpMocks(mockFetcher, mockFileSystem);
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            expectEquals(processor.getName(), juce::String("AnalogIQ"),
                         "Processor name should be AnalogIQ, but got: " + processor.getName());
        }

        beginTest("Plugin State Management");
        {
            setUpMocks(mockFetcher, mockFileSystem);
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);

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
            setUpMocks(mockFetcher, mockFileSystem);
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);

            // Create editor and get rack (using testing constructor to avoid gear library loading)
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
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
                        TestImageHelper::getEmptyTestTags(),
                        mockFetcher,
                        mockFileSystem,
                        cacheManager,
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

                // Editor will be cleaned up automatically by unique_ptr
            }
        }

        beginTest("Gear Load Instance");
        {
            setUpMocks(mockFetcher, mockFileSystem);

            // Create processor and editor (using testing constructor to avoid gear library loading)
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
            expect(editor != nullptr, "Editor should be created");

            // Get the rack
            auto *rack = editor->getRack();
            expect(rack != nullptr, "Rack should exist");

            // Create a gear item that will be used for both instances
            GearItem testGear(
                "la2a-compressor",
                "LA2A Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                TestImageHelper::getEmptyTestTags(),
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to the gear item
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Gain", juce::Rectangle<float>(50, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Switch, "Comp/Limit", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Button, "On/Off", juce::Rectangle<float>(60, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Fader, "Frequency", juce::Rectangle<float>(60, 0, 50, 50)));
            testGear.createInstance(testGear.unitId);

            // Add the test gear to the gear library so it can be loaded later
            processor.getGearLibrary().addItem(testGear.unitId, testGear.name, testGear.categoryString, testGear.manufacturer, testGear.manufacturer, true);

            // Get the added item and update it with our controls
            auto *addedItem = processor.getGearLibrary().getGearItemByUnitId(testGear.unitId);
            if (addedItem != nullptr)
            {
                addedItem->controls = testGear.controls;
            }

            // Step 1: Add 1 instances of the same unit to different slots
            // Create first instance for slot 0
            GearItem instance1 = testGear;
            instance1.createInstance(testGear.unitId);
            // Set control values for first instance
            if (instance1.controls.size() >= 2)
            {
                instance1.controls.getReference(0).value = 0.3f; // Peak Reduction
                instance1.controls.getReference(1).value = 0.4f; // Gain
                instance1.controls.getReference(2).value = 0.5f; // Comp/Limit
                instance1.controls.getReference(3).value = 0.6f; // On/Off
                instance1.controls.getReference(4).value = 0.7f; // Frequency
            }

            // Set the instances in a slots
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->setGearItem(&instance1);
            }

            // Verify instance is in its slot
            if (auto *slot0 = rack->getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have first instance");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->isInstance, "Slot 0 item should be an instance");
                    expectEquals(item->controls[0].value, 0.3f, "Slot 0 Peak Reduction should be 0.3");
                    expectEquals(item->controls[1].value, 0.4f, "Slot 0 Gain should be 0.4");
                    expectEquals(item->controls[2].value, 0.5f, "Slot 0 Comp/Limit should be 0.5");
                    expectEquals(item->controls[3].value, 0.6f, "Slot 0 On/Off should be 0.6");
                    expectEquals(item->controls[4].value, 0.7f, "Slot 0 Frequency should be 0.7");
                }
            }

            // Step 2: Simulate closing the UI by saving state
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
            processor.saveInstanceStateFromRack(rack, instanceTree);

            // Step 3: Simulate reopening the UI by clearing the rack
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->clearGearItem();
            }

            // Verify rack is empty
            expect(rack->getSlot(0)->getGearItem() == nullptr, "Slot 0 should be empty after clearing");

            // Step 4: Load the saved state (simulating reopening the UI)
            processor.loadInstanceState(rack);

            // Process JUCE message loop to allow async schema load and callbacks to complete
            if (juce::MessageManager::getInstanceWithoutCreating() != nullptr)
            {
                // Use a simple delay to allow async operations to complete
                juce::Thread::sleep(100);
            }

            // Step 5: Verify the instance is restored with correct settings
            if (auto *slot0 = rack->getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have restored instance");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->isInstance, "Restored item in slot 0 should be an instance");
                    expectEquals(item->controls[0].value, 0.3f, "Slot 0 Peak Reduction should be restored to 0.3");
                    expectEquals(item->controls[1].value, 0.4f, "Slot 0 Gain should be restored to 0.4");
                    expectEquals(item->controls[2].value, 0.5f, "Slot 0 Comp/Limit should be restored to 0.5");
                    expectEquals(item->controls[3].value, 0.6f, "Slot 0 On/Off should be restored to 0.6");
                    expectEquals(item->controls[4].value, 0.7f, "Slot 0 Frequency should be restored to 0.7");
                }
            }
        }

        beginTest("Multiple Instance Loading");
        {
            setUpMocks(mockFetcher, mockFileSystem);

            // Create processor and editor (using testing constructor to avoid gear library loading)
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
            expect(editor != nullptr, "Editor should be created");

            // Get the rack
            auto *rack = editor->getRack();
            expect(rack != nullptr, "Rack should exist");

            // Create a gear item that will be used for both instances
            GearItem testGear(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                TestImageHelper::getEmptyTestTags(),
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to the gear item
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.controls.add(GearControl(GearControl::Type::Knob, "Gain", juce::Rectangle<float>(50, 0, 50, 50)));
            testGear.createInstance(testGear.unitId);

            // Add the test gear to the gear library so it can be loaded later
            processor.getGearLibrary().addItem(testGear.unitId, testGear.name, testGear.categoryString, testGear.manufacturer, testGear.manufacturer, true);

            // Get the added item and update it with our controls
            auto *addedItem = processor.getGearLibrary().getGearItemByUnitId(testGear.unitId);
            if (addedItem != nullptr)
            {
                addedItem->controls = testGear.controls;
            }

            // Step 1: Add 2 instances of the same unit to different slots
            // Create first instance for slot 0
            GearItem instance1 = testGear;
            instance1.createInstance(testGear.unitId);
            // Set control values for first instance
            if (instance1.controls.size() >= 2)
            {
                instance1.controls.getReference(0).value = 0.3f; // Peak Reduction
                instance1.controls.getReference(1).value = 0.4f; // Gain
            }

            // Create second instance for slot 1
            GearItem instance2 = testGear;
            instance2.createInstance(testGear.unitId);
            // Set control values for second instance
            if (instance2.controls.size() >= 2)
            {
                instance2.controls.getReference(0).value = 0.7f; // Peak Reduction
                instance2.controls.getReference(1).value = 0.8f; // Gain
            }

            // Set the instances in their respective slots
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->setGearItem(&instance1);
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                slot1->setGearItem(&instance2);
            }

            // Verify both instances are in place with different settings
            if (auto *slot0 = rack->getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have first instance");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->isInstance, "Slot 0 item should be an instance");
                    expectEquals(item->controls[0].value, 0.3f, "Slot 0 Peak Reduction should be 0.3");
                    expectEquals(item->controls[1].value, 0.4f, "Slot 0 Gain should be 0.4");
                }
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                expect(slot1->getGearItem() != nullptr, "Slot 1 should have second instance");
                if (auto *item = slot1->getGearItem())
                {
                    expect(item->isInstance, "Slot 1 item should be an instance");
                    expectEquals(item->controls[0].value, 0.7f, "Slot 1 Peak Reduction should be 0.7");
                    expectEquals(item->controls[1].value, 0.8f, "Slot 1 Gain should be 0.8");
                }
            }

            // Step 2: Simulate closing the UI by saving state
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
            processor.saveInstanceStateFromRack(rack, instanceTree);

            // Step 3: Simulate reopening the UI by clearing the rack
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->clearGearItem();
            }
            if (auto *slot1 = rack->getSlot(1))
            {
                slot1->clearGearItem();
            }

            // Verify rack is empty
            expect(rack->getSlot(0)->getGearItem() == nullptr, "Slot 0 should be empty after clearing");
            expect(rack->getSlot(1)->getGearItem() == nullptr, "Slot 1 should be empty after clearing");

            // Step 4: Load the saved state (simulating reopening the UI)
            processor.loadInstanceState(rack);

            // Process JUCE message loop to allow async schema load and callbacks to complete
            if (juce::MessageManager::getInstanceWithoutCreating() != nullptr)
            {
                // Use a simple delay to allow async operations to complete
                juce::Thread::sleep(100);
            }

            // Step 5: Verify both instances are restored with correct settings
            if (auto *slot0 = rack->getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have restored instance");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->isInstance, "Restored item in slot 0 should be an instance");
                    expectEquals(item->controls[0].value, 0.3f, "Slot 0 Peak Reduction should be restored to 0.3");
                    expectEquals(item->controls[1].value, 0.4f, "Slot 0 Gain should be restored to 0.4");
                }
            }

            if (auto *slot1 = rack->getSlot(1))
            {
                expect(slot1->getGearItem() != nullptr, "Slot 1 should have restored instance");
                if (auto *item = slot1->getGearItem())
                {
                    expect(item->isInstance, "Restored item in slot 1 should be an instance");
                    expectEquals(item->controls[0].value, 0.7f, "Slot 1 Peak Reduction should be restored to 0.7");
                    expectEquals(item->controls[1].value, 0.8f, "Slot 1 Gain should be restored to 0.8");
                }
            }
        }

        beginTest("Notes Panel Persistence");
        {
            setUpMocks(mockFetcher, mockFileSystem);
            // Create processor and editor (using testing constructor to avoid gear library loading)
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
            expect(editor != nullptr, "Editor should be created");

            // Attach the editor to the processor so notes can be saved/loaded
            processor.lastCreatedEditor = editor.get();

            // Get the notes panel
            auto *notesPanel = editor->getNotesPanel();
            expect(notesPanel != nullptr, "Notes panel should exist");

            // Set notes content
            juce::String testNotes = "Test session notes";
            notesPanel->setText(testNotes);

            // Get the rack and add a gear item
            auto *rack = editor->getRack();
            expect(rack != nullptr, "Rack should exist");

            // Save the instance state (includes notes)
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);
            processor.saveInstanceStateFromRack(rack, instanceTree);

            notesPanel->setText("");

            // Load the instance state
            processor.loadInstanceState(rack);

            // Wait for async loading to complete
            juce::Thread::sleep(100); // Wait 100ms for async operations

            // Verify notes were restored
            expectEquals(notesPanel->getText(), testNotes, "Notes content should be restored");
        }

        beginTest("Instance Validation and Null Handling");
        {
            setUpMocks(mockFetcher, mockFileSystem);
            // Create processor and editor (using testing constructor to avoid gear library loading)
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
            expect(editor != nullptr, "Editor should be created");

            // Get the rack
            auto *rack = editor->getRack();
            expect(rack != nullptr, "Rack should exist");

            // Test loading with invalid gear library item (nullptr)
            // This tests our null pointer handling in the lambda callbacks
            auto instanceTree = processor.getState().state.getOrCreateChildWithName("instances", nullptr);

            // Create a slot tree with invalid sourceUnitId
            auto slotTree = instanceTree.getOrCreateChildWithName("slot_0", nullptr);
            slotTree.setProperty("sourceUnitId", "invalid-unit-id", nullptr);
            slotTree.setProperty("instanceId", "test-instance", nullptr);

            // Add some control data
            auto controlsTree = slotTree.getOrCreateChildWithName("controls", nullptr);
            auto controlTree = controlsTree.getOrCreateChildWithName("control_0", nullptr);
            controlTree.setProperty("value", 0.5f, nullptr);
            controlTree.setProperty("initialValue", 0.5f, nullptr);

            // Try to load the instance state - should handle gracefully
            processor.loadInstanceState(rack);

            // Wait for async loading to complete
            juce::Thread::sleep(100); // Wait 100ms for async operations

            // Verify that the slot is empty (invalid gear item wasn't loaded)
            if (auto *slot = rack->getSlot(0))
            {
                expect(slot->getGearItem() == nullptr, "Slot should be empty for invalid gear item");
            }

            // Test loading with valid gear item but invalid control data
            // Create a valid gear item
            GearItem testGear(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                TestImageHelper::getEmptyTestTags(),
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            testGear.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            testGear.createInstance(testGear.unitId);

            // Add the gear item to the processor's gear library so it can be loaded
            processor.getGearLibrary().addItem(testGear.unitId, testGear.name, testGear.categoryString, testGear.name, testGear.manufacturer, true);

            // Set the gear item in the slot
            if (auto *slot = rack->getSlot(0))
            {
                slot->setGearItem(&testGear);
            }

            // Save the instance state
            processor.saveInstanceStateFromRack(rack, instanceTree);

            // Clear the slot
            if (auto *slot = rack->getSlot(0))
            {
                slot->clearGearItem();
            }

            // Load the instance state
            processor.loadInstanceState(rack);

            // Wait for async loading to complete
            juce::Thread::sleep(100); // Wait 100ms for async operations

            // Verify the instance was loaded successfully
            if (auto *slot = rack->getSlot(0))
            {
                expect(slot->getGearItem() != nullptr, "Slot should have a gear item after loading");
                if (auto *item = slot->getGearItem())
                {
                    expect(item->isInstance, "Restored item should be an instance");
                    expect(item->instanceId.isNotEmpty(), "Restored instance should have an ID");
                }
            }
        }

        beginTest("Gear Reset Instance");
        {
            setUpMocks(mockFetcher, mockFileSystem);
            // Create processor and editor (using testing constructor to avoid gear library loading)
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            auto editor = std::make_unique<AnalogIQEditor>(processor, cacheManager, presetManager, true);
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
                TestImageHelper::getEmptyTestTags(),
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to first gear
            testGear1.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            auto &peakReduction1 = testGear1.controls.getReference(0);
            peakReduction1.value = 180;
            peakReduction1.initialValue = 180; // Set initial value for reset functionality

            // Create instance after setting value
            testGear1.createInstance(testGear1.unitId);

            // Add first gear to processor's gear library so resetAllInstances can find it
            processor.getGearLibrary().addItem(testGear1.unitId, testGear1.name, testGear1.categoryString, testGear1.name, testGear1.manufacturer, true);

            GearItem testGear2(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                TestImageHelper::getEmptyTestTags(),
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor);

            // Add controls to second gear
            testGear2.controls.add(GearControl(GearControl::Type::Knob, "Peak Reduction", juce::Rectangle<float>(0, 0, 50, 50)));
            auto &peakReduction2 = testGear2.controls.getReference(0);
            peakReduction2.value = 180;
            peakReduction2.initialValue = 180; // Set initial value for reset functionality

            // Create instance after setting value
            testGear2.createInstance(testGear2.unitId);

            // Add second gear to processor's gear library so resetAllInstances can find it
            processor.getGearLibrary().addItem(testGear2.unitId, testGear2.name, testGear2.categoryString, testGear2.name, testGear2.manufacturer, true);

            // Set the gear items in slots 0 and 1
            if (auto *slot0 = rack->getSlot(0))
            {
                slot0->setGearItem(&testGear1);
            }

            // attach editor so processor can access the rack
            processor.lastCreatedEditor = editor.get();

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

static AnalogIQProcessorTests analogIQProcessorTests;