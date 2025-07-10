/**
 * @file PresetIntegrationTests.cpp
 * @brief Integration tests for the preset system.
 *
 * This file contains integration tests that verify the complete preset system
 * workflow, including UI interactions, preset management, and error handling.
 */

#include <JuceHeader.h>
#include "TestFixture.h"
#include "AnalogIQProcessor.h"
#include "AnalogIQEditor.h"
#include "PresetManager.h"
#include "Rack.h"
#include "GearLibrary.h"
#include "GearItem.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"

using namespace juce;

/**
 * @brief Integration tests for the preset system.
 */
class PresetIntegrationTests : public juce::UnitTest
{
public:
    PresetIntegrationTests() : UnitTest("PresetIntegrationTests") {}

    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Set up mock response for the units index
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json",
            R"({
                "units": [
                    {
                        "unitId": "test-eq",
                        "name": "Test EQ",
                        "manufacturer": "Test Manufacturer",
                        "category": "equalizer",
                        "version": "1.0.0",
                        "schemaPath": "units/test-eq-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/test-eq-1.0.0.jpg",
                        "tags": ["test"]
                    },
                    {
                        "unitId": "test-compressor",
                        "name": "Test Compressor",
                        "manufacturer": "Test Manufacturer",
                        "category": "compressor",
                        "version": "1.0.0",
                        "schemaPath": "units/test-compressor-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/test-compressor-1.0.0.jpg",
                        "tags": ["test"]
                    },
                    {
                        "unitId": "test-gear",
                        "name": "Test Gear",
                        "manufacturer": "Test Manufacturer",
                        "category": "misc",
                        "version": "1.0.0",
                        "schemaPath": "units/test-gear-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/test-gear-1.0.0.jpg",
                        "tags": ["test"]
                    }
                ]
            })");

        // Create a simple test image
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

        // Set up mock responses for images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/test-eq-1.0.0.jpg",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/test-compressor-1.0.0.jpg",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/test-gear-1.0.0.jpg",
            imageData);

        // Set up mock responses for schemas
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-eq-1.0.0.json",
            R"({
                "unitId": "test-eq",
                "name": "Test EQ",
                "manufacturer": "Test Manufacturer",
                "tags": ["test"],
                "version": "1.0.0",
                "category": "equalizer",
                "formFactor": "19-inch-rack",
                "faceplateImage": "assets/faceplates/test-eq-1.0.0.jpg",
                "thumbnailImage": "assets/thumbnails/test-eq-1.0.0.jpg",
                "width": 1900,
                "height": 525,
                "controls": []
            })");

        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-compressor-1.0.0.json",
            R"({
                "unitId": "test-compressor",
                "name": "Test Compressor",
                "manufacturer": "Test Manufacturer",
                "tags": ["test"],
                "version": "1.0.0",
                "category": "compressor",
                "formFactor": "19-inch-rack",
                "faceplateImage": "assets/faceplates/test-compressor-1.0.0.jpg",
                "thumbnailImage": "assets/thumbnails/test-compressor-1.0.0.jpg",
                "width": 1900,
                "height": 525,
                "controls": []
            })");

        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-gear-1.0.0.json",
            R"({
                "unitId": "test-gear",
                "name": "Test Gear",
                "manufacturer": "Test Manufacturer",
                "tags": ["test"],
                "version": "1.0.0",
                "category": "misc",
                "formFactor": "19-inch-rack",
                "faceplateImage": "assets/faceplates/test-gear-1.0.0.jpg",
                "thumbnailImage": "assets/thumbnails/test-gear-1.0.0.jpg",
                "width": 1900,
                "height": 525,
                "controls": []
            })");
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Reset singletons to use mock file system
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Preset Save Workflow");
        {
            setUpMocks(mockFetcher);

            // Create processor and editor in a scope to ensure proper cleanup
            {
                auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher, mockFileSystem);
                PresetManager presetManager(mockFileSystem, cacheManager);
                auto editor = std::make_unique<AnalogIQEditor>(*processor, cacheManager, presetManager, true);

                auto &presetManagerRef = editor->getPresetManager();
                auto *rack = editor->getRack();
                auto *gearLibrary = editor->getGearLibrary();

                expect(rack != nullptr, "Rack should be accessible");
                expect(gearLibrary != nullptr, "Gear library should be accessible");

                // Create test gear items using the library
                juce::StringArray tags = {"test"};
                juce::Array<GearControl> controls;

                auto testEQ = std::make_unique<GearItem>(
                    "test-eq",
                    "Test EQ",
                    "Test Manufacturer",
                    "equalizer",
                    "1.0.0",
                    "units/test-eq-1.0.0.json",
                    "assets/thumbnails/test-eq-1.0.0.jpg",
                    tags,
                    mockFetcher,
                    mockFileSystem,
                    cacheManager,
                    GearType::Rack19Inch,
                    GearCategory::EQ,
                    1,
                    controls);

                auto testCompressor = std::make_unique<GearItem>(
                    "test-compressor",
                    "Test Compressor",
                    "Test Manufacturer",
                    "compressor",
                    "1.0.0",
                    "units/test-compressor-1.0.0.json",
                    "assets/thumbnails/test-compressor-1.0.0.jpg",
                    tags,
                    mockFetcher,
                    mockFileSystem,
                    cacheManager,
                    GearType::Rack19Inch,
                    GearCategory::Compressor,
                    1,
                    controls);

                // Add gear items to rack
                if (auto *slot0 = rack->getSlot(0))
                {
                    slot0->setGearItem(testEQ.get());
                    rack->createInstance(0);
                }

                if (auto *slot1 = rack->getSlot(1))
                {
                    slot1->setGearItem(testCompressor.get());
                    rack->createInstance(1);
                }

                // Test saving preset
                expect(presetManagerRef.savePreset("Integration Test Preset", rack),
                       "Should save preset successfully");

                // Verify preset was saved
                expect(presetManagerRef.isPresetValid("Integration Test Preset"),
                       "Saved preset should be valid");
            } // Editor and processor go out of scope here
        }

        // Clean up mock responses before next test
        mockFetcher.reset();

        beginTest("Preset Load Workflow");
        {
            setUpMocks(mockFetcher);

            // Create processor and editor for loading in a scope to ensure proper cleanup
            {
                auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher, mockFileSystem);
                PresetManager presetManager(mockFileSystem, cacheManager);
                auto editor = std::make_unique<AnalogIQEditor>(*processor, cacheManager, presetManager, true);
                auto *rack = editor->getRack();
                auto *gearLibrary = editor->getGearLibrary();

                auto &presetManagerRef = editor->getPresetManager();

                // Manually add gear items to the library so they can be found during loading
                gearLibrary->addItem("Test EQ", "equalizer", "Test Equalizer", "Test Manufacturer");
                gearLibrary->addItem("Test Compressor", "compressor", "Test Compressor", "Test Manufacturer");

                // Test loading preset
                expect(presetManagerRef.loadPreset("Integration Test Preset", rack, gearLibrary),
                       "Should load preset successfully");

                // Verify loaded preset
                if (auto *slot0 = rack->getSlot(0))
                {
                    expect(slot0->getGearItem() != nullptr, "Slot 0 should have a gear item after loading");
                    if (auto *item = slot0->getGearItem())
                    {
                        expect(item->name == "Test EQ", "Loaded item should have correct name");
                        expect(item->isInstance, "Loaded item should be an instance");
                    }
                }

                if (auto *slot1 = rack->getSlot(1))
                {
                    expect(slot1->getGearItem() != nullptr, "Slot 1 should have a gear item after loading");
                    if (auto *item = slot1->getGearItem())
                    {
                        expect(item->name == "Test Compressor", "Loaded item should have correct name");
                        expect(item->isInstance, "Loaded item should be an instance");
                    }
                }

                // Test deleting preset
                expect(presetManagerRef.deletePreset("Integration Test Preset"),
                       "Should delete preset successfully");
                expect(!presetManagerRef.isPresetValid("Integration Test Preset"),
                       "Preset should not exist after deletion");
            } // Editor and processor go out of scope here
        }

        // Clean up mock responses before next test
        mockFetcher.reset();

        beginTest("Preset UI Integration");
        {
            setUpMocks(mockFetcher);

            // Test that the editor can be created and resized without errors
            {
                auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher, mockFileSystem);
                PresetManager presetManager(mockFileSystem, cacheManager);
                auto editor = std::make_unique<AnalogIQEditor>(*processor, cacheManager, presetManager, true);

                // Test editor initialization
                expect(editor != nullptr, "Editor should be created successfully");

                // Test editor resizing (menu positioning)
                editor->setSize(800, 600);
                editor->resized();
                expect(true, "Editor should resize without errors");

                // Test that preset manager is accessible through editor
                auto &editorPresetManager = editor->getPresetManager();
                expect(&editorPresetManager != nullptr, "Preset manager should be accessible through editor");

                // Test that rack and gear library are accessible through editor
                auto *rack = editor->getRack();
                auto *gearLibrary = editor->getGearLibrary();
                expect(rack != nullptr, "Rack should be accessible through editor");
                expect(gearLibrary != nullptr, "Gear library should be accessible through editor");
            } // Editor and processor go out of scope here
        }

        // Clean up mock responses before next test
        mockFetcher.reset();

        beginTest("Preset Error Handling Integration");
        {
            setUpMocks(mockFetcher);

            {
                auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher, mockFileSystem);
                PresetManager presetManager(mockFileSystem, cacheManager);
                auto editor = std::make_unique<AnalogIQEditor>(*processor, cacheManager, presetManager, true);

                auto &presetManagerRef = editor->getPresetManager();
                auto *rack = editor->getRack();
                auto *gearLibrary = editor->getGearLibrary();

                // Test saving with invalid name
                expect(!presetManagerRef.savePreset("", rack),
                       "Saving with empty name should fail");
                expect(!presetManagerRef.getLastErrorMessage().isEmpty(),
                       "Error message should be set after failed save");

                // Test loading non-existent preset
                expect(!presetManagerRef.loadPreset("NonExistentPreset", rack, gearLibrary),
                       "Loading non-existent preset should fail");
                expect(!presetManagerRef.getLastErrorMessage().isEmpty(),
                       "Error message should be set after failed load");

                // Test deleting non-existent preset
                expect(!presetManagerRef.deletePreset("NonExistentPreset"),
                       "Deleting non-existent preset should fail");
                expect(!presetManagerRef.getLastErrorMessage().isEmpty(),
                       "Error message should be set after failed delete");
            } // Editor and processor go out of scope here
        }

        // Clean up mock responses before next test
        mockFetcher.reset();

        beginTest("Preset State Management Integration");
        {
            setUpMocks(mockFetcher);

            {
                auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher, mockFileSystem);
                PresetManager presetManager(mockFileSystem, cacheManager);
                auto editor = std::make_unique<AnalogIQEditor>(*processor, cacheManager, presetManager, true);

                auto &presetManagerRef = editor->getPresetManager();
                auto *rack = editor->getRack();
                auto *gearLibrary = editor->getGearLibrary();

                // Test that rack starts empty
                bool hasGearItems = false;
                for (int i = 0; i < rack->getNumSlots(); ++i)
                {
                    if (auto *slot = rack->getSlot(i))
                    {
                        if (slot->getGearItem() != nullptr)
                        {
                            hasGearItems = true;
                            break;
                        }
                    }
                }
                expect(!hasGearItems, "Rack should start empty");

                // Create and save a preset
                juce::StringArray tags = {"test"};
                juce::Array<GearControl> controls;

                auto testGear = std::make_unique<GearItem>(
                    "test-gear",
                    "Test Gear",
                    "Test Manufacturer",
                    "misc",
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

                if (auto *slot = rack->getSlot(0))
                {
                    slot->setGearItem(testGear.get());
                    rack->createInstance(0);
                }

                expect(presetManagerRef.savePreset("State Test Preset", rack),
                       "Should save preset successfully");

                // Clear the rack
                for (int i = 0; i < rack->getNumSlots(); ++i)
                {
                    if (auto *slot = rack->getSlot(i))
                    {
                        slot->setGearItem(nullptr);
                    }
                }

                // Verify rack is empty
                hasGearItems = false;
                for (int i = 0; i < rack->getNumSlots(); ++i)
                {
                    if (auto *slot = rack->getSlot(i))
                    {
                        if (slot->getGearItem() != nullptr)
                        {
                            hasGearItems = true;
                            break;
                        }
                    }
                }
                expect(!hasGearItems, "Rack should be empty after clearing");

                // Manually add gear items to the library so they can be found during loading
                gearLibrary->addItem("Test Gear", "misc", "Test Gear", "Test Manufacturer");

                // Load the preset
                expect(presetManagerRef.loadPreset("State Test Preset", rack, gearLibrary),
                       "Should load preset successfully");

                // Verify gear item was restored
                if (auto *slot = rack->getSlot(0))
                {
                    expect(slot->getGearItem() != nullptr, "Gear item should be restored after loading preset");
                }

                // Clean up
                presetManagerRef.deletePreset("State Test Preset");
            } // Editor and processor go out of scope here
        }

        // Final cleanup
        mockFetcher.reset();
    }
};

static PresetIntegrationTests presetIntegrationTests;