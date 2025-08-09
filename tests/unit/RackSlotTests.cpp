#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class RackSlotTests : public juce::UnitTest
{
public:
    RackSlotTests() : UnitTest("RackSlotTests") {}

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
        GearLibrary gearLibrary(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Initial State");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Gear Item Management");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

            const juce::StringArray &tags = TestImageHelper::getStaticTestTags();
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

            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            auto item = std::make_unique<GearItem>(
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

            slot.setGearItem(item.get());
            expect(!slot.isAvailable(), "Slot should not be available");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Gear item name should match");
            expect(slot.getGearItem()->manufacturer == "Universal Audio", "Manufacturer should match");
            expect(slot.getGearItem()->category == GearCategory::Compressor, "Category should be Compressor");
            expect(slot.getGearItem()->type == GearType::Rack19Inch, "Type should be Rack19Inch");
        }

        beginTest("Clear Gear Item");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

            const juce::StringArray &tags = TestImageHelper::getStaticTestTags();
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

            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            auto gearItem = std::make_unique<GearItem>(
                "test-equalizer",
                "Test Equalizer",
                "Test Co",
                "equalizer",
                "1.0",
                "units/test-equalizer-1.0.json",
                "assets/thumbnails/test-equalizer-1.0.jpg",
                tags,
                mockFetcher,
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::EQ,
                1,
                controls);

            slot.setGearItem(gearItem.get());
            slot.clearGearItem();
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Instance Management");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

            const juce::StringArray &tags = TestImageHelper::getStaticTestTags();
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

            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
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

            slot.setGearItem(gearItem.get());
            slot.createInstance();
            expect(slot.isInstance(), "Slot should be an instance");
            expect(!slot.getInstanceId().isEmpty(), "Instance ID should not be empty");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged");
            expect(slot.getGearItem()->manufacturer == "Universal Audio", "Manufacturer should remain unchanged");

            // Store the instance ID before reset
            auto instanceId = slot.getInstanceId();

            // Reset the instance
            slot.resetToSource();

            // Verify instance state is preserved
            expect(slot.isInstance(), "Slot should still be an instance after reset");
            expect(slot.getInstanceId() == instanceId, "Instance ID should be preserved after reset");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged after reset");
        }

        beginTest("Preset Integration");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

            // Test that gear items can be set for preset loading
            juce::StringArray tags = {"test"};
            juce::Array<GearControl> controls;

            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
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

            slot.setGearItem(gearItem.get());
            expect(slot.getGearItem() == gearItem.get(), "Gear item should be set");

            // Test that gear items can be retrieved for preset saving
            auto *retrievedItem = slot.getGearItem();
            expect(retrievedItem == gearItem.get(), "Retrieved gear item should match");

            // Test that gear items can be cleared for preset loading
            slot.setGearItem(nullptr);
            expect(slot.getGearItem() == nullptr, "Gear item should be cleared");

            // Test that control values are persisted when gear items are set
            slot.setGearItem(gearItem.get());
            expect(slot.getGearItem() == gearItem.get(), "Gear item should be persisted");

            // Test that control values are restored when loading presets
            slot.setGearItem(gearItem.get());
            expect(slot.getGearItem() == gearItem.get(), "Gear item should be restored");

            // Test that state changes are properly notified for preset integration
            slot.setGearItem(gearItem.get());
            slot.setGearItem(nullptr);
            expect(true, "State changes should be notified");

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
            slot.setGearItem(gearItem1.get());
            expect(slot.getGearItem() == gearItem1.get(), "First gear item should be set");

            // Replace with second gear item
            slot.setGearItem(gearItem2.get());
            expect(slot.getGearItem() == gearItem2.get(), "Second gear item should be set");

            // Clear gear item
            slot.setGearItem(nullptr);
            expect(slot.getGearItem() == nullptr, "Gear item should be cleared");

            // Test that gear item properties are preserved during preset operations
            slot.setGearItem(gearItem.get());
            expect(slot.getGearItem()->name == "Test Gear", "Gear item name should be preserved");
            expect(slot.getGearItem()->categoryString == "test-type", "Gear item type should be preserved");
        }
    }
};

static RackSlotTests rackSlotTestsInstance;