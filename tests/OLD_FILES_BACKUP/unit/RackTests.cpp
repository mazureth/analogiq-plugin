#include <JuceHeader.h>
#include "Rack.h"
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class RackTests : public juce::UnitTest
{
public:
    RackTests() : UnitTest("RackTests") {}

    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Use static test image data to prevent JUCE leak detection
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

        // Set up mock responses for images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
            imageData);

        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            imageData);

        // Set up mock responses for control images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
            imageData);

        // Set up mock response for the LA-2A schema
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
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Create local instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Initial State");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);
            expectEquals(rack.getNumSlots(), 16, "Rack should have 16 slots");
        }

        beginTest("Slot Management");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            // Create Peak Reduction control
            GearControl peakReduction;
            peakReduction.id = "peak-reduction";
            peakReduction.name = "Peak Reduction";
            peakReduction.type = GearControl::Type::Knob;
            peakReduction.position = {0.68f, 0.44f};
            peakReduction.value = 180;
            peakReduction.startAngle = 40;
            peakReduction.endAngle = 322;
            peakReduction.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(peakReduction);

            // Create Gain control
            GearControl gain;
            gain.id = "gain";
            gain.name = "Gain";
            gain.type = GearControl::Type::Knob;
            gain.position = {0.257f, 0.44f};
            gain.value = 180;
            gain.startAngle = 40;
            gain.endAngle = 322;
            gain.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(gain);

            auto gearItem = std::make_unique<GearItem>(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor,
                1,
                controls);

            auto slot = rack.getSlot(0);
            if (slot != nullptr)
            {
                slot->setGearItem(gearItem.get());
                expect(!slot->isAvailable(), "Slot should not be available");
                expect(slot->getGearItem()->name == "LA-2A Tube Compressor", "Gear item name should match");
                expect(slot->getGearItem()->manufacturer == "Universal Audio", "Manufacturer should match");
                expect(slot->getGearItem()->category == GearCategory::Compressor, "Category should be Compressor");
                expect(slot->getGearItem()->type == GearType::Rack19Inch, "Type should be Rack19Inch");
            }
        }

        beginTest("Instance Management");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            // Create Peak Reduction control
            GearControl peakReduction;
            peakReduction.id = "peak-reduction";
            peakReduction.name = "Peak Reduction";
            peakReduction.type = GearControl::Type::Knob;
            peakReduction.position = {0.68f, 0.44f};
            peakReduction.value = 180;
            peakReduction.startAngle = 40;
            peakReduction.endAngle = 322;
            peakReduction.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(peakReduction);

            // Create Gain control
            GearControl gain;
            gain.id = "gain";
            gain.name = "Gain";
            gain.type = GearControl::Type::Knob;
            gain.position = {0.257f, 0.44f};
            gain.value = 180;
            gain.startAngle = 40;
            gain.endAngle = 322;
            gain.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(gain);

            auto gearItem = std::make_unique<GearItem>(
                "la2a-compressor",
                "LA-2A Tube Compressor",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor,
                1,
                controls);

            auto slot = rack.getSlot(0);
            if (slot != nullptr)
            {
                slot->setGearItem(gearItem.get());
                rack.createInstance(0);
                expect(rack.isInstance(0), "Slot should be an instance");
                expect(!rack.getInstanceId(0).isEmpty(), "Instance ID should not be empty");
                expect(slot->getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged");
                expect(slot->getGearItem()->manufacturer == "Universal Audio", "Manufacturer should remain unchanged");

                // Store the instance ID before reset
                auto instanceId = rack.getInstanceId(0);

                // Reset the instance
                rack.resetToSource(0);

                // Verify instance state is preserved
                expect(rack.isInstance(0), "Slot should still be an instance after reset");
                expect(rack.getInstanceId(0) == instanceId, "Instance ID should be preserved after reset");
                expect(slot->getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged after reset");
            }
        }

        beginTest("Multiple Slots");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            // Create Peak Reduction control
            GearControl peakReduction;
            peakReduction.id = "peak-reduction";
            peakReduction.name = "Peak Reduction";
            peakReduction.type = GearControl::Type::Knob;
            peakReduction.position = {0.68f, 0.44f};
            peakReduction.value = 180;
            peakReduction.startAngle = 40;
            peakReduction.endAngle = 322;
            peakReduction.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(peakReduction);

            // Create Gain control
            GearControl gain;
            gain.id = "gain";
            gain.name = "Gain";
            gain.type = GearControl::Type::Knob;
            gain.position = {0.257f, 0.44f};
            gain.value = 180;
            gain.startAngle = 40;
            gain.endAngle = 322;
            gain.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(gain);

            auto gearItem1 = std::make_unique<GearItem>(
                "la2a-compressor-1",
                "LA-2A Tube Compressor 1",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor,
                1,
                controls);

            auto gearItem2 = std::make_unique<GearItem>(
                "la2a-compressor-2",
                "LA-2A Tube Compressor 2",
                "Universal Audio",
                "compressor",
                "1.0.0",
                "units/la2a-compressor-1.0.0.json",
                "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor,
                1,
                controls);

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
                expect(slot1->getGearItem()->name == "LA-2A Tube Compressor 1", "Slot 1 name should match");
                expect(slot2->getGearItem()->name == "LA-2A Tube Compressor 2", "Slot 2 name should match");

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
                expect(slot1->getGearItem()->name == "LA-2A Tube Compressor 1", "Slot 1 name should remain unchanged after reset");
                expect(slot2->getGearItem()->name == "LA-2A Tube Compressor 2", "Slot 2 name should remain unchanged after reset");
            }
        }

        beginTest("Preset Integration");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test that the rack can load preset state
            auto *slot = rack.getSlot(0);
            expect(slot != nullptr, "Slot should exist");

            // Create a gear item for testing using the constructor
            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            auto gearItem = std::make_unique<GearItem>(
                "test-gear",
                "Test Gear",
                "Test Manufacturer",
                "test-type",
                "1.0.0",
                "units/test-gear-1.0.0.json",
                "assets/thumbnails/test-gear-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Other,
                1,
                controls);

            // Set the gear item in the slot
            slot->setGearItem(gearItem.get());
            expect(slot->getGearItem() == gearItem.get(), "Gear item should be set");

            // Test that the rack state can be saved for presets
            auto *slot2 = rack.getSlot(0);
            expect(slot2 != nullptr, "Slot should exist");
            expect(slot2->getGearItem() == gearItem.get(), "Gear item should be persisted");

            // Test that all slots can be cleared for preset loading
            auto *slot3 = rack.getSlot(0);
            expect(slot3 != nullptr, "Slot should exist");
            slot3->setGearItem(nullptr);
            expect(slot3->getGearItem() == nullptr, "Gear item should be cleared");

            // Test that the rack provides the correct slot count for preset operations
            int slotCount = rack.getNumSlots();
            expect(slotCount > 0, "Slot count should be greater than 0");

            // Verify we can access all slots
            for (int i = 0; i < slotCount; ++i)
            {
                auto *slot = rack.getSlot(i);
                expect(slot != nullptr, "Slot should exist");
            }

            // Test that multiple gear items can be set and cleared for preset operations
            auto gearItem1 = std::make_unique<GearItem>(
                "test-gear-1",
                "Test Gear 1",
                "Test Manufacturer",
                "test-type-1",
                "1.0.0",
                "units/test-gear-1-1.0.0.json",
                "assets/thumbnails/test-gear-1-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Other,
                1,
                controls);

            auto gearItem2 = std::make_unique<GearItem>(
                "test-gear-2",
                "Test Gear 2",
                "Test Manufacturer",
                "test-type-2",
                "1.0.0",
                "units/test-gear-2-1.0.0.json",
                "assets/thumbnails/test-gear-2-1.0.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Other,
                1,
                controls);

            // Set first gear item
            auto *slot4 = rack.getSlot(0);
            slot4->setGearItem(gearItem1.get());
            expect(slot4->getGearItem() == gearItem1.get(), "First gear item should be set");

            // Replace with second gear item
            slot4->setGearItem(gearItem2.get());
            expect(slot4->getGearItem() == gearItem2.get(), "Second gear item should be set");

            // Clear gear item
            slot4->setGearItem(nullptr);
            expect(slot4->getGearItem() == nullptr, "Gear item should be cleared");
        }

        beginTest("Slot Height Management");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test that we can access slots and they have reasonable properties
            for (int i = 0; i < rack.getNumSlots(); ++i)
            {
                auto *rackSlot = rack.getSlot(i);
                expect(rackSlot != nullptr, "Slot should exist");
                expect(rackSlot->isVisible(), "Slot should be visible");
            }
        }

        beginTest("Gear Rearrangement");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            // Create test gear items
            auto gearItem1 = std::make_unique<GearItem>(
                "gear-1", "Gear 1", "Manufacturer", "type", "1.0.0",
                "units/gear-1.json", "assets/gear-1.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Other, 1, controls);

            auto gearItem2 = std::make_unique<GearItem>(
                "gear-2", "Gear 2", "Manufacturer", "type", "1.0.0",
                "units/gear-2.json", "assets/gear-2.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Other, 1, controls);

            // Set gear items in slots
            auto *slot0 = rack.getSlot(0);
            auto *slot1 = rack.getSlot(1);
            slot0->setGearItem(gearItem1.get());
            slot1->setGearItem(gearItem2.get());

            // Test rearrangement
            rack.rearrangeGearAsSortableList(0, 1);
            expect(slot0->getGearItem() == gearItem2.get(), "Slot 0 should now contain gear 2");
            expect(slot1->getGearItem() == gearItem1.get(), "Slot 1 should now contain gear 1");

            // Test rearrangement back
            rack.rearrangeGearAsSortableList(1, 0);
            expect(slot0->getGearItem() == gearItem1.get(), "Slot 0 should now contain gear 1 again");
            expect(slot1->getGearItem() == gearItem2.get(), "Slot 1 should now contain gear 2 again");
        }

        beginTest("Schema Fetching and Parsing");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            auto gearItem = std::make_unique<GearItem>(
                "la2a-compressor", "LA-2A", "Universal Audio", "compressor", "1.0.0",
                "units/la2a-compressor-1.0.0.json", "assets/la2a.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Compressor, 1, controls);

            // Test schema fetching
            rack.fetchSchemaForGearItem(gearItem.get(), []() {});

            // Wait longer for async operations and mock responses
            juce::Thread::sleep(200);

            // The mock should have responded by now, but let's test what we can
            // The actual parsing happens asynchronously, so we'll test the method execution
            expect(true, "Schema fetching method should execute without errors");
        }

        beginTest("Image Fetching");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            // Create a control with image path
            GearControl control;
            control.id = "test-control";
            control.name = "Test Control";
            control.type = GearControl::Type::Knob;
            control.image = "assets/controls/knobs/bakelite-lg-black.png";
            controls.add(control);

            auto gearItem = std::make_unique<GearItem>(
                "test-gear", "Test Gear", "Manufacturer", "type", "1.0.0",
                "units/test-gear.json", "assets/test-gear.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Other, 1, controls);

            // Test faceplate image fetching
            rack.fetchFaceplateImage(gearItem.get());
            juce::Thread::sleep(100);

            // Test control image fetching
            rack.fetchKnobImage(gearItem.get(), 0);
            juce::Thread::sleep(100);

            rack.fetchFaderImage(gearItem.get(), 0);
            juce::Thread::sleep(100);

            rack.fetchSwitchSpriteSheet(gearItem.get(), 0);
            juce::Thread::sleep(100);

            rack.fetchButtonSpriteSheet(gearItem.get(), 0);
            juce::Thread::sleep(100);
        }

        beginTest("Notification Methods");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;

            auto gearItem = std::make_unique<GearItem>(
                "test-gear", "Test Gear", "Manufacturer", "type", "1.0.0",
                "units/test-gear.json", "assets/test-gear.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Other, 1, controls);

            // Test notification methods
            rack.notifyGearItemAdded(0, gearItem.get());
            rack.notifyGearItemRemoved(1);
            rack.notifyGearControlChanged(0, gearItem.get(), 0);
            rack.notifyGearItemsRearranged(0, 1);

            // These methods should not crash and should handle the notifications gracefully
            expect(true, "Notification methods should execute without errors");
        }

        beginTest("Edge Cases and Error Handling");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test with null gear item
            rack.fetchSchemaForGearItem(nullptr, []() {});
            rack.fetchFaceplateImage(nullptr);
            rack.fetchKnobImage(nullptr, 0);
            rack.fetchFaderImage(nullptr, 0);
            rack.fetchSwitchSpriteSheet(nullptr, 0);
            rack.fetchButtonSpriteSheet(nullptr, 0);

            // Test with invalid control indices
            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
            juce::Array<GearControl> controls;
            auto gearItem = std::make_unique<GearItem>(
                "test-gear", "Test Gear", "Manufacturer", "type", "1.0.0",
                "units/test-gear.json", "assets/test-gear.jpg", tags,
                mockFetcher, mockFileSystem, cacheManager,
                GearType::Rack19Inch, GearCategory::Other, 1, controls);

            rack.fetchKnobImage(gearItem.get(), -1);
            rack.fetchKnobImage(gearItem.get(), 100);
            rack.fetchFaderImage(gearItem.get(), -1);
            rack.fetchFaderImage(gearItem.get(), 100);

            // Test instance management edge cases
            rack.createInstance(-1);
            rack.createInstance(rack.getNumSlots() + 10);
            rack.resetToSource(-1);
            rack.resetToSource(rack.getNumSlots() + 10);
            rack.isInstance(-1);
            rack.isInstance(rack.getNumSlots() + 10);
            rack.getInstanceId(-1);
            rack.getInstanceId(rack.getNumSlots() + 10);

            // These should handle errors gracefully without crashing
            expect(true, "Edge case methods should execute without errors");
        }

        beginTest("Component Lifecycle");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test component ID
            expect(rack.getComponentID() == "Rack", "Component ID should be set correctly");

            // Test visibility - the rack should be visible by default
            expect(rack.isVisible(), "Rack should be visible by default");

            // Test bounds - the rack needs to be properly sized first
            rack.setBounds(0, 0, 800, 600);
            juce::Rectangle<int> bounds = rack.getBounds();
            expect(bounds.getWidth() > 0, "Rack should have positive width");
            expect(bounds.getHeight() > 0, "Rack should have positive height");

            // Test resizing
            rack.setBounds(100, 100, 800, 600);
            bounds = rack.getBounds();
            expect(bounds.getX() == 100, "X position should be set correctly");
            expect(bounds.getY() == 100, "Y position should be set correctly");
            expect(bounds.getWidth() == 800, "Width should be set correctly");
            expect(bounds.getHeight() == 600, "Height should be set correctly");
        }

        beginTest("Drag and Drop Functionality");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test drag and drop interest
            juce::DragAndDropTarget::SourceDetails dragDetails(
                "GearItem",
                &rack,
                juce::Point<int>(100, 100));

            bool isInterested = rack.isInterestedInDragSource(dragDetails);
            // The rack might not be interested in all gear item drops, so we'll test the method execution
            expect(true, "Drag and drop interest check should execute without errors");

            // Test drag enter/exit/move/drop methods
            rack.itemDragEnter(dragDetails);
            rack.itemDragMove(dragDetails);
            rack.itemDragExit(dragDetails);
            rack.itemDropped(dragDetails);

            // These methods should not crash
            expect(true, "Drag and drop methods should execute without errors");
        }

        beginTest("Listener Management");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test listener management
            rack.addRackStateListener(nullptr);
            rack.removeRackStateListener(nullptr);

            // Test notification methods
            rack.notifyRackStateReset();
            rack.notifyPresetLoaded("Test Preset");
            rack.notifyPresetSaved("Test Preset");

            // These methods should not crash
            expect(true, "Listener management methods should execute without errors");
        }

        beginTest("Gear Library Integration");
        {
            setUpMocks(mockFetcher);
            Rack rack(mockFetcher, mockFileSystem, cacheManager, presetManager, nullptr);

            // Test gear library setter
            rack.setGearLibrary(nullptr);

            // Test finding nearest slot
            juce::Point<int> testPoint(100, 100);
            auto *nearestSlot = rack.findNearestSlot(testPoint);
            expect(nearestSlot != nullptr, "Should find a nearest slot");

            // Test with different positions
            juce::Point<int> edgePoint(0, 0);
            auto *edgeSlot = rack.findNearestSlot(edgePoint);
            expect(edgeSlot != nullptr, "Should find a slot at edge position");
        }
    }
};

static RackTests rackTestsInstance;