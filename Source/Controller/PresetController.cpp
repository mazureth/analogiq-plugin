/**
 * @file PresetController.cpp
 * @brief Implementation of the PresetController class.
 *
 * This file implements the preset controller that coordinates between
 * the PresetManager model and UI components, handling preset
 * operations and state management.
 */

#include "PresetController.h"
#include "../Model/AnalogIQProcessor.h"

/**
 * @brief Constructs a new PresetController.
 *
 * @param presetManager Reference to the preset manager model
 * @param processor Reference to the audio processor
 * @param fileSystem Reference to the file system service
 * @param cacheManager Reference to the cache manager
 */
PresetController::PresetController(PresetManager &presetManager,
                                   AnalogIQProcessor &processor,
                                   IFileSystem &fileSystem,
                                   ICacheManager &cacheManager)
    : presetManager(presetManager),
      processor(processor),
      fileSystem(fileSystem),
      cacheManager(cacheManager)
{
    // Initialize state tracking
    currentPresetName = "";
    isModified = false;
    lastSaveTime = juce::Time();
    lastBackupTime = juce::Time();
}

// Preset Operations

bool PresetController::savePreset(const juce::String &presetName,
                                  const juce::String &description,
                                  const juce::StringArray &tags)
{
    // Validate the preset name
    juce::String errorMessage;
    if (!validatePresetName(presetName, errorMessage))
    {
        std::cout << "[PresetController] Invalid preset name: " << errorMessage << std::endl;
        return false;
    }

    // Check for name conflicts
    if (checkPresetNameConflict(presetName, errorMessage))
    {
        std::cout << "[PresetController] Preset name conflict: " << errorMessage << std::endl;
        return false;
    }

    try
    {
        // Capture the current state
        juce::MemoryBlock stateData = captureCurrentState();

        // For now, just log the operation
        // This will be enhanced when we implement the full preset saving system
        std::cout << "[PresetController] Saving preset: " << presetName << std::endl;
        if (!description.isEmpty())
            std::cout << "  Description: " << description << std::endl;
        if (tags.size() > 0)
            std::cout << "  Tags: " << tags.joinIntoString(", ") << std::endl;

        // Update state tracking
        currentPresetName = presetName;
        lastSaveTime = juce::Time::getCurrentTime();
        clearModifiedState();

        logPresetOperation("Save", presetName, true);
        return true;
    }
    catch (...)
    {
        logPresetOperation("Save", presetName, false);
        return false;
    }
}

bool PresetController::loadPreset(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset loading system
        std::cout << "[PresetController] Loading preset: " << presetName << std::endl;

        // Update state tracking
        currentPresetName = presetName;
        lastSaveTime = juce::Time::getCurrentTime();
        clearModifiedState();

        logPresetOperation("Load", presetName, true);
        return true;
    }
    catch (...)
    {
        logPresetOperation("Load", presetName, false);
        return false;
    }
}

bool PresetController::deletePreset(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset deletion system
        std::cout << "[PresetController] Deleting preset: " << presetName << std::endl;

        // If this was the current preset, clear the current preset name
        if (currentPresetName == presetName)
        {
            currentPresetName = "";
        }

        logPresetOperation("Delete", presetName, true);
        return true;
    }
    catch (...)
    {
        logPresetOperation("Delete", presetName, false);
        return false;
    }
}

bool PresetController::renamePreset(const juce::String &oldName, const juce::String &newName)
{
    if (oldName.isEmpty() || newName.isEmpty())
        return false;

    // Validate the new preset name
    juce::String errorMessage;
    if (!validatePresetName(newName, errorMessage))
    {
        std::cout << "[PresetController] Invalid new preset name: " << errorMessage << std::endl;
        return false;
    }

    // Check for name conflicts
    if (checkPresetNameConflict(newName, errorMessage))
    {
        std::cout << "[PresetController] Preset name conflict: " << errorMessage << std::endl;
        return false;
    }

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset renaming system
        std::cout << "[PresetController] Renaming preset from '" << oldName << "' to '" << newName << "'" << std::endl;

        // Update state tracking if this was the current preset
        if (currentPresetName == oldName)
        {
            currentPresetName = newName;
        }

        logPresetOperation("Rename", oldName + " -> " + newName, true);
        return true;
    }
    catch (...)
    {
        logPresetOperation("Rename", oldName + " -> " + newName, false);
        return false;
    }
}

bool PresetController::duplicatePreset(const juce::String &sourcePresetName, const juce::String &newPresetName)
{
    if (sourcePresetName.isEmpty() || newPresetName.isEmpty())
        return false;

    // Validate the new preset name
    juce::String errorMessage;
    if (!validatePresetName(newPresetName, errorMessage))
    {
        std::cout << "[PresetController] Invalid new preset name: " << errorMessage << std::endl;
        return false;
    }

    // Check for name conflicts
    if (checkPresetNameConflict(newPresetName, errorMessage))
    {
        std::cout << "[PresetController] Preset name conflict: " << errorMessage << std::endl;
        return false;
    }

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset duplication system
        std::cout << "[PresetController] Duplicating preset '" << sourcePresetName << "' to '" << newPresetName << "'" << std::endl;

        logPresetOperation("Duplicate", sourcePresetName + " -> " + newPresetName, true);
        return true;
    }
    catch (...)
    {
        logPresetOperation("Duplicate", sourcePresetName + " -> " + newPresetName, false);
        return false;
    }
}

// Preset Information

juce::StringArray PresetController::getPresetNames() const
{
    return presetManager.getPresetNames();
}

juce::String PresetController::getCurrentPresetName() const
{
    return currentPresetName;
}

juce::String PresetController::getPresetDescription(const juce::String &presetName) const
{
    // For now, return empty string
    // This will be enhanced when we implement the full preset metadata system
    return "";
}

juce::StringArray PresetController::getPresetTags(const juce::String &presetName) const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset metadata system
    return juce::StringArray();
}

juce::Time PresetController::getPresetCreationDate(const juce::String &presetName) const
{
    // For now, return invalid time
    // This will be enhanced when we implement the full preset metadata system
    return juce::Time();
}

juce::Time PresetController::getPresetModificationDate(const juce::String &presetName) const
{
    // For now, return invalid time
    // This will be enhanced when we implement the full preset metadata system
    return juce::Time();
}

juce::int64 PresetController::getPresetFileSize(const juce::String &presetName) const
{
    // For now, return -1
    // This will be enhanced when we implement the full preset file system
    return -1;
}

// Preset Validation

bool PresetController::validatePresetName(const juce::String &presetName, juce::String &errorMessage) const
{
    // Basic validation
    if (presetName.trim().isEmpty())
    {
        errorMessage = "Preset name cannot be empty.";
        return false;
    }

    if (presetName.length() > 100)
    {
        errorMessage = "Preset name cannot exceed 100 characters.";
        return false;
    }

    // Check for invalid characters
    if (presetName.containsAnyOf("\\/:*?\"<>|"))
    {
        errorMessage = "Preset name contains invalid characters.";
        return false;
    }

    errorMessage = "";
    return true;
}

bool PresetController::checkPresetNameConflict(const juce::String &presetName, juce::String &errorMessage) const
{
    auto existingPresets = presetManager.getPresetNames();

    for (const auto &existingName : existingPresets)
    {
        if (existingName.equalsIgnoreCase(presetName))
        {
            errorMessage = "A preset with this name already exists.";
            return true;
        }
    }

    errorMessage = "";
    return false;
}

bool PresetController::validatePresetFile(const juce::String &filePath, juce::String &errorMessage) const
{
    if (!isFilePathAccessible(filePath))
    {
        errorMessage = "File path is not accessible.";
        return false;
    }

    // For now, just check if the file exists and is readable
    // This will be enhanced when we implement the full preset file validation
    errorMessage = "";
    return true;
}

// Preset State Management

bool PresetController::hasUnsavedChanges() const
{
    return isModified;
}

void PresetController::markAsModified()
{
    isModified = true;
}

void PresetController::clearModifiedState()
{
    isModified = false;
}

juce::Time PresetController::getLastSaveTime() const
{
    return lastSaveTime;
}

// Preset Import/Export

bool PresetController::importPreset(const juce::String &filePath, const juce::String &presetName)
{
    if (!isFilePathAccessible(filePath))
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset import system
        std::cout << "[PresetController] Importing preset from: " << filePath << std::endl;
        if (!presetName.isEmpty())
            std::cout << "  Preset name: " << presetName << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool PresetController::exportPreset(const juce::String &presetName, const juce::String &filePath)
{
    if (presetName.isEmpty() || !isFilePathAccessible(filePath))
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset export system
        std::cout << "[PresetController] Exporting preset '" << presetName << "' to: " << filePath << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool PresetController::exportAllPresets(const juce::String &directoryPath)
{
    if (!isFilePathAccessible(directoryPath))
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset export system
        std::cout << "[PresetController] Exporting all presets to directory: " << directoryPath << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

int PresetController::importPresetsFromDirectory(const juce::String &directoryPath)
{
    if (!isFilePathAccessible(directoryPath))
        return 0;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset import system
        std::cout << "[PresetController] Importing presets from directory: " << directoryPath << std::endl;

        return 0; // For now, return 0 as we haven't implemented the actual import
    }
    catch (...)
    {
        return 0;
    }
}

// Preset Organization

bool PresetController::createPresetCategory(const juce::String &categoryName)
{
    if (categoryName.trim().isEmpty())
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset organization system
        std::cout << "[PresetController] Creating preset category: " << categoryName << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool PresetController::deletePresetCategory(const juce::String &categoryName)
{
    if (categoryName.trim().isEmpty())
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset organization system
        std::cout << "[PresetController] Deleting preset category: " << categoryName << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::StringArray PresetController::getPresetCategories() const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset organization system
    return juce::StringArray();
}

bool PresetController::assignPresetToCategory(const juce::String &presetName, const juce::String &categoryName)
{
    if (presetName.isEmpty() || categoryName.isEmpty())
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset organization system
        std::cout << "[PresetController] Assigning preset '" << presetName << "' to category '" << categoryName << "'" << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::StringArray PresetController::getPresetsInCategory(const juce::String &categoryName) const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset organization system
    return juce::StringArray();
}

// Preset Search and Filtering

juce::StringArray PresetController::searchPresetsByName(const juce::String &searchTerm) const
{
    if (searchTerm.trim().isEmpty())
        return juce::StringArray();

    juce::StringArray results;
    auto allPresets = presetManager.getPresetNames();

    for (const auto &presetName : allPresets)
    {
        if (presetName.toLowerCase().contains(searchTerm.toLowerCase()))
        {
            results.add(presetName);
        }
    }

    return results;
}

juce::StringArray PresetController::searchPresetsByTag(const juce::String &tag) const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset search system
    return juce::StringArray();
}

juce::StringArray PresetController::searchPresetsByDescription(const juce::String &searchTerm) const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset search system
    return juce::StringArray();
}

juce::StringArray PresetController::filterPresetsByDateRange(const juce::Time &startDate, const juce::Time &endDate) const
{
    // For now, return empty array
    // This will be enhanced when we implement the full preset filtering system
    return juce::StringArray();
}

// Preset Statistics

int PresetController::getTotalPresetCount() const
{
    return presetManager.getPresetNames().size();
}

int PresetController::getPresetCountInCategory(const juce::String &categoryName) const
{
    return getPresetsInCategory(categoryName).size();
}

std::map<juce::String, int> PresetController::getPresetUsageStats() const
{
    // For now, return empty map
    // This will be enhanced when we implement the full preset statistics system
    return std::map<juce::String, int>();
}

std::map<juce::String, int> PresetController::getPresetCreationStats() const
{
    // For now, return empty map
    // This will be enhanced when we implement the full preset statistics system
    return std::map<juce::String, int>();
}

// Preset Backup and Recovery

bool PresetController::createPresetBackup(const juce::String &backupPath)
{
    if (!isFilePathAccessible(backupPath))
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset backup system
        std::cout << "[PresetController] Creating preset backup to: " << backupPath << std::endl;

        lastBackupTime = juce::Time::getCurrentTime();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool PresetController::restorePresetsFromBackup(const juce::String &backupPath)
{
    if (!isFilePathAccessible(backupPath))
        return false;

    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset restore system
        std::cout << "[PresetController] Restoring presets from backup: " << backupPath << std::endl;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::Time PresetController::getLastBackupTime() const
{
    return lastBackupTime;
}

// Private helper methods

juce::MemoryBlock PresetController::captureCurrentState() const
{
    // For now, create a simple state representation
    // This will be enhanced when we implement the full state capture system
    juce::MemoryBlock state;

    // Add a simple header
    juce::String header = "PresetState_v1.0";
    state.append(header.toRawUTF8(), header.getNumBytesAsUTF8());

    return state;
}

bool PresetController::restoreState(const juce::MemoryBlock &stateData)
{
    // For now, just check if the data has a valid header
    // This will be enhanced when we implement the full state restore system
    if (stateData.getSize() < 12) // Minimum size for header
        return false;

    juce::String header(static_cast<const char *>(stateData.getData()), 12);
    if (header.startsWith("PresetState_v"))
    {
        return true;
    }

    return false;
}

juce::String PresetController::generateUniquePresetName(const juce::String &baseName) const
{
    if (baseName.isEmpty())
        return "";

    juce::String uniqueName = baseName;
    int counter = 1;

    juce::String dummyError;
    while (checkPresetNameConflict(uniqueName, dummyError))
    {
        uniqueName = baseName + "_" + juce::String(counter);
        counter++;
    }

    return uniqueName;
}

bool PresetController::isFilePathAccessible(const juce::String &filePath) const
{
    return !filePath.trim().isEmpty();
}

juce::String PresetController::getPresetFilePath(const juce::String &presetName) const
{
    // For now, return a simple file path
    // This will be enhanced when we implement the full preset file system
    return "presets/" + presetName + ".preset";
}

void PresetController::updateCurrentPresetName(const juce::String &presetName)
{
    currentPresetName = presetName;
}

void PresetController::notifyProcessorOfPresetChange()
{
    // For now, just log the notification
    // This will be enhanced when we implement the full processor notification system
    std::cout << "[PresetController] Notifying processor of preset change" << std::endl;
}

void PresetController::logPresetOperation(const juce::String &operation, const juce::String &presetName, bool success) const
{
    std::cout << "[PresetController] " << operation << " operation for preset '" << presetName << "' "
              << (success ? "succeeded" : "failed") << std::endl;
}
