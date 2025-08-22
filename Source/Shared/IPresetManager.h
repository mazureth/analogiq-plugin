#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

class IPresetManager
{
public:
    virtual ~IPresetManager() = default;

    // Preset management
    virtual bool savePreset(const juce::String& presetName, const juce::AudioProcessorValueTreeState& state) = 0;
    virtual bool loadPreset(const juce::String& presetName, juce::AudioProcessorValueTreeState& state) = 0;
    virtual bool deletePreset(const juce::String& presetName) = 0;
    virtual bool presetExists(const juce::String& presetName) = 0;
    virtual juce::StringArray getPresetNames() = 0;
    virtual juce::String getCurrentPresetName() const = 0;
    virtual void setCurrentPresetName(const juce::String& presetName) = 0;

    // Preset categories and organization
    virtual bool createPresetCategory(const juce::String& categoryName) = 0;
    virtual bool deletePresetCategory(const juce::String& categoryName) = 0;
    virtual juce::StringArray getPresetCategories() = 0;
    virtual bool assignPresetToCategory(const juce::String& presetName, const juce::String& categoryName) = 0;
    virtual juce::StringArray getPresetsInCategory(const juce::String& categoryName) = 0;

    // Preset metadata
    virtual juce::String getPresetDescription(const juce::String& presetName) = 0;
    virtual bool setPresetDescription(const juce::String& presetName, const juce::String& description) = 0;
    virtual juce::Time getPresetCreationTime(const juce::String& presetName) = 0;
    virtual juce::Time getPresetLastModifiedTime(const juce::String& presetName) = 0;
    virtual juce::String getPresetAuthor(const juce::String& presetName) = 0;
    virtual bool setPresetAuthor(const juce::String& presetName, const juce::String& author) = 0;

    // Preset validation and integrity
    virtual bool validatePreset(const juce::String& presetName) = 0;
    virtual bool isPresetCorrupted(const juce::String& presetName) = 0;
    virtual bool repairPreset(const juce::String& presetName) = 0;

    // Preset import/export
    virtual bool exportPreset(const juce::String& presetName, const juce::String& exportPath) = 0;
    virtual bool importPreset(const juce::String& importPath, const juce::String& presetName = "") = 0;
    virtual bool exportPresetBank(const juce::StringArray& presetNames, const juce::String& exportPath) = 0;
    virtual bool importPresetBank(const juce::String& importPath) = 0;

    // Preset search and filtering
    virtual juce::StringArray searchPresets(const juce::String& searchTerm) = 0;
    virtual juce::StringArray filterPresetsByCategory(const juce::String& categoryName) = 0;
    virtual juce::StringArray filterPresetsByAuthor(const juce::String& authorName) = 0;
    virtual juce::StringArray filterPresetsByDateRange(const juce::Time& startDate, const juce::Time& endDate) = 0;

    // Preset statistics
    virtual int getTotalPresetCount() = 0;
    virtual int getPresetCountInCategory(const juce::String& categoryName) = 0;
    virtual juce::int64 getPresetFileSize(const juce::String& presetName) = 0;
    virtual juce::int64 getTotalPresetStorageSize() = 0;

    // Preset backup and restore
    virtual bool backupPresets(const juce::String& backupPath) = 0;
    virtual bool restorePresets(const juce::String& backupPath) = 0;
    virtual bool createPresetBackup() = 0;
    virtual juce::StringArray getAvailableBackups() = 0;

    // Null Object Pattern implementation
    static IPresetManager& getDummy();
};

// Dummy implementation for testing and fallback
class DummyPresetManager : public IPresetManager
{
public:
    bool savePreset(const juce::String&, const juce::AudioProcessorValueTreeState&) override { return false; }
    bool loadPreset(const juce::String&, juce::AudioProcessorValueTreeState&) override { return false; }
    bool deletePreset(const juce::String&) override { return false; }
    bool presetExists(const juce::String&) override { return false; }
    juce::StringArray getPresetNames() override { return juce::StringArray(); }
    juce::String getCurrentPresetName() const override { return ""; }
    void setCurrentPresetName(const juce::String&) override {}
    bool createPresetCategory(const juce::String&) override { return false; }
    bool deletePresetCategory(const juce::String&) override { return false; }
    juce::StringArray getPresetCategories() override { return juce::StringArray(); }
    bool assignPresetToCategory(const juce::String&, const juce::String&) override { return false; }
    juce::StringArray getPresetsInCategory(const juce::String&) override { return juce::StringArray(); }
    juce::String getPresetDescription(const juce::String&) override { return ""; }
    bool setPresetDescription(const juce::String&, const juce::String&) override { return false; }
    juce::Time getPresetCreationTime(const juce::String&) override { return juce::Time(); }
    juce::Time getPresetLastModifiedTime(const juce::String&) override { return juce::Time(); }
    juce::String getPresetAuthor(const juce::String&) override { return ""; }
    bool setPresetAuthor(const juce::String&, const juce::String&) override { return false; }
    bool validatePreset(const juce::String&) override { return false; }
    bool isPresetCorrupted(const juce::String&) override { return true; }
    bool repairPreset(const juce::String&) override { return false; }
    bool exportPreset(const juce::String&, const juce::String&) override { return false; }
    bool importPreset(const juce::String&, const juce::String&) override { return false; }
    bool exportPresetBank(const juce::StringArray&, const juce::String&) override { return false; }
    bool importPresetBank(const juce::String&) override { return false; }
    juce::StringArray searchPresets(const juce::String&) override { return juce::StringArray(); }
    juce::StringArray filterPresetsByCategory(const juce::String&) override { return juce::StringArray(); }
    juce::StringArray filterPresetsByAuthor(const juce::String&) override { return juce::StringArray(); }
    juce::StringArray filterPresetsByDateRange(const juce::Time&, const juce::Time&) override { return juce::StringArray(); }
    int getTotalPresetCount() override { return 0; }
    int getPresetCountInCategory(const juce::String&) override { return 0; }
    juce::int64 getPresetFileSize(const juce::String&) override { return 0; }
    juce::int64 getTotalPresetStorageSize() override { return 0; }
    bool backupPresets(const juce::String&) override { return false; }
    bool restorePresets(const juce::String&) override { return false; }
    bool createPresetBackup() override { return false; }
    juce::StringArray getAvailableBackups() override { return juce::StringArray(); }
};

inline IPresetManager& IPresetManager::getDummy()
{
    static DummyPresetManager dummy;
    return dummy;
}
