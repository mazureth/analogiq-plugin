/**
 * @file PresetIntegrationTests.cpp
 * @brief Integration tests for the preset system.
 *
 * This file contains integration tests that verify the complete preset system
 * workflow, including UI interactions, preset management, and error handling.
 */

#include <JuceHeader.h>
#include "TestFixture.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetManager.h"
#include "Rack.h"
#include "GearLibrary.h"
#include "GearItem.h"
#include "MockNetworkFetcher.h"

using namespace juce;

/**
 * @brief Integration tests for the preset system.
 */
class PresetIntegrationTests : public juce::UnitTest
{
public:
    PresetIntegrationTests() : UnitTest("PresetIntegrationTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        mockFetcher.reset();

        beginTest("Complete Preset Workflow");
        {
            // Create processor and editor
            auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher);
            auto editor = std::make_unique<AnalogIQEditor>(*processor);

            auto &presetManager = PresetManager::getInstance();
            auto *rack = editor->getRack();
            auto *gearLibrary = editor->getGearLibrary();

            expect(rack != nullptr, "Rack should be accessible");
            expect(gearLibrary != nullptr, "Gear library should be accessible");

            // Create test gear items
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
            expect(presetManager.savePreset("Integration Test Preset", rack),
                   "Should save preset successfully");

            // Verify preset was saved
            expect(presetManager.isPresetValid("Integration Test Preset"),
                   "Saved preset should be valid");

            // Test loading preset into a new rack
            auto newProcessor = std::make_unique<AnalogIQProcessor>(mockFetcher);
            auto newEditor = std::make_unique<AnalogIQEditor>(*newProcessor);
            auto *newRack = newEditor->getRack();
            auto *newGearLibrary = newEditor->getGearLibrary();

            expect(presetManager.loadPreset("Integration Test Preset", newRack, newGearLibrary),
                   "Should load preset successfully");

            // Verify loaded preset
            if (auto *slot0 = newRack->getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have a gear item after loading");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->name == "Test EQ", "Loaded item should have correct name");
                    expect(item->isInstance, "Loaded item should be an instance");
                }
            }

            if (auto *slot1 = newRack->getSlot(1))
            {
                expect(slot1->getGearItem() != nullptr, "Slot 1 should have a gear item after loading");
                if (auto *item = slot1->getGearItem())
                {
                    expect(item->name == "Test Compressor", "Loaded item should have correct name");
                    expect(item->isInstance, "Loaded item should be an instance");
                }
            }

            // Test deleting preset
            expect(presetManager.deletePreset("Integration Test Preset"),
                   "Should delete preset successfully");
            expect(!presetManager.isPresetValid("Integration Test Preset"),
                   "Preset should not exist after deletion");
        }

        beginTest("Preset UI Integration");
        {
            // Test that the editor can be created and resized without errors
            auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher);
            auto editor = std::make_unique<AnalogIQEditor>(*processor);

            // Test editor initialization
            expect(editor != nullptr, "Editor should be created successfully");

            // Test editor resizing (menu positioning)
            editor->setSize(800, 600);
            editor->resized();
            expect(true, "Editor should resize without errors");

            // Test that preset manager is accessible through editor
            auto &presetManager = editor->getPresetManager();
            expect(&presetManager != nullptr, "Preset manager should be accessible through editor");

            // Test that rack and gear library are accessible through editor
            auto *rack = editor->getRack();
            auto *gearLibrary = editor->getGearLibrary();
            expect(rack != nullptr, "Rack should be accessible through editor");
            expect(gearLibrary != nullptr, "Gear library should be accessible through editor");
        }

        beginTest("Preset Error Handling Integration");
        {
            auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher);
            auto editor = std::make_unique<AnalogIQEditor>(*processor);

            auto &presetManager = PresetManager::getInstance();
            auto *rack = editor->getRack();
            auto *gearLibrary = editor->getGearLibrary();

            // Test saving with invalid name
            expect(!presetManager.savePreset("", rack),
                   "Saving with empty name should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(),
                   "Error message should be set after failed save");

            // Test loading non-existent preset
            expect(!presetManager.loadPreset("NonExistentPreset", rack, gearLibrary),
                   "Loading non-existent preset should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(),
                   "Error message should be set after failed load");

            // Test deleting non-existent preset
            expect(!presetManager.deletePreset("NonExistentPreset"),
                   "Deleting non-existent preset should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(),
                   "Error message should be set after failed delete");
        }

        beginTest("Preset State Management Integration");
        {
            auto processor = std::make_unique<AnalogIQProcessor>(mockFetcher);
            auto editor = std::make_unique<AnalogIQEditor>(*processor);

            auto &presetManager = PresetManager::getInstance();
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
                "test",
                "1.0.0",
                "units/test-gear-1.0.0.json",
                "assets/thumbnails/test-gear-1.0.0.jpg",
                tags,
                mockFetcher,
                GearType::Rack19Inch,
                GearCategory::Other,
                1,
                controls);

            if (auto *slot = rack->getSlot(0))
            {
                slot->setGearItem(testGear.get());
                rack->createInstance(0);
            }

            expect(presetManager.savePreset("State Test Preset", rack),
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

            // Load the preset
            expect(presetManager.loadPreset("State Test Preset", rack, gearLibrary),
                   "Should load preset successfully");

            // Verify gear item was restored
            if (auto *slot = rack->getSlot(0))
            {
                expect(slot->getGearItem() != nullptr, "Gear item should be restored after loading preset");
            }

            // Clean up
            presetManager.deletePreset("State Test Preset");
        }
    }
};