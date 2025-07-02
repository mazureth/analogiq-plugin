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
     * @brief Gets the singleton instance of PresetManager.
     *
     * @return Reference to the PresetManager instance
     */
    static PresetManager &getInstance();

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
     * @return The presets directory as a JUCE File object
     */
    juce::File getPresetsDirectory() const;

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
     * @brief Initializes the presets directory structure.
     *
     * Creates the presets directory if it doesn't exist.
     *
     * @return true if initialization was successful, false otherwise
     */
    bool initializePresetsDirectory() const;

private:
    /**
     * @brief Private constructor for singleton pattern.
     */
    PresetManager() = default;

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
     * @brief Gets the preset file for a given name.
     *
     * @param name The preset name
     * @return The preset file
     */
    juce::File getPresetFile(const juce::String &name) const;
};