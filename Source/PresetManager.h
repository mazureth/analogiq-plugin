/**
 * @file PresetManager.h
 * @brief Header file for the PresetManager class.
 *
 * This file defines the PresetManager class, which handles saving, loading, and managing preset files. Presets include the complete state of
 * all gear instances in the rack, including control values and settings.
 */

#pragma once

#include <JuceHeader.h>
#include "Rack.h"
#include "GearLibrary.h"
#include "IFileSystem.h"
#include "FileSystem.h"

// Forward declarations
class Rack;
class GearLibrary;

/**
 * @brief Manages preset operations for the Analogiq plugin.
 *
 * The PresetManager class handles saving and loading of complete rack configurations
 * as presets. Each preset contains the ordered list of gear units, their instance
 * states, and control values. Presets are stored as JSON files in the user's
 * application data directory.
 */
class PresetManager
{
public:
    /**
     * @brief Constructor for PresetManager.
     *
     * @param fileSystem Reference to the file system implementation
     * @param cacheManager Reference to the cache manager implementation
     */
    PresetManager(IFileSystem &fileSystem, CacheManager &cacheManager);

    /**
     * @brief Destructor.
     */
    ~PresetManager() = default;

    // Prevent copying and assignment
    PresetManager(const PresetManager &) = delete;
    PresetManager &operator=(const PresetManager &) = delete;

    // Core preset operations
    /**
     * @brief Saves the current rack configuration as a preset.
     *
     * @param name The name of the preset to save
     * @param rack Pointer to the rack to save
     * @return true if the preset was saved successfully, false otherwise
     */
    bool savePreset(const juce::String &name, Rack *rack);

    /**
     * @brief Loads a preset into the rack.
     *
     * @param name The name of the preset to load
     * @param rack Pointer to the rack to load into
     * @param gearLibrary Pointer to the gear library for unit lookup
     * @return true if the preset was loaded successfully, false otherwise
     */
    bool loadPreset(const juce::String &name, Rack *rack, GearLibrary *gearLibrary);

    /**
     * @brief Deletes a preset file.
     *
     * @param name The name of the preset to delete
     * @return true if the preset was deleted successfully, false otherwise
     */
    bool deletePreset(const juce::String &name);

    /**
     * @brief Gets a list of all available preset names.
     *
     * @return Array of preset names
     */
    juce::StringArray getPresetNames() const;

    // Utility methods
    /**
     * @brief Gets the presets directory.
     *
     * @return The presets directory path as a string
     */
    juce::String getPresetsDirectory() const;

    /**
     * @brief Checks if a preset exists and is valid.
     *
     * @param name The name of the preset to check
     * @return true if the preset exists and is valid, false otherwise
     */
    bool isPresetValid(const juce::String &name) const;

    /**
     * @brief Gets the creation timestamp of a preset.
     *
     * @param name The name of the preset
     * @return The timestamp as a 64-bit integer, or 0 if preset doesn't exist
     */
    juce::int64 getPresetTimestamp(const juce::String &name) const;

    /**
     * @brief Gets the display name for a preset (formatted with timestamp).
     *
     * @param name The name of the preset
     * @return The formatted display name
     */
    juce::String getPresetDisplayName(const juce::String &name) const;

    /**
     * @brief Gets the display name for a preset (without timestamp).
     *
     * @param name The name of the preset
     * @return The display name without timestamp
     */
    juce::String getPresetDisplayNameNoTimestamp(const juce::String &name) const;

    /**
     * @brief Initializes the presets directory structure.
     *
     * Creates the presets directory if it doesn't exist.
     *
     * @return true if initialization was successful, false otherwise
     */
    bool initializePresetsDirectory() const;

    // Error handling and validation methods
    /**
     * @brief Gets the last error message.
     *
     * @return The last error message, or empty string if no error
     */
    juce::String getLastErrorMessage() const;

    /**
     * @brief Clears the last error message.
     */
    void clearLastError();

    /**
     * @brief Validates a preset name with detailed error reporting.
     *
     * @param name The preset name to validate
     * @param errorMessage Output parameter for detailed error message
     * @return true if the name is valid, false otherwise
     */
    bool validatePresetName(const juce::String &name, juce::String &errorMessage) const;

    /**
     * @brief Validates a preset file for corruption or format issues.
     *
     * @param name The preset name to validate
     * @param errorMessage Output parameter for detailed error message
     * @return true if the preset is valid, false otherwise
     */
    bool validatePresetFile(const juce::String &name, juce::String &errorMessage) const;

    /**
     * @brief Checks if a preset name conflicts with existing presets.
     *
     * @param name The preset name to check
     * @param errorMessage Output parameter for detailed error message
     * @return true if the name conflicts, false otherwise
     */
    bool checkPresetNameConflict(const juce::String &name, juce::String &errorMessage) const;

    /**
     * @brief Gets detailed information about a preset.
     *
     * @param name The preset name
     * @param errorMessage Output parameter for error message if preset doesn't exist
     * @return A JSON object with preset details, or empty var if preset doesn't exist
     */
    juce::var getPresetInfo(const juce::String &name, juce::String &errorMessage) const;

private:
    mutable juce::String lastErrorMessage; ///< Stores the last error message
    IFileSystem &fileSystem;               ///< Reference to the file system implementation
    CacheManager &cacheManager;            ///< Reference to the cache manager

    /**
     * @brief Converts a preset name to a safe filename.
     *
     * @param name The preset name to convert
     * @return A safe filename for the preset
     */
    juce::String nameToFilename(const juce::String &name) const;

    /**
     * @brief Converts a filename back to a preset name.
     *
     * @param filename The filename to convert
     * @return The original preset name
     */
    juce::String filenameToName(const juce::String &filename) const;

    /**
     * @brief Validates a preset name.
     *
     * @param name The preset name to validate
     * @return true if the name is valid, false otherwise
     */
    bool isValidPresetName(const juce::String &name) const;

    /**
     * @brief Serializes a rack configuration to JSON.
     *
     * @param rack Pointer to the rack to serialize
     * @return JSON string representing the rack configuration
     */
    juce::String serializeRackToJSON(Rack *rack) const;

    /**
     * @brief Deserializes a JSON string to a rack configuration.
     *
     * @param jsonData The JSON data to deserialize
     * @param rack Pointer to the rack to load into
     * @param gearLibrary Pointer to the gear library for unit lookup
     * @return true if deserialization was successful, false otherwise
     */
    bool deserializeJSONToRack(const juce::String &jsonData, Rack *rack, GearLibrary *gearLibrary) const;

    /**
     * @brief Gets the preset file path for a given name.
     *
     * @param name The preset name
     * @return The preset file path as a string
     */
    juce::String getPresetFile(const juce::String &name) const;
};