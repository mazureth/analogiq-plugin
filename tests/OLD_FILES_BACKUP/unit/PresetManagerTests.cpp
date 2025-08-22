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
#include "MockFileSystem.h"
#include "CacheManager.h"
#include "TestImageHelper.h"

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
        // Create a mock file system for testing
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFileSystem.reset();

        // Create instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        // Create gear library with mock dependencies
        GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);

        beginTest("Directory Management");
        {
            // Test presets directory path
            auto presetsDir = presetManager.getPresetsDirectory();
            expect(!presetsDir.isEmpty(), "Presets directory path should not be empty");

            // Test initialization
            expect(presetManager.initializePresetsDirectory(), "Should initialize presets directory");
        }

        beginTest("Preset Save and Load");
        {
            // Create a test rack with gear items
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);

            // Create test gear items
            const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
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
                ConcreteMockNetworkFetcher::getInstance(),
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
                ConcreteMockNetworkFetcher::getInstance(),
                mockFileSystem,
                cacheManager,
                GearType::Rack19Inch,
                GearCategory::Compressor,
                1,
                controls);

            // Add gear items to the library so they can be found during loading
            gearLibrary.addItem("test-eq", "Test EQ", "EQ", "Test Equalizer", "Test Manufacturer", true);
            gearLibrary.addItem("test-compressor", "Test Compressor", "Compressor", "Test Compressor", "Test Manufacturer", true);

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
            Rack newRack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);
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
            // Create some test presets
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);

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
            // Create a test preset
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);
            presetManager.savePreset("Delete Test", &rack);

            // Verify it exists
            expect(presetManager.isPresetValid("Delete Test"), "Preset should exist before deletion");

            // Delete it
            expect(presetManager.deletePreset("Delete Test"), "Should delete preset successfully");

            // Verify it's gone
            expect(!presetManager.isPresetValid("Delete Test"), "Preset should not exist after deletion");
        }

        beginTest("Preset Validation");
        {
            // Test valid preset names
            juce::String errorMessage;
            expect(presetManager.validatePresetName("Valid Preset", errorMessage), "Valid preset name should pass validation");
            expect(presetManager.validatePresetName("Another Valid", errorMessage), "Another valid preset name should pass validation");

            // Test invalid preset names
            expect(!presetManager.validatePresetName("", errorMessage), "Empty preset name should fail validation");
            expect(!presetManager.validatePresetName("Preset<Invalid>", errorMessage), "Preset name with invalid characters should fail validation");
            expect(!presetManager.validatePresetName("Preset:Invalid", errorMessage), "Preset name with invalid characters should fail validation");
        }

        beginTest("Preset File Operations");
        {
            // Create a test preset
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);
            presetManager.savePreset("File Test", &rack);

            // Test file validation
            juce::String errorMessage;
            expect(presetManager.validatePresetFile("File Test", errorMessage), "Valid preset file should pass validation");

            // Test non-existent preset
            expect(!presetManager.validatePresetFile("NonExistent", errorMessage), "Non-existent preset should fail validation");
        }

        beginTest("Preset Info and Metadata");
        {
            // Create a test preset
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);
            presetManager.savePreset("Info Test", &rack);

            // Test getting preset info
            juce::String errorMessage;
            auto presetInfo = presetManager.getPresetInfo("Info Test", errorMessage);
            expect(presetInfo.isObject(), "Preset info should be a valid object");

            // Test getting info for non-existent preset
            auto nonExistentInfo = presetManager.getPresetInfo("NonExistent", errorMessage);
            expect(!nonExistentInfo.isObject(), "Non-existent preset should return empty var");
        }

        beginTest("Preset Name Conflicts");
        {
            // Create a test preset
            GearLibrary gearLibrary(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager);
            Rack rack(ConcreteMockNetworkFetcher::getInstance(), mockFileSystem, cacheManager, presetManager, &gearLibrary);
            presetManager.savePreset("Conflict Test", &rack);

            // Test name conflict detection
            juce::String errorMessage;
            expect(presetManager.checkPresetNameConflict("Conflict Test", errorMessage), "Existing preset name should conflict");
            expect(!presetManager.checkPresetNameConflict("New Preset", errorMessage), "New preset name should not conflict");
        }

        beginTest("Error Handling");
        {
            // Test error message handling
            presetManager.clearLastError();
            expect(presetManager.getLastErrorMessage().isEmpty(), "Error message should be empty after clearing");

            // Test with invalid operations
            juce::String errorMessage;
            expect(!presetManager.validatePresetFile("NonExistent", errorMessage), "Should handle non-existent preset gracefully");
        }
    }
};

static PresetManagerTests presetManagerTests;