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

/**
 * @brief Gets the singleton instance of PresetManager.
 *
 * @return Reference to the PresetManager instance
 */
PresetManager &PresetManager::getInstance()
{
    static PresetManager instance;
    return instance;
}

/**
 * @brief Gets the presets directory.
 *
 * @return The presets directory as a JUCE File object
 */
juce::File PresetManager::getPresetsDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AnalogiqCache")
        .getChildFile("presets");
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
    auto presetsDir = getPresetsDirectory();
    return presetsDir.createDirectory();
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
juce::File PresetManager::getPresetFile(const juce::String &name) const
{
    return getPresetsDirectory().getChildFile(nameToFilename(name));
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
                            GearItem *newItem = new GearItem(*sourceItem, INetworkFetcher::getDummy());

                            // Set it in the slot
                            if (auto *slot = rack->getSlot(slotIndex))
                            {
                                slot->setGearItem(newItem);

                                // Load control values
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
                                                int controlIndex = controlObj->getProperty("index");
                                                if (controlIndex >= 0 && controlIndex < newItem->controls.size())
                                                {
                                                    auto &control = newItem->controls.getReference(controlIndex);
                                                    control.value = controlObj->getProperty("value");
                                                    control.initialValue = controlObj->getProperty("initialValue");

                                                    if (control.type == GearControl::Type::Switch)
                                                    {
                                                        control.currentIndex = controlObj->getProperty("currentIndex");
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
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
    if (!isValidPresetName(name) || rack == nullptr)
        return false;

    // Initialize presets directory
    if (!initializePresetsDirectory())
        return false;

    // Serialize rack to JSON
    juce::String jsonData = serializeRackToJSON(rack);
    if (jsonData.isEmpty())
        return false;

    // Save to file
    auto presetFile = getPresetFile(name);
    return presetFile.replaceWithText(jsonData);
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
    if (name.isEmpty() || rack == nullptr || gearLibrary == nullptr)
        return false;

    auto presetFile = getPresetFile(name);
    if (!presetFile.existsAsFile())
        return false;

    // Load JSON data
    juce::String jsonData = presetFile.loadFileAsString();
    if (jsonData.isEmpty())
        return false;

    // Deserialize to rack
    return deserializeJSONToRack(jsonData, rack, gearLibrary);
}

/**
 * @brief Deletes a preset file.
 *
 * @param name The name of the preset to delete
 * @return true if the preset was deleted successfully, false otherwise
 */
bool PresetManager::deletePreset(const juce::String &name)
{
    if (name.isEmpty())
        return false;

    auto presetFile = getPresetFile(name);
    if (!presetFile.existsAsFile())
        return false;

    return presetFile.deleteFile();
}

/**
 * @brief Gets a list of all available preset names.
 *
 * @return Array of preset names
 */
juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    auto presetsDir = getPresetsDirectory();

    if (presetsDir.exists())
    {
        auto files = presetsDir.findChildFiles(juce::File::findFiles, false, "*.json");
        for (auto &file : files)
        {
            names.add(filenameToName(file.getFileName()));
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
    if (!presetFile.existsAsFile())
        return false;

    // Try to parse the JSON to validate it
    juce::String jsonData = presetFile.loadFileAsString();
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
    if (!presetFile.existsAsFile())
        return 0;

    // Try to get timestamp from file modification time
    juce::int64 fileTime = presetFile.getLastModificationTime().toMilliseconds();

    // If that fails, try to get it from the JSON
    juce::String jsonData = presetFile.loadFileAsString();
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