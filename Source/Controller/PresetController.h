/**
 * @file PresetController.h
 * @brief Header file for the PresetController class.
 *
 * This file defines the preset controller that coordinates between
 * the PresetManager model and UI components, handling preset
 * operations and state management.
 */

#pragma once

#include <JuceHeader.h>
#include "../Model/PresetManager.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"

// Forward declarations
class AnalogIQProcessor;
class Rack;

/**
 * @brief Controller for preset operations and coordination.
 *
 * The PresetController is responsible for:
 * - Coordinating between PresetManager (Model) and UI components
 * - Handling preset save/load/delete operations
 * - Managing preset validation and conflict resolution
 * - Coordinating with the audio processor and rack state
 * - Handling preset import/export operations
 * - Managing preset metadata and organization
 */
class PresetController
{
public:
    /**
     * @brief Constructs a new PresetController.
     *
     * @param presetManager Reference to the preset manager model
     * @param processor Reference to the audio processor
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     */
    PresetController(PresetManager &presetManager,
                     AnalogIQProcessor &processor,
                     IFileSystem &fileSystem,
                     ICacheManager &cacheManager);

    /**
     * @brief Destructor for PresetController.
     */
    ~PresetController() = default;

    // Preset Operations
    /**
     * @brief Saves the current state as a preset.
     *
     * @param presetName The name for the new preset
     * @param description Optional description for the preset
     * @param tags Optional tags for the preset
     * @return true if successful, false otherwise
     */
    bool savePreset(const juce::String &presetName,
                    const juce::String &description = "",
                    const juce::StringArray &tags = juce::StringArray());

    /**
     * @brief Loads a preset by name.
     *
     * @param presetName The name of the preset to load
     * @return true if successful, false otherwise
     */
    bool loadPreset(const juce::String &presetName);

    /**
     * @brief Saves the current rack state as a preset.
     *
     * @param presetName The name for the new preset
     * @param rackStateJSON The rack state as JSON string
     * @return true if successful, false otherwise
     */
    bool savePreset(const juce::String &presetName, const juce::String &rackStateJSON);

    /**
     * @brief Loads a preset by name and returns rack state.
     *
     * @param presetName The name of the preset to load
     * @param rackStateJSON Output parameter for the rack state JSON
     * @return true if successful, false otherwise
     */
    bool loadPreset(const juce::String &presetName, juce::String &rackStateJSON);

    /**
     * @brief Deletes a preset by name.
     *
     * @param presetName The name of the preset to delete
     * @return true if successful, false otherwise
     */
    bool deletePreset(const juce::String &presetName);

    /**
     * @brief Renames a preset.
     *
     * @param oldName The current preset name
     * @param newName The new preset name
     * @return true if successful, false otherwise
     */
    bool renamePreset(const juce::String &oldName, const juce::String &newName);

    /**
     * @brief Duplicates a preset.
     *
     * @param sourcePresetName The name of the preset to duplicate
     * @param newPresetName The name for the duplicate preset
     * @return true if successful, false otherwise
     */
    bool duplicatePreset(const juce::String &sourcePresetName, const juce::String &newPresetName);

    // Preset Information
    /**
     * @brief Gets the list of available preset names.
     *
     * @return Array of preset names
     */
    juce::StringArray getPresetNames() const;

    /**
     * @brief Gets the current preset name.
     *
     * @return The current preset name, or empty string if none
     */
    juce::String getCurrentPresetName() const;

    /**
     * @brief Gets the preset description.
     *
     * @param presetName The name of the preset
     * @return The preset description, or empty string if not found
     */
    juce::String getPresetDescription(const juce::String &presetName) const;

    /**
     * @brief Gets the preset tags.
     *
     * @param presetName The name of the preset
     * @return Array of preset tags
     */
    juce::StringArray getPresetTags(const juce::String &presetName) const;

    /**
     * @brief Gets the preset creation date.
     *
     * @param presetName The name of the preset
     * @return The preset creation date, or invalid time if not found
     */
    juce::Time getPresetCreationDate(const juce::String &presetName) const;

    /**
     * @brief Gets the preset modification date.
     *
     * @param presetName The name of the preset
     * @return The preset modification date, or invalid time if not found
     */
    juce::Time getPresetModificationDate(const juce::String &presetName) const;

    /**
     * @brief Gets the preset file size.
     *
     * @param presetName The name of the preset
     * @return The preset file size in bytes, or -1 if not found
     */
    juce::int64 getPresetFileSize(const juce::String &presetName) const;

    // Preset Validation
    /**
     * @brief Validates a preset name.
     *
     * @param presetName The preset name to validate
     * @param errorMessage Output parameter for error message
     * @return true if the name is valid, false otherwise
     */
    bool validatePresetName(const juce::String &presetName, juce::String &errorMessage) const;

    /**
     * @brief Checks for preset name conflicts.
     *
     * @param presetName The preset name to check
     * @param errorMessage Output parameter for error message
     * @return true if there's a conflict, false otherwise
     */
    bool checkPresetNameConflict(const juce::String &presetName, juce::String &errorMessage) const;

    /**
     * @brief Validates a preset file.
     *
     * @param filePath The file path to validate
     * @param errorMessage Output parameter for error message
     * @return true if the file is valid, false otherwise
     */
    bool validatePresetFile(const juce::String &filePath, juce::String &errorMessage) const;

    // Preset State Management
    /**
     * @brief Checks if there are unsaved changes.
     *
     * @return true if there are unsaved changes, false otherwise
     */
    bool hasUnsavedChanges() const;

    /**
     * @brief Marks the current state as having unsaved changes.
     */
    void markAsModified();

    /**
     * @brief Clears the modified state.
     */
    void clearModifiedState();

    /**
     * @brief Gets the last save time.
     *
     * @return The last save time, or invalid time if never saved
     */
    juce::Time getLastSaveTime() const;

    // Preset Import/Export
    /**
     * @brief Imports a preset from a file.
     *
     * @param filePath The file path to import from
     * @param presetName Optional name for the imported preset
     * @return true if successful, false otherwise
     */
    bool importPreset(const juce::String &filePath, const juce::String &presetName = "");

    /**
     * @brief Exports a preset to a file.
     *
     * @param presetName The name of the preset to export
     * @param filePath The file path to export to
     * @return true if successful, false otherwise
     */
    bool exportPreset(const juce::String &presetName, const juce::String &filePath);

    /**
     * @brief Exports all presets to a directory.
     *
     * @param directoryPath The directory path to export to
     * @return true if successful, false otherwise
     */
    bool exportAllPresets(const juce::String &directoryPath);

    /**
     * @brief Imports presets from a directory.
     *
     * @param directoryPath The directory path to import from
     * @return Number of presets successfully imported
     */
    int importPresetsFromDirectory(const juce::String &directoryPath);

    // Preset Organization
    /**
     * @brief Creates a preset category.
     *
     * @param categoryName The name of the category to create
     * @return true if successful, false otherwise
     */
    bool createPresetCategory(const juce::String &categoryName);

    /**
     * @brief Deletes a preset category.
     *
     * @param categoryName The name of the category to delete
     * @return true if successful, false otherwise
     */
    bool deletePresetCategory(const juce::String &categoryName);

    /**
     * @brief Gets all preset categories.
     *
     * @return Array of category names
     */
    juce::StringArray getPresetCategories() const;

    /**
     * @brief Assigns a preset to a category.
     *
     * @param presetName The name of the preset
     * @param categoryName The name of the category
     * @return true if successful, false otherwise
     */
    bool assignPresetToCategory(const juce::String &presetName, const juce::String &categoryName);

    /**
     * @brief Gets presets in a category.
     *
     * @param categoryName The name of the category
     * @return Array of preset names in the category
     */
    juce::StringArray getPresetsInCategory(const juce::String &categoryName) const;

    // Preset Search and Filtering
    /**
     * @brief Searches for presets by name.
     *
     * @param searchTerm The search term to use
     * @return Array of matching preset names
     */
    juce::StringArray searchPresetsByName(const juce::String &searchTerm) const;

    /**
     * @brief Searches for presets by tag.
     *
     * @param tag The tag to search for
     * @return Array of matching preset names
     */
    juce::StringArray searchPresetsByTag(const juce::String &tag) const;

    /**
     * @brief Searches for presets by description.
     *
     * @param searchTerm The search term to use
     * @return Array of matching preset names
     */
    juce::StringArray searchPresetsByDescription(const juce::String &searchTerm) const;

    /**
     * @brief Filters presets by creation date range.
     *
     * @param startDate The start date
     * @param endDate The end date
     * @return Array of matching preset names
     */
    juce::StringArray filterPresetsByDateRange(const juce::Time &startDate, const juce::Time &endDate) const;

    // Preset Statistics
    /**
     * @brief Gets the total number of presets.
     *
     * @return Total number of presets
     */
    int getTotalPresetCount() const;

    /**
     * @brief Gets the number of presets in a category.
     *
     * @param categoryName The name of the category
     * @return Number of presets in the category
     */
    int getPresetCountInCategory(const juce::String &categoryName) const;

    /**
     * @brief Gets preset usage statistics.
     *
     * @return Map of preset names to usage counts
     */
    std::map<juce::String, int> getPresetUsageStats() const;

    /**
     * @brief Gets preset creation statistics.
     *
     * @return Map of creation dates to preset counts
     */
    std::map<juce::String, int> getPresetCreationStats() const;

    // Preset Backup and Recovery
    /**
     * @brief Creates a backup of all presets.
     *
     * @param backupPath The backup file path
     * @return true if successful, false otherwise
     */
    bool createPresetBackup(const juce::String &backupPath);

    /**
     * @brief Restores presets from a backup.
     *
     * @param backupPath The backup file path
     * @return true if successful, false otherwise
     */
    bool restorePresetsFromBackup(const juce::String &backupPath);

    /**
     * @brief Gets the last backup time.
     *
     * @return The last backup time, or invalid time if never backed up
     */
    juce::Time getLastBackupTime() const;

private:
    // Core dependencies
    PresetManager &presetManager;
    AnalogIQProcessor &processor;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;

    // State tracking
    juce::String currentPresetName;
    bool isModified{false};
    juce::Time lastSaveTime;
    juce::Time lastBackupTime;

    // Private helper methods
    /**
     * @brief Captures the current processor and rack state.
     *
     * @return Memory block containing the captured state
     */
    juce::MemoryBlock captureCurrentState() const;

    /**
     * @brief Restores state to the processor and rack.
     *
     * @param stateData The state data to restore
     * @return true if successful, false otherwise
     */
    bool restoreState(const juce::MemoryBlock &stateData);

    /**
     * @brief Generates a unique preset name.
     *
     * @param baseName The base name to use
     * @return A unique preset name
     */
    juce::String generateUniquePresetName(const juce::String &baseName) const;

    /**
     * @brief Validates that a file path is accessible.
     *
     * @param filePath The file path to validate
     * @return true if accessible, false otherwise
     */
    bool isFilePathAccessible(const juce::String &filePath) const;

    /**
     * @brief Gets the preset file path.
     *
     * @param presetName The name of the preset
     * @return The preset file path
     */
    juce::String getPresetFilePath(const juce::String &presetName) const;

    /**
     * @brief Updates the current preset name.
     *
     * @param presetName The new preset name
     */
    void updateCurrentPresetName(const juce::String &presetName);

    /**
     * @brief Notifies the processor of preset changes.
     */
    void notifyProcessorOfPresetChange();

    /**
     * @brief Logs preset operations for debugging.
     *
     * @param operation The operation being performed
     * @param presetName The name of the preset
     * @param success Whether the operation was successful
     */
    void logPresetOperation(const juce::String &operation, const juce::String &presetName, bool success) const;
};
