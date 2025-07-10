/**
 * @file PresetManager.cpp
 * @brief Implementation of the PresetManager class.
 *
 * This file implements the PresetManager class, which handles saving and loading
 * of rack configurations as presets. Presets include the complete state of
 * all gear instances in the rack, including control values and settings.
 */

#include "PresetManager.h"
#include "CacheManager.h"
#include "GearItem.h"
#include "FileSystem.h"
#include "Rack.h"
#include "GearLibrary.h"
#include "IFileSystem.h"
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

/**
 * @brief Constructor for PresetManager.
 *
 * @param fileSystem Reference to the file system implementation
 * @param cacheManager Reference to the cache manager implementation
 */
PresetManager::PresetManager(IFileSystem &fileSystem, CacheManager &cacheManager)
    : fileSystem(fileSystem), cacheManager(cacheManager)
{
}

/**
 * @brief Gets the presets directory.
 *
 * @return The presets directory as a JUCE File object
 */
juce::String PresetManager::getPresetsDirectory() const
{
    // Use OS-agnostic approach through the injected fileSystem
    juce::String cacheRoot = fileSystem.getCacheRootDirectory();
    return fileSystem.joinPath(cacheRoot, "presets");
}

/**
 * @brief Initializes the presets directory structure.
 *
 * Creates the presets directory if it doesn't exist.
 *
 * @return true if initialization was successful, false otherwise
 */
bool PresetManager::initializePresetsDirectory() const
{
    // Use OS-agnostic approach through the injected fileSystem
    juce::String cacheRoot = fileSystem.getCacheRootDirectory();
    juce::String presetsDir = fileSystem.joinPath(cacheRoot, "presets");

    // Create directories if they don't exist
    if (!fileSystem.directoryExists(cacheRoot))
        fileSystem.createDirectory(cacheRoot);

    if (!fileSystem.directoryExists(presetsDir))
        fileSystem.createDirectory(presetsDir);

    return fileSystem.directoryExists(presetsDir);
}

/**
 * @brief Converts a preset name to a safe filename.
 *
 * @param name The preset name to convert
 * @return A safe filename for the preset
 */
juce::String PresetManager::nameToFilename(const juce::String &name) const
{
    // Replace invalid characters with underscores
    juce::String filename = name;
    filename = filename.replaceCharacters("<>:\"/\\|?*", "_");
    filename = filename.trim();

    // Ensure it's not empty
    if (filename.isEmpty())
        filename = "untitled";

    // Add .json extension
    return filename + ".json";
}

/**
 * @brief Converts a filename back to a preset name.
 *
 * @param filename The filename to convert
 * @return The original preset name
 */
juce::String PresetManager::filenameToName(const juce::String &filename) const
{
    // Remove .json extension
    if (filename.endsWith(".json"))
        return filename.substring(0, filename.length() - 5);
    return filename;
}

/**
 * @brief Validates a preset name.
 *
 * @param name The preset name to validate
 * @return true if the name is valid, false otherwise
 */
bool PresetManager::isValidPresetName(const juce::String &name) const
{
    if (name.trim().isEmpty())
        return false;

    // Check for invalid characters
    if (name.containsAnyOf("<>:\"/\\|?*"))
        return false;

    return true;
}

/**
 * @brief Gets the preset file for a given name.
 *
 * @param name The preset name
 * @return The preset file
 */
juce::String PresetManager::getPresetFile(const juce::String &name) const
{
    // Use OS-agnostic approach through the injected fileSystem
    juce::String cacheRoot = fileSystem.getCacheRootDirectory();
    juce::String presetsDir = fileSystem.joinPath(cacheRoot, "presets");
    return fileSystem.joinPath(presetsDir, nameToFilename(name));
}

/**
 * @brief Serializes a rack configuration to JSON.
 *
 * @param rack Pointer to the rack to serialize
 * @return JSON string representing the rack configuration
 */
juce::String PresetManager::serializeRackToJSON(Rack *rack) const
{
    if (rack == nullptr)
        return "{}";

    juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();

    // Add metadata
    jsonObj->setProperty("version", "1.0");
    jsonObj->setProperty("timestamp", juce::Time::getCurrentTime().toMilliseconds());

    // Serialize slots
    juce::Array<juce::var> slotsArray;

    for (int i = 0; i < rack->getNumSlots(); ++i)
    {
        if (auto *slot = rack->getSlot(i))
        {
            if (auto *item = slot->getGearItem())
            {
                // Only save slots that have gear items
                juce::DynamicObject::Ptr slotObj = new juce::DynamicObject();
                slotObj->setProperty("slotIndex", i);
                slotObj->setProperty("unitId", item->unitId);

                // Save instance information
                if (item->isInstance)
                {
                    slotObj->setProperty("instanceId", item->instanceId);
                    slotObj->setProperty("sourceUnitId", item->sourceUnitId);
                }

                // Serialize controls
                juce::Array<juce::var> controlsArray;
                for (int j = 0; j < item->controls.size(); ++j)
                {
                    const auto &control = item->controls[j];
                    juce::DynamicObject::Ptr controlObj = new juce::DynamicObject();
                    controlObj->setProperty("index", j);
                    controlObj->setProperty("value", control.value);
                    controlObj->setProperty("initialValue", control.initialValue);

                    if (control.type == GearControl::Type::Switch)
                    {
                        controlObj->setProperty("currentIndex", control.currentIndex);
                    }

                    controlsArray.add(juce::var(controlObj));
                }
                slotObj->setProperty("controls", controlsArray);

                slotsArray.add(juce::var(slotObj));
            }
        }
    }

    jsonObj->setProperty("slots", slotsArray);

    return juce::JSON::toString(juce::var(jsonObj));
}

/**
 * @brief Deserializes a JSON string to a rack configuration.
 *
 * @param jsonData The JSON data to deserialize
 * @param rack Pointer to the rack to load into
 * @param gearLibrary Pointer to the gear library for unit lookup
 * @return true if deserialization was successful, false otherwise
 */
bool PresetManager::deserializeJSONToRack(const juce::String &jsonData, Rack *rack, GearLibrary *gearLibrary) const
{
    if (rack == nullptr || gearLibrary == nullptr)
        return false;

    // Parse JSON
    auto jsonVar = juce::JSON::parse(jsonData);
    if (!jsonVar.isObject())
        return false;

    auto jsonObj = jsonVar.getDynamicObject();
    if (jsonObj == nullptr)
        return false;

    // Clear current rack
    for (int i = 0; i < rack->getNumSlots(); ++i)
    {
        if (auto *slot = rack->getSlot(i))
        {
            slot->clearGearItem();
        }
    }

    // Load slots
    auto slotsVar = jsonObj->getProperty("slots");
    if (slotsVar.isArray())
    {
        auto slotsArray = slotsVar.getArray();
        for (auto slotVar : *slotsArray)
        {
            if (slotVar.isObject())
            {
                auto slotObj = slotVar.getDynamicObject();
                if (slotObj != nullptr)
                {
                    int slotIndex = slotObj->getProperty("slotIndex");
                    juce::String unitId = slotObj->getProperty("unitId");

                    if (slotIndex >= 0 && slotIndex < rack->getNumSlots() && !unitId.isEmpty())
                    {
                        // Find the gear item in the library
                        const auto &items = gearLibrary->getItems();
                        const GearItem *sourceItem = nullptr;

                        for (int i = 0; i < items.size(); ++i)
                        {
                            const auto &item = items.getReference(i);
                            if (item.unitId == unitId)
                            {
                                sourceItem = &item;
                                break;
                            }
                        }

                        if (sourceItem != nullptr)
                        {
                            // Create a new instance of the gear item
                            GearItem *newItem = new GearItem(*sourceItem, INetworkFetcher::getDummy(), fileSystem, cacheManager);

                            // Check if this was an instance and restore instance properties
                            auto instanceIdVar = slotObj->getProperty("instanceId");
                            auto sourceUnitIdVar = slotObj->getProperty("sourceUnitId");

                            if (instanceIdVar.isString() && sourceUnitIdVar.isString())
                            {
                                newItem->isInstance = true;
                                newItem->instanceId = instanceIdVar.toString();
                                newItem->sourceUnitId = sourceUnitIdVar.toString();
                            }

                            // Store saved control values for later application
                            struct SavedControlValues
                            {
                                int index;
                                float value;
                                float initialValue;
                                int currentIndex;
                            };
                            juce::Array<SavedControlValues> savedControls;

                            // Extract control values from preset
                            auto controlsVar = slotObj->getProperty("controls");
                            if (controlsVar.isArray())
                            {
                                auto controlsArray = controlsVar.getArray();
                                for (auto controlVar : *controlsArray)
                                {
                                    if (controlVar.isObject())
                                    {
                                        auto controlObj = controlVar.getDynamicObject();
                                        if (controlObj != nullptr)
                                        {
                                            SavedControlValues saved;
                                            saved.index = controlObj->getProperty("index");
                                            saved.value = controlObj->getProperty("value");
                                            saved.initialValue = controlObj->getProperty("initialValue");
                                            saved.currentIndex = controlObj->getProperty("currentIndex");
                                            savedControls.add(saved);
                                        }
                                    }
                                }
                            }

                            // Set it in the slot
                            if (auto *slot = rack->getSlot(slotIndex))
                            {
                                slot->setGearItem(newItem);

                                // Trigger the same loading sequence as normal gear item addition
                                // This will fetch schema, faceplate, and control images
                                // After schema parsing, apply the saved control values
                                rack->fetchSchemaForGearItem(newItem, [newItem, savedControls]()
                                                             {
                                    // Apply saved control values after schema parsing
                                    for (const auto& saved : savedControls)
                                    {
                                        if (saved.index >= 0 && saved.index < newItem->controls.size())
                                        {
                                            auto &control = newItem->controls.getReference(saved.index);
                                            control.value = saved.value;
                                            control.initialValue = saved.initialValue;

                                            if (control.type == GearControl::Type::Switch)
                                            {
                                                control.currentIndex = saved.currentIndex;
                                            }
                                        }
                                    } });
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

/**
 * @brief Saves the current rack configuration as a preset.
 *
 * @param name The name of the preset to save
 * @param rack Pointer to the rack to save
 * @return true if the preset was saved successfully, false otherwise
 */
bool PresetManager::savePreset(const juce::String &name, Rack *rack)
{
    clearLastError();

    // Validate preset name
    juce::String validationError;
    if (!validatePresetName(name, validationError))
    {
        lastErrorMessage = validationError;
        return false;
    }

    // Check for name conflicts
    juce::String conflictError;
    if (checkPresetNameConflict(name, conflictError))
    {
        lastErrorMessage = conflictError;
        return false;
    }

    // Validate rack pointer
    if (rack == nullptr)
    {
        lastErrorMessage = "Rack pointer is null.";
        return false;
    }

    // Initialize presets directory
    if (!initializePresetsDirectory())
    {
        lastErrorMessage = "Failed to create presets directory.";
        return false;
    }

    // Serialize rack to JSON
    juce::String jsonData = serializeRackToJSON(rack);
    if (jsonData.isEmpty())
    {
        lastErrorMessage = "Failed to serialize rack configuration.";
        return false;
    }

    // Save to file
    auto presetFile = getPresetFile(name);
    if (!fileSystem.writeFile(presetFile, jsonData))
    {
        lastErrorMessage = "Failed to write preset file to disk.";
        return false;
    }

    return true;
}

/**
 * @brief Loads a preset into the rack.
 *
 * @param name The name of the preset to load
 * @param rack Pointer to the rack to load into
 * @param gearLibrary Pointer to the gear library for unit lookup
 * @return true if the preset was loaded successfully, false otherwise
 */
bool PresetManager::loadPreset(const juce::String &name, Rack *rack, GearLibrary *gearLibrary)
{
    clearLastError();

    // Validate preset name
    if (name.isEmpty())
    {
        lastErrorMessage = "Preset name is empty.";
        return false;
    }

    // Validate rack pointer
    if (rack == nullptr)
    {
        lastErrorMessage = "Rack pointer is null.";
        return false;
    }

    // Validate gear library pointer
    if (gearLibrary == nullptr)
    {
        lastErrorMessage = "Gear library pointer is null.";
        return false;
    }

    // Validate preset file
    juce::String validationError;
    if (!validatePresetFile(name, validationError))
    {
        lastErrorMessage = validationError;
        return false;
    }

    // Load JSON data
    auto presetFile = getPresetFile(name);
    if (!fileSystem.fileExists(presetFile))
    {
        lastErrorMessage = "Preset file does not exist.";
        return false;
    }

    juce::String jsonData = fileSystem.readFile(presetFile);
    if (jsonData.isEmpty())
    {
        lastErrorMessage = "Failed to read preset file.";
        return false;
    }

    // Deserialize to rack
    if (!deserializeJSONToRack(jsonData, rack, gearLibrary))
    {
        lastErrorMessage = "Failed to deserialize preset data.";
        return false;
    }

    return true;
}

/**
 * @brief Deletes a preset file.
 *
 * @param name The name of the preset to delete
 * @return true if the preset was deleted successfully, false otherwise
 */
bool PresetManager::deletePreset(const juce::String &name)
{
    clearLastError();

    // Validate preset name
    if (name.isEmpty())
    {
        lastErrorMessage = "Preset name is empty.";
        return false;
    }

    auto presetFile = getPresetFile(name);
    if (!fileSystem.fileExists(presetFile))
    {
        lastErrorMessage = "Preset file does not exist.";
        return false;
    }

    if (!fileSystem.deleteFile(presetFile))
    {
        lastErrorMessage = "Failed to delete preset file.";
        return false;
    }

    return true;
}

/**
 * @brief Gets a list of all available preset names.
 *
 * @return Array of preset names
 */
juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;

    // Use OS-agnostic approach through the injected fileSystem
    juce::String cacheRoot = fileSystem.getCacheRootDirectory();
    juce::String presetsDir = fileSystem.joinPath(cacheRoot, "presets");

    if (fileSystem.directoryExists(presetsDir))
    {
        auto files = fileSystem.getFiles(presetsDir);
        for (auto &filename : files)
        {
            if (filename.endsWith(".json"))
            {
                names.add(filenameToName(filename));
            }
        }

        // Sort alphabetically
        names.sort(true);
    }

    return names;
}

/**
 * @brief Checks if a preset exists and is valid.
 *
 * @param name The name of the preset to check
 * @return true if the preset exists and is valid, false otherwise
 */
bool PresetManager::isPresetValid(const juce::String &name) const
{
    if (name.isEmpty())
        return false;

    auto presetFile = getPresetFile(name);
    if (!fileSystem.fileExists(presetFile))
        return false;

    // Try to parse the JSON to validate it
    juce::String jsonData = fileSystem.readFile(presetFile);
    if (jsonData.isEmpty())
        return false;

    auto jsonVar = juce::JSON::parse(jsonData);
    return jsonVar.isObject();
}

/**
 * @brief Gets the creation timestamp of a preset.
 *
 * @param name The name of the preset
 * @return The timestamp as a 64-bit integer, or 0 if preset doesn't exist
 */
juce::int64 PresetManager::getPresetTimestamp(const juce::String &name) const
{
    if (name.isEmpty())
        return 0;

    auto presetFile = getPresetFile(name);
    if (!fileSystem.fileExists(presetFile))
        return 0;

    // Try to get timestamp from file modification time
    juce::int64 fileTime = fileSystem.getFileTime(presetFile).toMilliseconds();

    // If that fails, try to get it from the JSON
    juce::String jsonData = fileSystem.readFile(presetFile);
    if (!jsonData.isEmpty())
    {
        auto jsonVar = juce::JSON::parse(jsonData);
        if (jsonVar.isObject())
        {
            auto jsonObj = jsonVar.getDynamicObject();
            if (jsonObj != nullptr)
            {
                auto timestampVar = jsonObj->getProperty("timestamp");
                if (timestampVar.isInt64())
                {
                    return timestampVar;
                }
            }
        }
    }

    return fileTime;
}

/**
 * @brief Gets the display name for a preset (formatted with timestamp).
 *
 * @param name The name of the preset
 * @return The formatted display name
 */
juce::String PresetManager::getPresetDisplayName(const juce::String &name) const
{
    if (name.isEmpty())
        return "";

    juce::int64 timestamp = getPresetTimestamp(name);
    if (timestamp == 0)
        return name;

    juce::Time time(timestamp);
    juce::String dateStr = time.toString(true, true, false, true);

    return name + " (" + dateStr + ")";
}

/**
 * @brief Gets the display name for a preset (without timestamp).
 *
 * @param name The name of the preset
 * @return The display name without timestamp
 */
juce::String PresetManager::getPresetDisplayNameNoTimestamp(const juce::String &name) const
{
    if (name.isEmpty())
        return "";

    return name;
}

/**
 * @brief Gets the last error message.
 *
 * @return The last error message, or empty string if no error
 */
juce::String PresetManager::getLastErrorMessage() const
{
    return lastErrorMessage;
}

/**
 * @brief Clears the last error message.
 */
void PresetManager::clearLastError()
{
    lastErrorMessage.clear();
}

/**
 * @brief Validates a preset name with detailed error reporting.
 *
 * @param name The preset name to validate
 * @param errorMessage Output parameter for detailed error message
 * @return true if the name is valid, false otherwise
 */
bool PresetManager::validatePresetName(const juce::String &name, juce::String &errorMessage) const
{
    errorMessage.clear();

    // Check for empty or whitespace-only names
    if (name.trim().isEmpty())
    {
        errorMessage = "Preset name cannot be empty or contain only whitespace.";
        return false;
    }

    // Check for invalid characters
    if (name.containsAnyOf("<>:\"/\\|?*"))
    {
        errorMessage = "Preset name contains invalid characters. The following characters are not allowed: < > : \" / \\ | ? *";
        return false;
    }

    // Check for reserved names
    juce::String lowerName = name.toLowerCase();
    if (lowerName == "con" || lowerName == "prn" || lowerName == "aux" ||
        lowerName == "nul" || lowerName == "com1" || lowerName == "com2" ||
        lowerName == "com3" || lowerName == "com4" || lowerName == "com5" ||
        lowerName == "com6" || lowerName == "com7" || lowerName == "com8" ||
        lowerName == "com9" || lowerName == "lpt1" || lowerName == "lpt2" ||
        lowerName == "lpt3" || lowerName == "lpt4" || lowerName == "lpt5" ||
        lowerName == "lpt6" || lowerName == "lpt7" || lowerName == "lpt8" ||
        lowerName == "lpt9")
    {
        errorMessage = "Preset name is a reserved system name and cannot be used.";
        return false;
    }

    // Check for names that start or end with dots or spaces
    if (name.startsWith(".") || name.endsWith(".") ||
        name.startsWith(" ") || name.endsWith(" "))
    {
        errorMessage = "Preset name cannot start or end with dots or spaces.";
        return false;
    }

    // Check for names that are too long (considering filename conversion)
    if (name.length() > 200)
    {
        errorMessage = "Preset name is too long. Maximum length is 200 characters.";
        return false;
    }

    return true;
}

/**
 * @brief Validates a preset file for corruption or format issues.
 *
 * @param name The preset name to validate
 * @param errorMessage Output parameter for detailed error message
 * @return true if the preset is valid, false otherwise
 */
bool PresetManager::validatePresetFile(const juce::String &name, juce::String &errorMessage) const
{
    errorMessage.clear();

    if (name.isEmpty())
    {
        errorMessage = "Preset name is empty.";
        return false;
    }

    auto presetFile = getPresetFile(name);

    if (!fileSystem.fileExists(presetFile))
    {
        errorMessage = "Preset file does not exist.";
        return false;
    }

    if (fileSystem.getFileSize(presetFile) == 0)
    {
        errorMessage = "Preset file is empty.";
        return false;
    }

    // Try to read and parse the JSON
    juce::String jsonData = fileSystem.readFile(presetFile);
    if (jsonData.isEmpty())
    {
        errorMessage = "Failed to read preset file.";
        return false;
    }

    auto jsonVar = juce::JSON::parse(jsonData);
    if (!jsonVar.isObject())
    {
        errorMessage = "Preset file contains invalid JSON format.";
        return false;
    }

    auto jsonObj = jsonVar.getDynamicObject();
    if (jsonObj == nullptr)
    {
        errorMessage = "Preset file contains invalid JSON structure.";
        return false;
    }

    // Check for required fields
    if (!jsonObj->hasProperty("timestamp"))
    {
        errorMessage = "Preset file is missing timestamp field.";
        return false;
    }

    if (!jsonObj->hasProperty("slots"))
    {
        errorMessage = "Preset file is missing slots field.";
        return false;
    }

    auto slotsVar = jsonObj->getProperty("slots");
    if (!slotsVar.isArray())
    {
        errorMessage = "Preset file contains invalid slots data.";
        return false;
    }

    return true;
}

/**
 * @brief Checks if a preset name conflicts with existing presets.
 *
 * @param name The preset name to check
 * @param errorMessage Output parameter for detailed error message
 * @return true if the name conflicts, false otherwise
 */
bool PresetManager::checkPresetNameConflict(const juce::String &name, juce::String &errorMessage) const
{
    errorMessage.clear();

    if (name.isEmpty())
    {
        errorMessage = "Preset name is empty.";
        return false;
    }

    auto presetNames = getPresetNames();

    for (const auto &existingName : presetNames)
    {
        if (existingName.equalsIgnoreCase(name))
        {
            errorMessage = "A preset with the name '" + existingName + "' already exists. Please choose a different name.";
            return true;
        }
    }

    return false;
}

/**
 * @brief Gets detailed information about a preset.
 *
 * @param name The preset name
 * @param errorMessage Output parameter for error message if preset doesn't exist
 * @return A JSON object with preset details, or empty var if preset doesn't exist
 */
juce::var PresetManager::getPresetInfo(const juce::String &name, juce::String &errorMessage) const
{
    errorMessage.clear();

    if (name.isEmpty())
    {
        errorMessage = "Preset name is empty.";
        return juce::var();
    }

    auto presetFile = getPresetFile(name);

    if (!fileSystem.fileExists(presetFile))
    {
        errorMessage = "Preset file does not exist.";
        return juce::var();
    }

    juce::String jsonData = fileSystem.readFile(presetFile);
    if (jsonData.isEmpty())
    {
        errorMessage = "Failed to read preset file.";
        return juce::var();
    }

    auto jsonVar = juce::JSON::parse(jsonData);
    if (!jsonVar.isObject())
    {
        errorMessage = "Preset file contains invalid JSON format.";
        return juce::var();
    }

    auto jsonObj = jsonVar.getDynamicObject();
    if (jsonObj == nullptr)
    {
        errorMessage = "Preset file contains invalid JSON structure.";
        return juce::var();
    }

    // Create info object
    juce::DynamicObject::Ptr info = new juce::DynamicObject();

    // Add basic info
    info->setProperty("name", name);
    info->setProperty("filename", fileSystem.getFileName(presetFile));
    info->setProperty("fileSize", fileSystem.getFileSize(presetFile));
    info->setProperty("lastModified", fileSystem.getFileTime(presetFile).toMilliseconds());

    // Add timestamp if available
    if (jsonObj->hasProperty("timestamp"))
    {
        info->setProperty("timestamp", jsonObj->getProperty("timestamp"));
    }

    // Count slots and gear items
    int slotCount = 0;
    int gearItemCount = 0;

    if (jsonObj->hasProperty("slots") && jsonObj->getProperty("slots").isArray())
    {
        auto slotsArray = jsonObj->getProperty("slots").getArray();
        slotCount = slotsArray->size();

        for (const auto &slotVar : *slotsArray)
        {
            if (slotVar.isObject())
            {
                auto slotObj = slotVar.getDynamicObject();
                if (slotObj != nullptr && slotObj->hasProperty("unitId"))
                {
                    gearItemCount++;
                }
            }
        }
    }

    info->setProperty("slotCount", slotCount);
    info->setProperty("gearItemCount", gearItemCount);

    return juce::var(info);
}