#pragma once
#include "../Shared/IPresetManager.h"
#include "../Shared/IFileSystem.h"
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>

class PresetManager : public IPresetManager
{
public:
    explicit PresetManager(IFileSystem &fileSystem);
    ~PresetManager() override;

    // IPresetManager implementation
    bool savePreset(const juce::String &presetName, const juce::AudioProcessorValueTreeState &state) override;
    bool loadPreset(const juce::String &presetName, juce::AudioProcessorValueTreeState &state) override;
    bool deletePreset(const juce::String &presetName) override;
    bool presetExists(const juce::String &presetName) override;
    juce::StringArray getPresetNames() override;
    juce::String getCurrentPresetName() const override;
    void setCurrentPresetName(const juce::String &presetName) override;

    bool createPresetCategory(const juce::String &categoryName) override;
    bool deletePresetCategory(const juce::String &categoryName) override;
    juce::StringArray getPresetCategories() override;
    bool assignPresetToCategory(const juce::String &presetName, const juce::String &categoryName) override;
    juce::StringArray getPresetsInCategory(const juce::String &categoryName) override;

    juce::String getPresetDescription(const juce::String &presetName) override;
    bool setPresetDescription(const juce::String &presetName, const juce::String &description) override;
    juce::Time getPresetCreationTime(const juce::String &presetName) override;
    juce::Time getPresetLastModifiedTime(const juce::String &presetName) override;
    juce::String getPresetAuthor(const juce::String &presetName) override;
    bool setPresetAuthor(const juce::String &presetName, const juce::String &author) override;

    bool validatePreset(const juce::String &presetName) override;
    bool isPresetCorrupted(const juce::String &presetName) override;
    bool repairPreset(const juce::String &presetName) override;

    bool exportPreset(const juce::String &presetName, const juce::String &exportPath) override;
    bool importPreset(const juce::String &importPath, const juce::String &presetName = "") override;
    bool exportPresetBank(const juce::StringArray &presetNames, const juce::String &exportPath) override;
    bool importPresetBank(const juce::String &importPath) override;

    juce::StringArray searchPresets(const juce::String &searchTerm) override;
    juce::StringArray filterPresetsByCategory(const juce::String &categoryName) override;
    juce::StringArray filterPresetsByAuthor(const juce::String &authorName) override;
    juce::StringArray filterPresetsByDateRange(const juce::Time &startDate, const juce::Time &endDate) override;

    int getTotalPresetCount() override;
    int getPresetCountInCategory(const juce::String &categoryName) override;
    juce::int64 getPresetFileSize(const juce::String &presetName) override;
    juce::int64 getTotalPresetStorageSize() override;

    bool backupPresets(const juce::String &backupPath) override;
    bool restorePresets(const juce::String &backupPath) override;
    bool createPresetBackup() override;
    juce::StringArray getAvailableBackups() override;

private:
    struct PresetMetadata
    {
        juce::String name;
        juce::String description;
        juce::String author;
        juce::String category;
        juce::Time creationTime;
        juce::Time lastModifiedTime;
        juce::int64 fileSize;
        bool isValid;
    };

    struct PresetCategory
    {
        juce::String name;
        juce::StringArray presetNames;
        juce::Time creationTime;
    };

    IFileSystem &fileSystem;
    juce::String presetsRootDir;
    juce::String currentPresetName;
    std::unordered_map<juce::String, PresetMetadata> presetMetadata;
    std::unordered_map<juce::String, PresetCategory> categories;

    // Helper methods
    void initializePresetsDirectory();
    juce::String generatePresetPath(const juce::String &presetName);
    juce::String generateMetadataPath(const juce::String &presetName);
    juce::String generateCategoryPath(const juce::String &categoryName);
    void loadPresetMetadata();
    void savePresetMetadata();
    void loadCategories();
    void saveCategories();
    bool validatePresetFile(const juce::String &presetPath);
    juce::String sanitizePresetName(const juce::String &presetName);
    juce::String generateBackupPath();
    bool createBackupDirectory();
};
