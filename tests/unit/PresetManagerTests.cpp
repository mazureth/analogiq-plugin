/**
 * @file PresetManagerTests.cpp
 * @brief Unit tests for the PresetManager class.
 *
 * This file contains unit tests for the PresetManager class, testing
 * preset save, load, delete, and utility operations.
 */

#include <JuceHeader.h>
#include "TestFixture.h"
#include "PresetManager.h"
#include "Rack.h"
#include "GearLibrary.h"
#include "GearItem.h"
#include "MockNetworkFetcher.h"

using namespace juce;

/**
 * @brief Unit tests for the PresetManager class.
 */
class PresetManagerTests : public juce::UnitTest
{
public:
    PresetManagerTests() : UnitTest("PresetManagerTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        mockFetcher.reset();

        beginTest("Singleton Pattern");
        {
            auto &instance1 = PresetManager::getInstance();
            auto &instance2 = PresetManager::getInstance();
            expect(&instance1 == &instance2, "Singleton instances should be the same");
        }

        beginTest("Directory Management");
        {
            auto &presetManager = PresetManager::getInstance();

            // Test presets directory path
            auto presetsDir = presetManager.getPresetsDirectory();
            expect(presetsDir.exists() || presetsDir.createDirectory(), "Should be able to create presets directory");

            // Test initialization
            expect(presetManager.initializePresetsDirectory(), "Should initialize presets directory");
        }

        beginTest("Preset Save and Load");
        {
            auto &presetManager = PresetManager::getInstance();

            // Create a test rack with gear items
            Rack rack(mockFetcher);
            GearLibrary gearLibrary(mockFetcher, false);

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

            // Add gear items to rack slots
            if (auto *slot0 = rack.getSlot(0))
            {
                slot0->setGearItem(testEQ.get());
                rack.createInstance(0);
            }

            if (auto *slot1 = rack.getSlot(1))
            {
                slot1->setGearItem(testCompressor.get());
                rack.createInstance(1);
            }

            // Test saving preset
            expect(presetManager.savePreset("Test Preset", &rack), "Should save preset successfully");

            // Test that preset exists
            expect(presetManager.isPresetValid("Test Preset"), "Saved preset should be valid");

            // Test loading preset into a new rack
            Rack newRack(mockFetcher);
            expect(presetManager.loadPreset("Test Preset", &newRack, &gearLibrary), "Should load preset successfully");

            // Verify loaded preset
            if (auto *slot0 = newRack.getSlot(0))
            {
                expect(slot0->getGearItem() != nullptr, "Slot 0 should have a gear item after loading");
                if (auto *item = slot0->getGearItem())
                {
                    expect(item->name == "Test EQ", "Loaded item should have correct name");
                    expect(item->isInstance, "Loaded item should be an instance");
                }
            }

            if (auto *slot1 = newRack.getSlot(1))
            {
                expect(slot1->getGearItem() != nullptr, "Slot 1 should have a gear item after loading");
                if (auto *item = slot1->getGearItem())
                {
                    expect(item->name == "Test Compressor", "Loaded item should have correct name");
                    expect(item->isInstance, "Loaded item should be an instance");
                }
            }
        }

        beginTest("Preset List Operations");
        {
            auto &presetManager = PresetManager::getInstance();

            // Create some test presets
            Rack rack(mockFetcher);
            GearLibrary gearLibrary(mockFetcher, false);

            presetManager.savePreset("Preset A", &rack);
            presetManager.savePreset("Preset B", &rack);
            presetManager.savePreset("Preset C", &rack);

            // Test getting preset names
            auto presetNames = presetManager.getPresetNames();
            expect(presetNames.contains("Preset A"), "Should contain Preset A");
            expect(presetNames.contains("Preset B"), "Should contain Preset B");
            expect(presetNames.contains("Preset C"), "Should contain Preset C");

            // Test preset timestamps
            expect(presetManager.getPresetTimestamp("Preset A") > 0, "Preset should have a valid timestamp");
            expect(presetManager.getPresetTimestamp("NonExistent") == 0, "Non-existent preset should have zero timestamp");

            // Test display names
            auto displayName = presetManager.getPresetDisplayName("Preset A");
            expect(displayName.startsWith("Preset A ("), "Display name should start with preset name and opening parenthesis");
            expect(displayName.endsWith(")"), "Display name should end with closing parenthesis");
        }

        beginTest("Preset Delete");
        {
            auto &presetManager = PresetManager::getInstance();

            // Create a test preset
            Rack rack(mockFetcher);
            presetManager.savePreset("Delete Test", &rack);

            // Verify it exists
            expect(presetManager.isPresetValid("Delete Test"), "Preset should exist before deletion");

            // Delete it
            expect(presetManager.deletePreset("Delete Test"), "Should delete preset successfully");

            // Verify it's gone
            expect(!presetManager.isPresetValid("Delete Test"), "Preset should not exist after deletion");

            // Test deleting non-existent preset
            expect(presetManager.deletePreset("NonExistent"), "Should handle deleting non-existent preset gracefully");
        }

        beginTest("Control Values Preservation");
        {
            auto &presetManager = PresetManager::getInstance();

            Rack rack(mockFetcher);
            GearLibrary gearLibrary(mockFetcher, false);

            // Create a test gear item with controls
            juce::StringArray tags = {"test"};
            juce::Array<GearControl> controls;

            // Add a test control
            GearControl testControl;
            testControl.id = "test-control";
            testControl.name = "Test Control";
            testControl.type = GearControl::Type::Knob;
            testControl.position = {0.5f, 0.5f};
            testControl.value = 0.5f;
            testControl.initialValue = 0.5f;
            controls.add(testControl);

            auto testUnit = std::make_unique<GearItem>(
                "test-unit",
                "Test Unit",
                "Test Manufacturer",
                "equalizer",
                "1.0.0",
                "units/test-unit-1.0.0.json",
                "assets/thumbnails/test-unit-1.0.0.jpg",
                tags,
                mockFetcher,
                GearType::Rack19Inch,
                GearCategory::EQ,
                1,
                controls);

            if (auto *slot = rack.getSlot(0))
            {
                slot->setGearItem(testUnit.get());
                rack.createInstance(0);

                // Set control values
                if (auto *item = slot->getGearItem())
                {
                    if (item->controls.size() > 0)
                    {
                        GearControl &control = item->controls.getReference(0);
                        control.value = 0.75f;
                        control.initialValue = 0.5f;
                    }
                }
            }

            // Save preset
            expect(presetManager.savePreset("Control Test", &rack), "Should save preset with control values");

            // Load into new rack
            Rack newRack(mockFetcher);
            expect(presetManager.loadPreset("Control Test", &newRack, &gearLibrary), "Should load preset with control values");

            // Verify control values were preserved
            if (auto *slot = newRack.getSlot(0))
            {
                if (auto *item = slot->getGearItem())
                {
                    if (item->controls.size() > 0)
                    {
                        float controlValue = item->controls[0].value;
                        float initialValue = item->controls[0].initialValue;
                        expect(controlValue == 0.75f, "Control value should be preserved");
                        expect(initialValue == 0.5f, "Control initial value should be preserved");
                    }
                }
            }
        }

        beginTest("Error Handling");
        {
            auto &presetManager = PresetManager::getInstance();

            // Test saving with invalid parameters
            expect(!presetManager.savePreset("", nullptr), "Should fail to save with empty name");
            expect(!presetManager.savePreset("Valid Name", nullptr), "Should fail to save with null rack");

            // Test loading with invalid parameters
            expect(!presetManager.loadPreset("", nullptr, nullptr), "Should fail to load with empty name");
            expect(!presetManager.loadPreset("Valid Name", nullptr, nullptr), "Should fail to load with null rack");

            // Test loading non-existent preset
            Rack rack(mockFetcher);
            GearLibrary gearLibrary(mockFetcher, false);
            expect(!presetManager.loadPreset("NonExistent", &rack, &gearLibrary), "Should fail to load non-existent preset");
        }
    }
};

static PresetManagerTests presetManagerTests;