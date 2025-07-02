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

            // Add gear items to the library so they can be found during loading
            gearLibrary.addItem("Test EQ", "EQ", "Test Equalizer", "Test Manufacturer");
            gearLibrary.addItem("Test Compressor", "Compressor", "Test Compressor", "Test Manufacturer");

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
            expect(!presetManager.deletePreset("NonExistent"), "Should fail to delete non-existent preset");
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

            // Add gear item to the library so it can be found during loading
            gearLibrary.addItem("Test Unit", "EQ", "Test Unit", "Test Manufacturer");

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

        beginTest("Enhanced Error Handling and Validation");
        {
            auto &presetManager = PresetManager::getInstance();

            // Test getLastErrorMessage
            presetManager.clearLastError();
            expect(presetManager.getLastErrorMessage().isEmpty(), "Last error message should be empty after clear");

            // Test validatePresetName with various invalid names
            juce::String errorMessage;

            // Empty name
            expect(!presetManager.validatePresetName("", errorMessage), "Empty name should be invalid");
            expect(errorMessage.isNotEmpty(), "Should provide error message for empty name");

            // Whitespace only
            expect(!presetManager.validatePresetName("   ", errorMessage), "Whitespace-only name should be invalid");
            expect(errorMessage.isNotEmpty(), "Should provide error message for whitespace-only name");

            // Invalid characters
            expect(!presetManager.validatePresetName("test<name", errorMessage), "Name with < should be invalid");
            expect(errorMessage.contains("invalid characters"), "Should mention invalid characters");

            expect(!presetManager.validatePresetName("test:name", errorMessage), "Name with : should be invalid");
            expect(!presetManager.validatePresetName("test/name", errorMessage), "Name with / should be invalid");
            expect(!presetManager.validatePresetName("test\\name", errorMessage), "Name with \\ should be invalid");

            // Reserved names
            expect(!presetManager.validatePresetName("CON", errorMessage), "Reserved name CON should be invalid");
            expect(errorMessage.contains("reserved system name"), "Should mention reserved system name");

            expect(!presetManager.validatePresetName("prn", errorMessage), "Reserved name prn should be invalid");
            expect(!presetManager.validatePresetName("AUX", errorMessage), "Reserved name AUX should be invalid");
            expect(!presetManager.validatePresetName("nul", errorMessage), "Reserved name nul should be invalid");

            // Names starting/ending with dots or spaces
            expect(!presetManager.validatePresetName(".test", errorMessage), "Name starting with dot should be invalid");
            expect(!presetManager.validatePresetName("test.", errorMessage), "Name ending with dot should be invalid");
            expect(!presetManager.validatePresetName(" test", errorMessage), "Name starting with space should be invalid");
            expect(!presetManager.validatePresetName("test ", errorMessage), "Name ending with space should be invalid");

            // Valid names
            expect(presetManager.validatePresetName("Valid Name", errorMessage), "Valid name should pass validation");
            expect(errorMessage.isEmpty(), "Should not provide error message for valid name");

            expect(presetManager.validatePresetName("Test-Preset_123", errorMessage), "Name with hyphens and underscores should be valid");
            expect(presetManager.validatePresetName("My Preset", errorMessage), "Name with spaces should be valid");
        }

        beginTest("Preset Name Conflict Detection");
        {
            auto &presetManager = PresetManager::getInstance();

            // Create a test preset
            Rack rack(mockFetcher);
            presetManager.savePreset("Conflict Test", &rack);

            juce::String errorMessage;

            // Test case-insensitive conflict detection
            expect(presetManager.checkPresetNameConflict("CONFLICT TEST", errorMessage), "Should detect case-insensitive conflict");
            expect(errorMessage.contains("already exists"), "Should mention existing preset");

            expect(presetManager.checkPresetNameConflict("conflict test", errorMessage), "Should detect lowercase conflict");
            expect(presetManager.checkPresetNameConflict("Conflict Test", errorMessage), "Should detect exact match conflict");

            // Test non-conflicting names
            expect(!presetManager.checkPresetNameConflict("Different Name", errorMessage), "Different name should not conflict");
            expect(errorMessage.isEmpty(), "Should not provide error message for non-conflicting name");

            // Clean up
            presetManager.deletePreset("Conflict Test");
        }

        beginTest("Preset File Validation");
        {
            auto &presetManager = PresetManager::getInstance();

            juce::String errorMessage;

            // Test validation of non-existent preset
            expect(!presetManager.validatePresetFile("NonExistent", errorMessage), "Non-existent preset should fail validation");
            expect(errorMessage.contains("does not exist"), "Should mention file doesn't exist");

            // Create a valid preset for testing
            Rack rack(mockFetcher);
            presetManager.savePreset("Validation Test", &rack);

            // Test validation of valid preset
            expect(presetManager.validatePresetFile("Validation Test", errorMessage), "Valid preset should pass validation");
            expect(errorMessage.isEmpty(), "Should not provide error message for valid preset");

            // Test getPresetInfo
            auto presetInfo = presetManager.getPresetInfo("Validation Test", errorMessage);
            expect(presetInfo.isObject(), "Should return object for valid preset");
            expect(errorMessage.isEmpty(), "Should not provide error message for valid preset");

            if (presetInfo.isObject())
            {
                auto infoObj = presetInfo.getDynamicObject();
                expect(infoObj != nullptr, "Info object should not be null");
                if (infoObj != nullptr)
                {
                    expect(infoObj->hasProperty("name"), "Should have name property");
                    expect(infoObj->hasProperty("filename"), "Should have filename property");
                    expect(infoObj->hasProperty("fileSize"), "Should have fileSize property");
                    expect(infoObj->hasProperty("slotCount"), "Should have slotCount property");
                    expect(infoObj->hasProperty("gearItemCount"), "Should have gearItemCount property");
                }
            }

            // Test getPresetInfo for non-existent preset
            auto nonExistentInfo = presetManager.getPresetInfo("NonExistent", errorMessage);
            expect(!nonExistentInfo.isObject(), "Should not return object for non-existent preset");
            expect(errorMessage.isNotEmpty(), "Should provide error message for non-existent preset");

            // Clean up
            presetManager.deletePreset("Validation Test");
        }

        beginTest("Error Handling and Validation");
        {
            auto &presetManager = PresetManager::getInstance();

            // Test preset name validation
            juce::String errorMessage;
            expect(presetManager.validatePresetName("Valid Preset", errorMessage), "Valid preset name should pass validation");
            expect(presetManager.validatePresetName("Preset_123", errorMessage), "Preset name with underscores should pass validation");
            expect(presetManager.validatePresetName("A", errorMessage), "Single character preset name should pass validation");

            expect(!presetManager.validatePresetName("", errorMessage), "Empty preset name should fail validation");
            expect(!presetManager.validatePresetName("   ", errorMessage), "Whitespace-only preset name should fail validation");
            expect(!presetManager.validatePresetName("Preset/with/slashes", errorMessage), "Preset name with slashes should fail validation");
            expect(!presetManager.validatePresetName("Preset\\with\\backslashes", errorMessage), "Preset name with backslashes should fail validation");
            expect(!presetManager.validatePresetName("Preset:with:colons", errorMessage), "Preset name with colons should fail validation");
            expect(!presetManager.validatePresetName("Preset*with*asterisks", errorMessage), "Preset name with asterisks should fail validation");
            expect(!presetManager.validatePresetName("Preset?with?question", errorMessage), "Preset name with question marks should fail validation");
            expect(!presetManager.validatePresetName("Preset\"with\"quotes", errorMessage), "Preset name with quotes should fail validation");
            expect(!presetManager.validatePresetName("Preset<with>brackets", errorMessage), "Preset name with angle brackets should fail validation");
            expect(!presetManager.validatePresetName("Preset|with|pipes", errorMessage), "Preset name with pipes should fail validation");

            // Test preset name length validation
            juce::String longName;
            for (int i = 0; i < 256; ++i)
                longName += "a";
            expect(!presetManager.validatePresetName(longName, errorMessage), "Preset name that's too long should fail validation");

            // Test preset file validation
            Rack rack(mockFetcher);
            GearLibrary gearLibrary(mockFetcher, false);
            expect(presetManager.savePreset("ValidationTest", &rack), "Should save preset for validation test");
            expect(presetManager.validatePresetFile("ValidationTest", errorMessage), "Valid preset file should pass validation");
            expect(!presetManager.validatePresetFile("NonExistentPreset", errorMessage), "Non-existent preset file should fail validation");

            // Test preset conflict detection
            expect(presetManager.checkPresetNameConflict("ValidationTest", errorMessage), "Existing preset should be detected as conflict");
            expect(!presetManager.checkPresetNameConflict("DifferentPreset", errorMessage), "Different preset name should not be detected as conflict");

            // Test error message handling
            expect(!presetManager.savePreset("", &rack), "Saving with invalid name should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(), "Error message should be set after failed operation");

            expect(!presetManager.loadPreset("NonExistentPreset", &rack, &gearLibrary), "Loading non-existent preset should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(), "Error message should be set after failed load");

            expect(!presetManager.deletePreset("NonExistentPreset"), "Deleting non-existent preset should fail");
            expect(!presetManager.getLastErrorMessage().isEmpty(), "Error message should be set after failed delete");

            // Test display name functions
            auto displayName = presetManager.getPresetDisplayName("ValidationTest");
            expect(displayName.contains("ValidationTest"), "Display name should contain preset name");
            expect(displayName.contains("("), "Display name should contain opening parenthesis");
            expect(displayName.contains(")"), "Display name should contain closing parenthesis");

            auto displayNameNoTimestamp = presetManager.getPresetDisplayNameNoTimestamp("ValidationTest");
            expect(displayNameNoTimestamp == "ValidationTest", "Display name without timestamp should be just the preset name");

            // Test preset list functions
            auto presetNames = presetManager.getPresetNames();
            expect(presetNames.size() > 0, "Preset list should contain saved presets");

            // Clean up test preset
            presetManager.deletePreset("ValidationTest");
        }
    }
};

static PresetManagerTests presetManagerTests;