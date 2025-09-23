#include "PresetManager.h"
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

PresetManager::PresetManager(IFileSystem &fs)
    : fileSystem(fs), currentPresetName("Default"), initialized(false)
{
    // Lazy initialization - no file system operations during construction
}

PresetManager::~PresetManager()
{
    savePresetMetadata();
    saveCategories();
}

void PresetManager::initializeLazy()
{
    if (initialized)
        return;

    initializePresetsDirectory();
    loadPresetMetadata();
    loadCategories();

    initialized = true;
}

void PresetManager::initializePresetsDirectory()
{
    presetsRootDir = fileSystem.joinPath(fileSystem.getCacheRootDirectory(), "Presets");
    if (!fileSystem.directoryExists(presetsRootDir))
    {
        fileSystem.createDirectory(presetsRootDir);
    }
}

juce::String PresetManager::generatePresetPath(const juce::String &presetName)
{
    auto sanitizedName = sanitizePresetName(presetName);
    return fileSystem.joinPath(presetsRootDir, sanitizedName + ".preset");
}

juce::String PresetManager::generateMetadataPath(const juce::String &presetName)
{
    auto sanitizedName = sanitizePresetName(presetName);
    return fileSystem.joinPath(presetsRootDir, sanitizedName + ".meta");
}

juce::String PresetManager::generateCategoryPath(const juce::String &categoryName)
{
    return fileSystem.joinPath(presetsRootDir, "categories.txt");
}

juce::String PresetManager::sanitizePresetName(const juce::String &presetName)
{
    // Remove invalid characters for filenames
    auto sanitized = presetName.replaceCharacters("<>:\"/\\|?*", "_________");
    return sanitized.trim();
}

bool PresetManager::savePreset(const juce::String &presetName, const juce::AudioProcessorValueTreeState &state)
{
    initializeLazy();
    if (presetName.isEmpty())
        return false;

    auto presetPath = generatePresetPath(presetName);
    auto metadataPath = generateMetadataPath(presetName);

    // Save the preset state
    auto presetXml = state.state.createXml();
    if (!presetXml)
        return false;

    juce::MemoryBlock presetData;
    juce::MemoryOutputStream stream(presetData, false);
    presetXml->writeTo(stream);

    if (!fileSystem.writeFile(presetPath, presetData))
        return false;

    // Create or update metadata
    PresetMetadata metadata;
    metadata.name = presetName;
    metadata.creationTime = (presetMetadata.find(presetName) != presetMetadata.end())
                                ? presetMetadata[presetName].creationTime // Preserve existing creation time
                                : juce::Time::getCurrentTime();           // Set new creation time for new presets
    metadata.lastModifiedTime = juce::Time::getCurrentTime();
    metadata.fileSize = presetData.getSize();
    metadata.isValid = true;

    // Preserve existing metadata if available
    if (presetMetadata.find(presetName) != presetMetadata.end())
    {
        auto &existing = presetMetadata[presetName];
        metadata.description = existing.description;
        metadata.author = existing.author;
        metadata.category = existing.category;
    }

    presetMetadata[presetName] = metadata;
    savePresetMetadata();

    return true;
}

bool PresetManager::loadPreset(const juce::String &presetName, juce::AudioProcessorValueTreeState &state)
{
    initializeLazy();
    if (presetName.isEmpty() || !presetExists(presetName))
        return false;

    auto presetPath = generatePresetPath(presetName);
    auto presetData = fileSystem.readBinaryFile(presetPath);

    if (presetData.getSize() == 0)
        return false;

    auto presetXml = juce::XmlDocument::parse(presetData.toString());
    if (!presetXml)
        return false;

    state.state = juce::ValueTree::fromXml(*presetXml);
    setCurrentPresetName(presetName);

    // Update last accessed time
    if (presetMetadata.find(presetName) != presetMetadata.end())
    {
        presetMetadata[presetName].lastModifiedTime = juce::Time::getCurrentTime();
        savePresetMetadata();
    }

    return true;
}

bool PresetManager::deletePreset(const juce::String &presetName)
{
    if (presetName.isEmpty() || !presetExists(presetName))
        return false;

    auto presetPath = generatePresetPath(presetName);
    auto metadataPath = generateMetadataPath(presetName);

    bool success = true;
    success &= fileSystem.deleteFile(presetPath);
    success &= fileSystem.deleteFile(metadataPath);

    if (success)
    {
        presetMetadata.erase(presetName);
        savePresetMetadata();
    }

    return success;
}

bool PresetManager::presetExists(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return false;
    auto presetPath = generatePresetPath(presetName);
    return fileSystem.fileExists(presetPath);
}

juce::StringArray PresetManager::getPresetNames()
{
    initializeLazy();
    juce::StringArray names;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid)
            names.add(preset.first);
    }
    return names;
}

juce::String PresetManager::getCurrentPresetName() const
{
    return currentPresetName;
}

void PresetManager::setCurrentPresetName(const juce::String &presetName)
{
    currentPresetName = presetName;
}

bool PresetManager::createPresetCategory(const juce::String &categoryName)
{
    if (categoryName.isEmpty())
        return false;

    if (categories.find(categoryName) != categories.end())
        return true; // Already exists

    PresetCategory category;
    category.name = categoryName;
    category.creationTime = juce::Time::getCurrentTime();
    categories[categoryName] = category;

    saveCategories();
    return true;
}

bool PresetManager::deletePresetCategory(const juce::String &categoryName)
{
    if (categoryName.isEmpty())
        return false;

    auto it = categories.find(categoryName);
    if (it == categories.end())
        return false;

    // Remove category from all presets
    for (auto &preset : presetMetadata)
    {
        if (preset.second.category == categoryName)
            preset.second.category = "";
    }

    categories.erase(it);
    saveCategories();
    savePresetMetadata();

    return true;
}

juce::StringArray PresetManager::getPresetCategories()
{
    juce::StringArray categoryNames;
    for (auto &category : categories)
    {
        categoryNames.add(category.first);
    }
    return categoryNames;
}

bool PresetManager::assignPresetToCategory(const juce::String &presetName, const juce::String &categoryName)
{
    if (presetName.isEmpty() || categoryName.isEmpty())
        return false;
    if (categories.find(categoryName) == categories.end())
        return false;
    if (presetMetadata.find(presetName) == presetMetadata.end())
        return false;

    presetMetadata[presetName].category = categoryName;
    categories[categoryName].presetNames.addIfNotAlreadyThere(presetName);

    savePresetMetadata();
    saveCategories();
    return true;
}

juce::StringArray PresetManager::getPresetsInCategory(const juce::String &categoryName)
{
    if (categoryName.isEmpty())
        return juce::StringArray();

    auto it = categories.find(categoryName);
    if (it == categories.end())
        return juce::StringArray();

    return it->second.presetNames;
}

juce::String PresetManager::getPresetDescription(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return "";

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
        return it->second.description;

    return "";
}

bool PresetManager::setPresetDescription(const juce::String &presetName, const juce::String &description)
{
    if (presetName.isEmpty())
        return false;

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
    {
        it->second.description = description;
        savePresetMetadata();
        return true;
    }

    return false;
}

juce::Time PresetManager::getPresetCreationTime(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return juce::Time();

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
        return it->second.creationTime;

    return juce::Time();
}

juce::Time PresetManager::getPresetLastModifiedTime(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return juce::Time();

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
        return it->second.lastModifiedTime;

    return juce::Time();
}

juce::String PresetManager::getPresetAuthor(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return "";

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
        return it->second.author;

    return "";
}

bool PresetManager::setPresetAuthor(const juce::String &presetName, const juce::String &author)
{
    if (presetName.isEmpty())
        return false;

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
    {
        it->second.author = author;
        savePresetMetadata();
        return true;
    }

    return false;
}

bool PresetManager::validatePreset(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return false;

    auto presetPath = generatePresetPath(presetName);
    return validatePresetFile(presetPath);
}

bool PresetManager::isPresetCorrupted(const juce::String &presetName)
{
    return !validatePreset(presetName);
}

bool PresetManager::repairPreset(const juce::String &presetName)
{
    // For now, just mark as invalid - actual repair logic would be more complex
    if (presetName.isEmpty())
        return false;

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
    {
        it->second.isValid = false;
        savePresetMetadata();
        return true;
    }

    return false;
}

bool PresetManager::exportPreset(const juce::String &presetName, const juce::String &exportPath)
{
    if (presetName.isEmpty() || !presetExists(presetName))
        return false;

    auto presetPath = generatePresetPath(presetName);
    auto presetData = fileSystem.readBinaryFile(presetPath);

    if (presetData.getSize() == 0)
        return false;

    return fileSystem.writeFile(exportPath, presetData);
}

bool PresetManager::importPreset(const juce::String &importPath, const juce::String &presetName)
{
    if (importPath.isEmpty())
        return false;

    auto presetData = fileSystem.readBinaryFile(importPath);
    if (presetData.getSize() == 0)
        return false;

    auto presetXml = juce::XmlDocument::parse(presetData.toString());
    if (!presetXml)
        return false;

    auto finalPresetName = presetName.isEmpty() ? fileSystem.getFileName(importPath).replaceCharacters(".preset", "") : presetName;

    auto presetPath = generatePresetPath(finalPresetName);

    if (fileSystem.writeFile(presetPath, presetData))
    {
        // Create metadata for imported preset
        PresetMetadata metadata;
        metadata.name = finalPresetName;
        metadata.creationTime = juce::Time::getCurrentTime();
        metadata.lastModifiedTime = juce::Time::getCurrentTime();
        metadata.fileSize = presetData.getSize();
        metadata.isValid = true;

        presetMetadata[finalPresetName] = metadata;
        savePresetMetadata();

        return true;
    }

    return false;
}

bool PresetManager::exportPresetBank(const juce::StringArray &presetNames, const juce::String &exportPath)
{
    if (presetNames.isEmpty() || exportPath.isEmpty())
        return false;

    // Create a simple bank format - concatenated presets with header
    juce::MemoryBlock bankData;
    juce::MemoryOutputStream bankStream(bankData, false);

    // Write bank header
    bankStream.writeString("AnalogIQ Preset Bank\n");
    bankStream.writeString("Version: 1.0\n");
    bankStream.writeString("Preset Count: " + juce::String(presetNames.size()) + "\n");
    bankStream.writeString("---\n");

    // Write each preset
    for (auto &presetName : presetNames)
    {
        if (presetExists(presetName))
        {
            auto presetPath = generatePresetPath(presetName);
            auto presetData = fileSystem.readBinaryFile(presetPath);

            bankStream.writeString("PRESET: " + presetName + "\n");
            bankStream.writeString("SIZE: " + juce::String(presetData.getSize()) + "\n");
            bankStream.write(presetData.getData(), presetData.getSize());
            bankStream.writeString("\n---\n");
        }
    }

    return fileSystem.writeFile(exportPath, bankData);
}

bool PresetManager::importPresetBank(const juce::String &importPath)
{
    // This is a simplified import - would need more robust parsing in production
    if (importPath.isEmpty())
        return false;

    auto bankData = fileSystem.readBinaryFile(importPath);
    if (bankData.getSize() == 0)
        return false;

    auto bankContent = bankData.toString();
    auto lines = juce::StringArray::fromLines(bankContent);

    // Simple parsing - look for preset markers
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].startsWith("PRESET: "))
        {
            auto presetName = lines[i].substring(8).trim();
            // Would need more sophisticated parsing to extract preset data
            // For now, just create a placeholder
            createPresetCategory("Imported");
            assignPresetToCategory(presetName, "Imported");
        }
    }

    return true;
}

juce::StringArray PresetManager::searchPresets(const juce::String &searchTerm)
{
    if (searchTerm.isEmpty())
        return getPresetNames();

    juce::StringArray results;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid)
        {
            auto &metadata = preset.second;
            if (metadata.name.containsIgnoreCase(searchTerm) ||
                metadata.description.containsIgnoreCase(searchTerm) ||
                metadata.author.containsIgnoreCase(searchTerm))
            {
                results.add(preset.first);
            }
        }
    }
    return results;
}

juce::StringArray PresetManager::filterPresetsByCategory(const juce::String &categoryName)
{
    return getPresetsInCategory(categoryName);
}

juce::StringArray PresetManager::filterPresetsByAuthor(const juce::String &authorName)
{
    if (authorName.isEmpty())
        return getPresetNames();

    juce::StringArray results;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid && preset.second.author.containsIgnoreCase(authorName))
        {
            results.add(preset.first);
        }
    }
    return results;
}

juce::StringArray PresetManager::filterPresetsByDateRange(const juce::Time &startDate, const juce::Time &endDate)
{
    juce::StringArray results;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid)
        {
            auto creationTime = preset.second.creationTime;
            if (creationTime >= startDate && creationTime <= endDate)
            {
                results.add(preset.first);
            }
        }
    }
    return results;
}

int PresetManager::getTotalPresetCount()
{
    int count = 0;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid)
            count++;
    }
    return count;
}

int PresetManager::getPresetCountInCategory(const juce::String &categoryName)
{
    if (categoryName.isEmpty())
        return 0;

    auto it = categories.find(categoryName);
    if (it != categories.end())
        return it->second.presetNames.size();

    return 0;
}

juce::int64 PresetManager::getPresetFileSize(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return 0;

    auto it = presetMetadata.find(presetName);
    if (it != presetMetadata.end())
        return it->second.fileSize;

    return 0;
}

juce::int64 PresetManager::getTotalPresetStorageSize()
{
    juce::int64 totalSize = 0;
    for (auto &preset : presetMetadata)
    {
        if (preset.second.isValid)
            totalSize += preset.second.fileSize;
    }
    return totalSize;
}

bool PresetManager::backupPresets(const juce::String &backupPath)
{
    if (backupPath.isEmpty())
        return false;

    auto presets = getPresetNames();
    return exportPresetBank(presets, backupPath);
}

bool PresetManager::restorePresets(const juce::String &backupPath)
{
    return importPresetBank(backupPath);
}

bool PresetManager::createPresetBackup()
{
    auto backupPath = generateBackupPath();
    return backupPresets(backupPath);
}

juce::StringArray PresetManager::getAvailableBackups()
{
    auto backupDir = fileSystem.joinPath(presetsRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
        return juce::StringArray();

    auto files = fileSystem.getFiles(backupDir);
    juce::StringArray backups;
    for (auto &file : files)
    {
        if (file.endsWith(".backup"))
            backups.add(fileSystem.getFileName(file));
    }
    return backups;
}

void PresetManager::loadPresetMetadata()
{
    auto metadataIndexPath = fileSystem.joinPath(presetsRootDir, "preset_index.txt");
    if (fileSystem.fileExists(metadataIndexPath))
    {
        auto content = fileSystem.readFile(metadataIndexPath);
        auto lines = juce::StringArray::fromLines(content);

        for (auto &line : lines)
        {
            auto parts = juce::StringArray::fromTokens(line, "|", "");
            if (parts.size() >= 8)
            {
                PresetMetadata metadata;
                metadata.name = parts[0];
                metadata.description = parts[1];
                metadata.author = parts[2];
                metadata.category = parts[3];
                metadata.creationTime = juce::Time(parts[4].getLargeIntValue());
                metadata.lastModifiedTime = juce::Time(parts[5].getLargeIntValue());
                metadata.fileSize = parts[6].getLargeIntValue();
                metadata.isValid = parts[7].getIntValue() != 0;

                presetMetadata[metadata.name] = metadata;
            }
        }
    }
}

void PresetManager::savePresetMetadata()
{
    auto metadataIndexPath = fileSystem.joinPath(presetsRootDir, "preset_index.txt");
    juce::String content;

    for (auto &preset : presetMetadata)
    {
        auto &metadata = preset.second;
        content += metadata.name + "|" +
                   metadata.description + "|" +
                   metadata.author + "|" +
                   metadata.category + "|" +
                   juce::String(metadata.creationTime.toMilliseconds()) + "|" +
                   juce::String(metadata.lastModifiedTime.toMilliseconds()) + "|" +
                   juce::String(metadata.fileSize) + "|" +
                   juce::String(metadata.isValid ? 1 : 0) + "\n";
    }

    fileSystem.writeFile(metadataIndexPath, content);
}

void PresetManager::loadCategories()
{
    auto categoryPath = generateCategoryPath("");
    if (fileSystem.fileExists(categoryPath))
    {
        auto content = fileSystem.readFile(categoryPath);
        auto lines = juce::StringArray::fromLines(content);

        for (auto &line : lines)
        {
            auto parts = juce::StringArray::fromTokens(line, "|", "");
            if (parts.size() >= 3)
            {
                PresetCategory category;
                category.name = parts[0];
                category.creationTime = juce::Time(parts[1].getLargeIntValue());

                auto presetNames = juce::StringArray::fromTokens(parts[2], ",", "");
                category.presetNames = presetNames;

                categories[category.name] = category;
            }
        }
    }
}

void PresetManager::saveCategories()
{
    auto categoryPath = generateCategoryPath("");
    juce::String content;

    for (auto &category : categories)
    {
        auto &cat = category.second;
        content += cat.name + "|" +
                   juce::String(cat.creationTime.toMilliseconds()) + "|" +
                   cat.presetNames.joinIntoString(",") + "\n";
    }

    fileSystem.writeFile(categoryPath, content);
}

bool PresetManager::validatePresetFile(const juce::String &presetPath)
{
    if (!fileSystem.fileExists(presetPath))
        return false;

    auto presetData = fileSystem.readBinaryFile(presetPath);
    if (presetData.getSize() == 0)
        return false;

    auto presetXml = juce::XmlDocument::parse(presetData.toString());
    return presetXml != nullptr;
}

juce::String PresetManager::generateBackupPath()
{
    auto backupDir = fileSystem.joinPath(presetsRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
    {
        fileSystem.createDirectory(backupDir);
    }

    auto timestamp = juce::Time::getCurrentTime();
    auto backupName = "backup_" + timestamp.formatted("%Y%m%d_%H%M%S") + ".backup";
    return fileSystem.joinPath(backupDir, backupName);
}

bool PresetManager::createBackupDirectory()
{
    auto backupDir = fileSystem.joinPath(presetsRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
    {
        return fileSystem.createDirectory(backupDir);
    }
    return true;
}

// Rack-specific preset methods
bool PresetManager::savePreset(const juce::String &presetName, const juce::String &rackStateJSON)
{
    initializeLazy();
    if (presetName.isEmpty())
    {
        return false;
    }

    auto presetPath = generatePresetPath(presetName);
    auto metadataPath = generateMetadataPath(presetName);

    // Save the rack state JSON
    juce::MemoryBlock presetData;
    presetData.append(rackStateJSON.toRawUTF8(), rackStateJSON.getNumBytesAsUTF8());

    if (!fileSystem.writeFile(presetPath, presetData))
    {
        return false;
    }

    // Create or update metadata
    PresetMetadata metadata;
    metadata.name = presetName;
    metadata.creationTime = (presetMetadata.find(presetName) != presetMetadata.end())
                                ? presetMetadata[presetName].creationTime // Preserve existing creation time
                                : juce::Time::getCurrentTime();           // Set new creation time for new presets
    metadata.lastModifiedTime = juce::Time::getCurrentTime();
    metadata.fileSize = presetData.getSize();
    metadata.isValid = true;

    // Preserve existing metadata if available
    if (presetMetadata.find(presetName) != presetMetadata.end())
    {
        auto &existing = presetMetadata[presetName];
        metadata.description = existing.description;
        metadata.author = existing.author;
        metadata.category = existing.category;
    }

    presetMetadata[presetName] = metadata;
    savePresetMetadata();

    return true;
}

bool PresetManager::loadPreset(const juce::String &presetName, juce::String &rackStateJSON)
{
    initializeLazy();
    if (presetName.isEmpty() || !presetExists(presetName))
        return false;

    auto presetPath = generatePresetPath(presetName);
    auto presetData = fileSystem.readBinaryFile(presetPath);

    if (presetData.getSize() == 0)
        return false;

    // Convert binary data to string
    rackStateJSON = juce::String::fromUTF8(static_cast<const char *>(presetData.getData()), static_cast<int>(presetData.getSize()));

    setCurrentPresetName(presetName);

    // Update last accessed time
    if (presetMetadata.find(presetName) != presetMetadata.end())
    {
        presetMetadata[presetName].lastModifiedTime = juce::Time::getCurrentTime();
        savePresetMetadata();
    }

    return true;
}
