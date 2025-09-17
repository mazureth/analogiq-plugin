/**
 * @file RackModel.h
 * @brief Header file for the RackModel class which manages rack data in the Model layer.
 *
 * This file defines the RackModel class which provides the data storage and management
 * for the virtual rack system. It stores all rack state independently of the UI,
 * enabling proper state persistence when the UI is closed.
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Shared/GearControl.h"
#include "../Shared/IRackStateListener.h"
#include <memory>
#include <vector>

// Forward declarations
class GearItem;
class GearLibrary;
class PresetManager;
class ICacheManager;

/**
 * @struct SlotData
 * @brief Represents the data for a single slot in the rack.
 *
 * This structure contains all the data needed to represent a slot in the rack,
 * including the gear item instance and its control values.
 */
struct SlotData
{
    // Basic slot information
    int slotIndex = -1;
    bool isOccupied = false;

    // Gear item data
    juce::String gearId;
    juce::String instanceId;
    juce::String gearName;
    juce::String manufacturer;
    juce::String version;
    juce::String description;

    // Control values - this is the key data that needs to persist
    juce::Array<GearControl> controls;

    // Faceplate and image data
    juce::String faceplateImagePath;
    juce::String thumbnailImagePath;

    // Image dimensions for dynamic height calculation
    int faceplateImageWidth = 0;
    int faceplateImageHeight = 0;

    // Metadata
    juce::StringArray tags;
    juce::String category;

    // Default constructor
    SlotData() = default;

    // Constructor with slot index
    SlotData(int index) : slotIndex(index) {}

    // Copy constructor
    SlotData(const SlotData &other) = default;

    // Assignment operator
    SlotData &operator=(const SlotData &other) = default;

    // Check if slot is empty
    bool isEmpty() const { return !isOccupied || gearId.isEmpty(); }

    // Clear slot data
    void clear()
    {
        isOccupied = false;
        gearId.clear();
        instanceId.clear();
        gearName.clear();
        manufacturer.clear();
        version.clear();
        description.clear();
        controls.clear();
        faceplateImagePath.clear();
        thumbnailImagePath.clear();
        tags.clear();
        category.clear();
    }
};

/**
 * @class RackModel
 * @brief Manages all rack data in the Model layer.
 *
 * The RackModel class provides the single source of truth for all rack data,
 * including slot information, gear instances, and control values. This enables
 * proper state persistence regardless of UI state.
 */
class RackModel
{
public:
    // Constructor
    RackModel(GearLibrary &gearLibrary, PresetManager &presetManager, ICacheManager &cacheManager);

    // Destructor
    ~RackModel();

    // Slot management
    int getSlotCount() const;
    bool isValidSlotIndex(int slotIndex) const;
    bool isSlotOccupied(int slotIndex) const;
    int getFirstEmptySlot() const;
    int getLastOccupiedSlot() const;
    void compactSlots();

    // Gear management
    bool addGearToSlot(int slotIndex, const juce::String &gearId);
    bool removeGearFromSlot(int slotIndex);
    bool moveGearBetweenSlots(int fromSlot, int toSlot);
    juce::String getGearInSlot(int slotIndex) const;
    GearLibrary &getGearLibrary() const;

    // Slot data access
    const SlotData *getSlotData(int slotIndex) const;
    SlotData *getSlotData(int slotIndex);
    void setSlotData(int slotIndex, const SlotData &data);

    // Control management
    bool updateControlValue(int slotIndex, int controlIndex, float value);
    bool updateControlIndex(int slotIndex, int controlIndex, int index);
    GearControl *getControl(int slotIndex, int controlIndex);
    const GearControl *getControl(int slotIndex, int controlIndex) const;

    // Rack configuration
    void setSlotLayout(int numSlots);
    int getSlotLayout() const;
    void setSlotSize(int width, int height);
    int getSlotWidth() const;
    int getSlotHeight() const;
    void setSlotSpacing(int spacing);
    int getSlotSpacing() const;

    // Dynamic slot sizing (matching original Rack behavior)
    int getSlotHeight(int slotIndex) const;
    int getDefaultSlotHeight() const;
    int getEffectiveSlotWidth(int rackWidth) const;

    // State persistence
    juce::ValueTree serializeToValueTree() const;
    bool deserializeFromValueTree(const juce::ValueTree &state);
    juce::String serializeToJSON() const;
    bool deserializeFromJSON(const juce::String &jsonString);

    // State management
    void clearAllSlots();
    void resetToDefault();
    bool validateState() const;
    void recoverFromInvalidState();

    // Event handling
    void addRackStateListener(RackStateListener *listener);
    void removeRackStateListener(RackStateListener *listener);

    // Preset notifications
    void notifyPresetLoaded(const juce::String &presetName);
    void notifyPresetSaved(const juce::String &presetName);

    // Utility methods
    juce::String getRackNotes() const;
    void setRackNotes(const juce::String &notes);

    // Debug and validation
    void logState() const;
    bool validateSlotConsistency() const;
    bool validateGearSlotRelationships() const;

private:
    // Dependencies
    GearLibrary &gearLibrary;
    PresetManager &presetManager;
    ICacheManager &cacheManager;

    // Rack data storage
    juce::Array<SlotData> slots;
    juce::String rackNotes;

    // Rack configuration
    int numSlots;
    int slotWidth;
    int slotHeight;
    int slotSpacing;

    // State listeners
    juce::Array<RackStateListener *> stateListeners;

    // Private helper methods
    void initializeDefaults();
    void ensureSlotCapacity(int slotIndex);
    void notifyStateChanged();
    void notifyGearItemAdded(int slotIndex, const GearItem *gearItem);
    void notifyGearItemRemoved(int slotIndex);
    void notifyGearControlChanged(int slotIndex, const GearItem *gearItem, int controlIndex);
    void notifyGearItemsRearranged(int sourceSlotIndex, int targetSlotIndex);
    void notifyRackStateReset();

    // Slot data management
    SlotData createSlotDataFromGearItem(int slotIndex, const GearItem *gearItem) const;
    void updateSlotDataFromGearItem(SlotData &slotData, const GearItem *gearItem) const;
    GearItem *createGearItemFromSlotData(const SlotData &slotData) const;

    // Validation helpers
    bool validateSlotIndex(int slotIndex) const;
    bool validateSlotData(const SlotData &data) const;

    // Serialization helpers
    juce::ValueTree serializeSlotData(const SlotData &data) const;
    bool deserializeSlotData(const juce::ValueTree &slotTree, SlotData &data);
    juce::DynamicObject *createSlotObject(const SlotData &data) const;
    bool loadSlotFromObject(const juce::DynamicObject &slotObject, SlotData &data);

    // Control serialization
    juce::ValueTree serializeControl(const GearControl &control) const;
    bool deserializeControl(const juce::ValueTree &controlTree, GearControl &control);

    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackModel)
};