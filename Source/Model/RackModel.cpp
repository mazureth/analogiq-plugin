/**
 * @file RackModel.cpp
 * @brief Implementation of the RackModel class.
 *
 * This file implements the RackModel class which provides the data storage and
 * management for the virtual rack system. It stores all rack state independently
 * of the UI, enabling proper state persistence when the UI is closed.
 */

#include "RackModel.h"
#include "GearItem.h"
#include "GearLibrary.h"
#include "PresetManager.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/IRackStateListener.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <algorithm>

RackModel::RackModel(GearLibrary &gearLib, PresetManager &presetMgr, ICacheManager &cacheMgr)
    : gearLibrary(gearLib), presetManager(presetMgr), cacheManager(cacheMgr),
      numSlots(0), slotWidth(200), slotHeight(150), slotSpacing(10)
{
    initializeDefaults();
}

RackModel::~RackModel()
{
    // Remove all state listeners
    stateListeners.clear();

    // Clear all slots
    clearAllSlots();
}

void RackModel::initializeDefaults()
{
    // Initialize with default configuration
    numSlots = 16;
    slotWidth = 200;
    slotHeight = 150;
    slotSpacing = 10;
    rackNotes = "";

    // Initialize slots array
    slots.clear();
    slots.resize(numSlots);
    for (int i = 0; i < numSlots; ++i)
    {
        slots[i] = SlotData(i);
    }
}

// Slot management
int RackModel::getSlotCount() const
{
    return slots.size();
}

bool RackModel::isValidSlotIndex(int slotIndex) const
{
    return slotIndex >= 0 && slotIndex < slots.size();
}

bool RackModel::isSlotOccupied(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    return slots[slotIndex].isOccupied && !slots[slotIndex].gearId.isEmpty();
}

int RackModel::getFirstEmptySlot() const
{
    for (int i = 0; i < slots.size(); ++i)
    {
        if (!isSlotOccupied(i))
            return i;
    }
    return -1;
}

int RackModel::getLastOccupiedSlot() const
{
    for (int i = slots.size() - 1; i >= 0; --i)
    {
        if (isSlotOccupied(i))
            return i;
    }
    return -1;
}

void RackModel::compactSlots()
{
    // Find all occupied slots
    std::vector<int> occupiedSlots;
    for (int i = 0; i < slots.size(); ++i)
    {
        if (isSlotOccupied(i))
        {
            occupiedSlots.push_back(i);
        }
    }

    // Sort occupied slots by index
    std::sort(occupiedSlots.begin(), occupiedSlots.end());

    // Move gear to consecutive slots starting from 0
    for (size_t i = 0; i < occupiedSlots.size(); ++i)
    {
        int currentSlot = occupiedSlots[i];
        int targetSlot = static_cast<int>(i);

        if (currentSlot != targetSlot)
        {
            moveGearBetweenSlots(currentSlot, targetSlot);
        }
    }
}

// Gear management
bool RackModel::addGearToSlot(int slotIndex, const juce::String &gearId)
{
    // Validate slot index
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("RackModel::addGearToSlot - Invalid slot index: " + juce::String(slotIndex));
        return false;
    }

    // Get gear item template from library
    auto gearItemTemplate = gearLibrary.getGearItem(gearId);
    if (!gearItemTemplate)
    {
        juce::Logger::writeToLog("RackModel::addGearToSlot - Gear item not found: " + gearId);
        return false;
    }

    // Create a unique instance for this rack slot
    auto gearItem = std::make_unique<GearItem>(gearItemTemplate->createInstance());

    // Create slot data from gear item
    SlotData slotData = createSlotDataFromGearItem(slotIndex, gearItem.get());

    // Set the slot data
    slots[slotIndex] = slotData;

    // Notify listeners
    notifyGearItemAdded(slotIndex, gearItem.get());
    notifyStateChanged();

    // Add to recently used
    cacheManager.addToRecentlyUsed(gearId);

    juce::Logger::writeToLog("RackModel::addGearToSlot - Added gear " + gearId + " to slot " + juce::String(slotIndex));
    return true;
}

bool RackModel::removeGearFromSlot(int slotIndex)
{
    // Validate slot index
    if (!isValidSlotIndex(slotIndex))
    {
        juce::Logger::writeToLog("RackModel::removeGearFromSlot - Invalid slot index: " + juce::String(slotIndex));
        return false;
    }

    // Clear the slot data
    slots[slotIndex].clear();

    // Notify listeners
    notifyGearItemRemoved(slotIndex);
    notifyStateChanged();

    juce::Logger::writeToLog("RackModel::removeGearFromSlot - Removed gear from slot " + juce::String(slotIndex));
    return true;
}

bool RackModel::moveGearBetweenSlots(int fromSlot, int toSlot)
{
    // Validate slot indices
    if (!isValidSlotIndex(fromSlot) || !isValidSlotIndex(toSlot))
    {
        juce::Logger::writeToLog("RackModel::moveGearBetweenSlots - Invalid slot indices: " +
                                 juce::String(fromSlot) + " -> " + juce::String(toSlot));
        return false;
    }

    if (fromSlot == toSlot)
        return true;

    // Get the slot data
    SlotData fromData = slots[fromSlot];
    SlotData toData = slots[toSlot];

    // If source slot is empty, nothing to move
    if (!fromData.isOccupied)
        return false;

    // If destination slot is empty, just move the gear
    if (!toData.isOccupied)
    {
        // Move gear from source to destination
        fromData.slotIndex = toSlot;
        slots.set(toSlot, fromData);
        slots.getReference(fromSlot).clear();

        // Notify listeners
        notifyGearItemsRearranged(fromSlot, toSlot);
        notifyStateChanged();

        return true;
    }
    else
    {
        // Both slots have gear - swap them
        fromData.slotIndex = toSlot;
        toData.slotIndex = fromSlot;
        slots.set(toSlot, fromData);
        slots.set(fromSlot, toData);

        // Notify listeners
        notifyGearItemsRearranged(fromSlot, toSlot);
        notifyStateChanged();

        return true;
    }
}

juce::String RackModel::getGearInSlot(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return "";

    return slots[slotIndex].gearId;
}

// Slot data access
const SlotData *RackModel::getSlotData(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return nullptr;

    return &slots.getReference(slotIndex);
}

SlotData *RackModel::getSlotData(int slotIndex)
{
    if (!isValidSlotIndex(slotIndex))
        return nullptr;

    return &slots.getReference(slotIndex);
}

void RackModel::setSlotData(int slotIndex, const SlotData &data)
{
    if (!isValidSlotIndex(slotIndex))
        return;

    SlotData slotData = data;
    slotData.slotIndex = slotIndex;
    slots.set(slotIndex, slotData);

    notifyStateChanged();
}

// Control management
bool RackModel::updateControlValue(int slotIndex, int controlIndex, float value)
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    SlotData &slotData = slots.getReference(slotIndex);
    if (!slotData.isOccupied || controlIndex < 0 || controlIndex >= slotData.controls.size())
        return false;

    // Update the control value
    slotData.controls.getReference(controlIndex).currentValue = value;

    // Notify listeners
    notifyGearControlChanged(slotIndex, nullptr, controlIndex);
    notifyStateChanged();

    return true;
}

bool RackModel::updateControlIndex(int slotIndex, int controlIndex, int index)
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    SlotData &slotData = slots.getReference(slotIndex);
    if (!slotData.isOccupied || controlIndex < 0 || controlIndex >= slotData.controls.size())
        return false;

    // Update the control index
    slotData.controls.getReference(controlIndex).currentIndex = index;
    slotData.controls.getReference(controlIndex).currentValue = static_cast<float>(index);

    // Notify listeners
    notifyGearControlChanged(slotIndex, nullptr, controlIndex);
    notifyStateChanged();

    return true;
}

GearControl *RackModel::getControl(int slotIndex, int controlIndex)
{
    if (!isValidSlotIndex(slotIndex))
        return nullptr;

    SlotData &slotData = slots.getReference(slotIndex);
    if (!slotData.isOccupied || controlIndex < 0 || controlIndex >= slotData.controls.size())
        return nullptr;

    return &slotData.controls.getReference(controlIndex);
}

const GearControl *RackModel::getControl(int slotIndex, int controlIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return nullptr;

    const SlotData &slotData = slots[slotIndex];
    if (!slotData.isOccupied || controlIndex < 0 || controlIndex >= slotData.controls.size())
        return nullptr;

    return &slotData.controls.getReference(controlIndex);
}

// Rack configuration
void RackModel::setSlotLayout(int newNumSlots)
{
    if (numSlots != newNumSlots)
    {
        numSlots = newNumSlots;

        // Resize slots array
        int oldSize = slots.size();
        slots.resize(numSlots);

        // Initialize new slots
        for (int i = oldSize; i < numSlots; ++i)
        {
            slots[i] = SlotData(i);
        }

        notifyStateChanged();
    }
}

int RackModel::getSlotLayout() const
{
    return numSlots;
}

void RackModel::setSlotSize(int width, int height)
{
    if (slotWidth != width || slotHeight != height)
    {
        slotWidth = width;
        slotHeight = height;
        notifyStateChanged();
    }
}

int RackModel::getSlotWidth() const
{
    return slotWidth;
}

int RackModel::getSlotHeight() const
{
    return slotHeight;
}

int RackModel::getSlotHeight(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return getDefaultSlotHeight();

    const SlotData &slotData = slots[slotIndex];
    if (!slotData.isOccupied)
        return getDefaultSlotHeight();

    // If the slot has faceplate image dimensions, calculate height based on aspect ratio
    if (slotData.faceplateImageWidth > 0 && slotData.faceplateImageHeight > 0)
    {
        // Calculate what the height would be if the width matched the effective slot width
        // This matches the original Rack::getSlotHeight logic
        int effectiveSlotWidth = 800; // Default rack width for calculation
        int scaledHeight = (slotData.faceplateImageHeight * effectiveSlotWidth) / slotData.faceplateImageWidth;

        // Add padding for slot UI elements (buttons, labels, etc.)
        int paddedHeight = scaledHeight + 40; // 20px padding top and bottom

        // Constrain to reasonable bounds (matching original implementation)
        return juce::jlimit(100, 400, paddedHeight);
    }

    // Default height if no special considerations apply
    return getDefaultSlotHeight();
}

int RackModel::getDefaultSlotHeight() const
{
    return 150; // Default height if not overridden
}

int RackModel::getEffectiveSlotWidth(int rackWidth) const
{
    // Calculate the effective slot width based on rack width minus margins
    // This matches the original Rack::updateSlotPositions logic
    return rackWidth - (2 * slotSpacing);
}

void RackModel::setSlotSpacing(int spacing)
{
    if (slotSpacing != spacing)
    {
        slotSpacing = spacing;
        notifyStateChanged();
    }
}

int RackModel::getSlotSpacing() const
{
    return slotSpacing;
}

// State persistence
juce::ValueTree RackModel::serializeToValueTree() const
{
    juce::ValueTree rackTree("Rack");

    // Save rack configuration
    rackTree.setProperty("numSlots", numSlots, nullptr);
    rackTree.setProperty("slotWidth", slotWidth, nullptr);
    rackTree.setProperty("slotHeight", slotHeight, nullptr);
    rackTree.setProperty("slotSpacing", slotSpacing, nullptr);
    rackTree.setProperty("rackNotes", rackNotes, nullptr);

    // Save slot data
    juce::ValueTree slotsTree("Slots");
    for (int i = 0; i < slots.size(); ++i)
    {
        if (slots[i].isOccupied)
        {
            juce::ValueTree slotTree = serializeSlotData(slots[i]);
            slotsTree.appendChild(slotTree, nullptr);
        }
    }
    rackTree.appendChild(slotsTree, nullptr);

    return rackTree;
}

bool RackModel::deserializeFromValueTree(const juce::ValueTree &state)
{
    if (!state.isValid() || state.getType() != juce::Identifier("Rack"))
        return false;

    // Load rack configuration
    numSlots = state.getProperty("numSlots", 16);
    slotWidth = state.getProperty("slotWidth", 200);
    slotHeight = state.getProperty("slotHeight", 150);
    slotSpacing = state.getProperty("slotSpacing", 10);
    rackNotes = state.getProperty("rackNotes", "");

    // Clear existing slots
    clearAllSlots();

    // Load slot data
    juce::ValueTree slotsTree = state.getChildWithName("Slots");
    if (slotsTree.isValid())
    {
        for (int i = 0; i < slotsTree.getNumChildren(); ++i)
        {
            juce::ValueTree slotTree = slotsTree.getChild(i);
            SlotData slotData;
            if (deserializeSlotData(slotTree, slotData))
            {
                // Ensure we have enough slots
                ensureSlotCapacity(slotData.slotIndex);
                slots[slotData.slotIndex] = slotData;
            }
        }
    }

    notifyStateChanged();
    return true;
}

juce::String RackModel::serializeToJSON() const
{
    juce::DynamicObject::Ptr rackObject = new juce::DynamicObject();

    // Add rack metadata
    rackObject->setProperty("version", "1.0");
    rackObject->setProperty("numSlots", numSlots);
    rackObject->setProperty("slotWidth", slotWidth);
    rackObject->setProperty("slotHeight", slotHeight);
    rackObject->setProperty("slotSpacing", slotSpacing);
    rackObject->setProperty("rackNotes", rackNotes);

    // Create array of slots
    juce::Array<juce::var> slotsArray;

    for (int i = 0; i < slots.size(); ++i)
    {
        if (slots[i].isOccupied)
        {
            juce::DynamicObject::Ptr slotObject = createSlotObject(slots[i]);
            if (slotObject)
            {
                slotsArray.add(slotObject.get());
            }
        }
    }

    rackObject->setProperty("slots", slotsArray);

    // Convert to JSON string
    return juce::JSON::toString(juce::var(rackObject.get()));
}

bool RackModel::deserializeFromJSON(const juce::String &jsonString)
{
    try
    {
        // Parse JSON
        juce::var parsedJson = juce::JSON::parse(jsonString);
        if (!parsedJson.isObject())
        {
            juce::Logger::writeToLog("RackModel::deserializeFromJSON - JSON is not an object");
            return false;
        }

        juce::DynamicObject *rackObject = parsedJson.getDynamicObject();
        if (!rackObject)
            return false;

        // Clear existing rack state
        clearAllSlots();

        // Load rack configuration
        if (rackObject->hasProperty("numSlots"))
        {
            int newNumSlots = rackObject->getProperty("numSlots");
            setSlotLayout(newNumSlots);
        }

        if (rackObject->hasProperty("slotWidth"))
            slotWidth = rackObject->getProperty("slotWidth");
        if (rackObject->hasProperty("slotHeight"))
            slotHeight = rackObject->getProperty("slotHeight");
        if (rackObject->hasProperty("slotSpacing"))
            slotSpacing = rackObject->getProperty("slotSpacing");
        if (rackObject->hasProperty("rackNotes"))
            rackNotes = rackObject->getProperty("rackNotes");

        // Load slots
        if (rackObject->hasProperty("slots"))
        {
            juce::var slotsVar = rackObject->getProperty("slots");
            if (slotsVar.isArray())
            {
                juce::Array<juce::var> *slotsArray = slotsVar.getArray();

                for (int i = 0; i < slotsArray->size(); ++i)
                {
                    juce::var slotVar = slotsArray->getReference(i);
                    if (slotVar.isObject())
                    {
                        juce::DynamicObject *slotObject = slotVar.getDynamicObject();
                        if (slotObject)
                        {
                            SlotData slotData;
                            if (loadSlotFromObject(*slotObject, slotData))
                            {
                                // Ensure we have enough slots
                                ensureSlotCapacity(slotData.slotIndex);
                                slots[slotData.slotIndex] = slotData;
                            }
                        }
                    }
                }
            }
        }

        notifyStateChanged();
        return true;
    }
    catch (const std::exception &e)
    {
        juce::Logger::writeToLog("RackModel::deserializeFromJSON - Error: " + juce::String(e.what()));
        return false;
    }
}

// State management
void RackModel::clearAllSlots()
{
    // Clear all slot data
    for (auto &slot : slots)
    {
        slot.clear();
    }

    notifyRackStateReset();
    notifyStateChanged();
}

void RackModel::resetToDefault()
{
    clearAllSlots();
    initializeDefaults();
    notifyStateChanged();
}

bool RackModel::validateState() const
{
    // Check if rack has valid number of slots
    if (slots.size() != static_cast<size_t>(numSlots))
    {
        juce::Logger::writeToLog("RackModel validation failed: slot count mismatch");
        return false;
    }

    // Validate slot consistency
    if (!validateSlotConsistency())
    {
        return false;
    }

    // Validate gear-slot relationships
    if (!validateGearSlotRelationships())
    {
        return false;
    }

    return true;
}

void RackModel::recoverFromInvalidState()
{
    juce::Logger::writeToLog("RackModel recovery: Attempting to recover from invalid state");

    // Clear all slots and start fresh
    clearAllSlots();

    // Recreate a basic rack structure
    setSlotLayout(16); // Default to 16 slots

    juce::Logger::writeToLog("RackModel recovery: Recovered to clean state with " + juce::String(slots.size()) + " slots");
}

// Event handling
void RackModel::addRackStateListener(RackStateListener *listener)
{
    if (listener && !stateListeners.contains(listener))
    {
        stateListeners.add(listener);
    }
}

void RackModel::removeRackStateListener(RackStateListener *listener)
{
    stateListeners.removeFirstMatchingValue(listener);
}

void RackModel::notifyPresetLoaded(const juce::String &presetName)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onPresetLoaded(nullptr, presetName);
        }
    }
}

void RackModel::notifyPresetSaved(const juce::String &presetName)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onPresetSaved(nullptr, presetName);
        }
    }
}

// Utility methods
juce::String RackModel::getRackNotes() const
{
    return rackNotes;
}

void RackModel::setRackNotes(const juce::String &notes)
{
    if (rackNotes != notes)
    {
        rackNotes = notes;
        notifyStateChanged();
    }
}

// Debug and validation
void RackModel::logState() const
{
    juce::Logger::writeToLog("RackModel State:");
    juce::Logger::writeToLog("  Slots: " + juce::String(slots.size()));
    juce::Logger::writeToLog("  Width: " + juce::String(slotWidth));
    juce::Logger::writeToLog("  Height: " + juce::String(slotHeight));
    juce::Logger::writeToLog("  Spacing: " + juce::String(slotSpacing));

    for (int i = 0; i < slots.size(); ++i)
    {
        if (slots[i].isOccupied)
        {
            juce::Logger::writeToLog("  Slot " + juce::String(i) + ": " + slots[i].gearId + " (" + slots[i].gearName + ")");
        }
    }
}

bool RackModel::validateSlotConsistency() const
{
    // Check that all slots exist and have valid indices
    for (int i = 0; i < slots.size(); ++i)
    {
        if (slots[i].slotIndex != i)
        {
            juce::Logger::writeToLog("RackModel validation failed: slot index mismatch at position " + juce::String(i));
            return false;
        }
    }

    return true;
}

bool RackModel::validateGearSlotRelationships() const
{
    // Check that gear items are properly associated with slots
    for (int i = 0; i < slots.size(); ++i)
    {
        if (slots[i].isOccupied)
        {
            if (slots[i].gearId.isEmpty())
            {
                juce::Logger::writeToLog("RackModel validation failed: occupied slot " + juce::String(i) + " has empty gearId");
                return false;
            }
        }
    }

    return true;
}

// Private helper methods
void RackModel::ensureSlotCapacity(int slotIndex)
{
    if (slotIndex >= slots.size())
    {
        slots.resize(slotIndex + 1);
        for (int i = static_cast<int>(slots.size()) - (slotIndex + 1); i < slots.size(); ++i)
        {
            if (slots[i].slotIndex == -1)
            {
                slots[i] = SlotData(i);
            }
        }
    }
}

void RackModel::notifyStateChanged()
{
    // Notify all listeners that rack state has changed
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            // IRackStateListener doesn't inherit from Component, so we can't call repaint()
            // The specific notification methods handle UI updates
        }
    }
}

void RackModel::notifyGearItemAdded(int slotIndex, const GearItem *gearItem)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemAdded(nullptr, slotIndex, gearItem);
        }
    }
}

void RackModel::notifyGearItemRemoved(int slotIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemRemoved(nullptr, slotIndex);
        }
    }
}

void RackModel::notifyGearControlChanged(int slotIndex, const GearItem *gearItem, int controlIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearControlChanged(nullptr, slotIndex, gearItem, controlIndex);
        }
    }
}

void RackModel::notifyGearItemsRearranged(int sourceSlotIndex, int targetSlotIndex)
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onGearItemsRearranged(nullptr, sourceSlotIndex, targetSlotIndex);
        }
    }
}

void RackModel::notifyRackStateReset()
{
    // Notify all RackStateListener implementations
    for (auto *listener : stateListeners)
    {
        if (listener)
        {
            listener->onRackStateReset(nullptr);
        }
    }
}

// Slot data management
SlotData RackModel::createSlotDataFromGearItem(int slotIndex, const GearItem *gearItem) const
{
    SlotData slotData(slotIndex);

    if (gearItem)
    {
        slotData.isOccupied = true;
        slotData.gearId = gearItem->unitId;
        slotData.instanceId = gearItem->instanceId;
        slotData.gearName = gearItem->name;
        slotData.manufacturer = gearItem->manufacturer;
        slotData.version = gearItem->version;
        slotData.description = gearItem->description;
        slotData.faceplateImagePath = gearItem->faceplateImagePath;
        slotData.thumbnailImagePath = ""; // GearItem doesn't have thumbnailImagePath
        slotData.tags = gearItem->tags;
        slotData.category = gearItem->categoryString;

        // Store faceplate image dimensions for dynamic height calculation
        if (gearItem->faceplateImage.isValid())
        {
            slotData.faceplateImageWidth = gearItem->faceplateImage.getWidth();
            slotData.faceplateImageHeight = gearItem->faceplateImage.getHeight();
        }
        else
        {
            slotData.faceplateImageWidth = 0;
            slotData.faceplateImageHeight = 0;
        }

        // Copy controls
        slotData.controls = gearItem->controls;
    }

    return slotData;
}

void RackModel::updateSlotDataFromGearItem(SlotData &slotData, const GearItem *gearItem) const
{
    if (gearItem)
    {
        slotData.gearId = gearItem->unitId;
        slotData.instanceId = gearItem->instanceId;
        slotData.gearName = gearItem->name;
        slotData.manufacturer = gearItem->manufacturer;
        slotData.version = gearItem->version;
        slotData.description = gearItem->description;
        slotData.faceplateImagePath = gearItem->faceplateImagePath;
        slotData.thumbnailImagePath = ""; // GearItem doesn't have thumbnailImagePath
        slotData.tags = gearItem->tags;
        slotData.category = gearItem->categoryString;

        // Update faceplate image dimensions for dynamic height calculation
        if (gearItem->faceplateImage.isValid())
        {
            slotData.faceplateImageWidth = gearItem->faceplateImage.getWidth();
            slotData.faceplateImageHeight = gearItem->faceplateImage.getHeight();
        }
        else
        {
            slotData.faceplateImageWidth = 0;
            slotData.faceplateImageHeight = 0;
        }

        // Update controls
        slotData.controls = gearItem->controls;
    }
}

GearItem *RackModel::createGearItemFromSlotData(const SlotData &slotData) const
{
    if (!slotData.isOccupied || slotData.gearId.isEmpty())
        return nullptr;

    // Get gear item template from library
    auto gearItemTemplate = gearLibrary.getGearItem(slotData.gearId);
    if (!gearItemTemplate)
        return nullptr;

    // Create a unique instance
    auto gearItem = std::make_unique<GearItem>(gearItemTemplate->createInstance());

    // Update with slot data
    gearItem->instanceId = slotData.instanceId;
    gearItem->controls = slotData.controls;

    return gearItem.release();
}

// Validation helpers
bool RackModel::validateSlotIndex(int slotIndex) const
{
    return slotIndex >= 0 && slotIndex < slots.size();
}

bool RackModel::validateSlotData(const SlotData &data) const
{
    if (data.isOccupied)
    {
        return !data.gearId.isEmpty() && !data.gearName.isEmpty();
    }
    return true;
}

// Serialization helpers
juce::ValueTree RackModel::serializeSlotData(const SlotData &data) const
{
    juce::ValueTree slotTree("Slot");

    slotTree.setProperty("slotIndex", data.slotIndex, nullptr);
    slotTree.setProperty("isOccupied", data.isOccupied, nullptr);

    if (data.isOccupied)
    {
        slotTree.setProperty("gearId", data.gearId, nullptr);
        slotTree.setProperty("instanceId", data.instanceId, nullptr);
        slotTree.setProperty("gearName", data.gearName, nullptr);
        slotTree.setProperty("manufacturer", data.manufacturer, nullptr);
        slotTree.setProperty("version", data.version, nullptr);
        slotTree.setProperty("description", data.description, nullptr);
        slotTree.setProperty("faceplateImagePath", data.faceplateImagePath, nullptr);
        slotTree.setProperty("thumbnailImagePath", data.thumbnailImagePath, nullptr);
        slotTree.setProperty("category", data.category, nullptr);
        slotTree.setProperty("faceplateImageWidth", data.faceplateImageWidth, nullptr);
        slotTree.setProperty("faceplateImageHeight", data.faceplateImageHeight, nullptr);

        // Serialize tags
        juce::ValueTree tagsTree("Tags");
        for (const auto &tag : data.tags)
        {
            juce::ValueTree tagTree("Tag");
            tagTree.setProperty("value", tag, nullptr);
            tagsTree.appendChild(tagTree, nullptr);
        }
        slotTree.appendChild(tagsTree, nullptr);

        // Serialize controls
        juce::ValueTree controlsTree("Controls");
        for (const auto &control : data.controls)
        {
            juce::ValueTree controlTree = serializeControl(control);
            controlsTree.appendChild(controlTree, nullptr);
        }
        slotTree.appendChild(controlsTree, nullptr);
    }

    return slotTree;
}

bool RackModel::deserializeSlotData(const juce::ValueTree &slotTree, SlotData &data)
{
    if (!slotTree.isValid() || slotTree.getType() != juce::Identifier("Slot"))
        return false;

    data.slotIndex = slotTree.getProperty("slotIndex", -1);
    data.isOccupied = slotTree.getProperty("isOccupied", false);

    if (data.isOccupied)
    {
        data.gearId = slotTree.getProperty("gearId", "");
        data.instanceId = slotTree.getProperty("instanceId", "");
        data.gearName = slotTree.getProperty("gearName", "");
        data.manufacturer = slotTree.getProperty("manufacturer", "");
        data.version = slotTree.getProperty("version", "");
        data.description = slotTree.getProperty("description", "");
        data.faceplateImagePath = slotTree.getProperty("faceplateImagePath", "");
        data.thumbnailImagePath = slotTree.getProperty("thumbnailImagePath", "");
        data.category = slotTree.getProperty("category", "");
        data.faceplateImageWidth = slotTree.getProperty("faceplateImageWidth", 0);
        data.faceplateImageHeight = slotTree.getProperty("faceplateImageHeight", 0);

        // Deserialize tags
        juce::ValueTree tagsTree = slotTree.getChildWithName("Tags");
        if (tagsTree.isValid())
        {
            data.tags.clear();
            for (int i = 0; i < tagsTree.getNumChildren(); ++i)
            {
                juce::ValueTree tagTree = tagsTree.getChild(i);
                if (tagTree.getType() == juce::Identifier("Tag"))
                {
                    data.tags.add(tagTree.getProperty("value", ""));
                }
            }
        }

        // Deserialize controls
        juce::ValueTree controlsTree = slotTree.getChildWithName("Controls");
        if (controlsTree.isValid())
        {
            data.controls.clear();
            for (int i = 0; i < controlsTree.getNumChildren(); ++i)
            {
                juce::ValueTree controlTree = controlsTree.getChild(i);
                GearControl control;
                if (deserializeControl(controlTree, control))
                {
                    data.controls.add(control);
                }
            }
        }
    }

    return true;
}

juce::DynamicObject *RackModel::createSlotObject(const SlotData &data) const
{
    juce::DynamicObject::Ptr slotObject = new juce::DynamicObject();

    slotObject->setProperty("slotIndex", data.slotIndex);
    slotObject->setProperty("isOccupied", data.isOccupied);

    if (data.isOccupied)
    {
        slotObject->setProperty("gearId", data.gearId);
        slotObject->setProperty("instanceId", data.instanceId);
        slotObject->setProperty("gearName", data.gearName);
        slotObject->setProperty("manufacturer", data.manufacturer);
        slotObject->setProperty("version", data.version);
        slotObject->setProperty("description", data.description);
        slotObject->setProperty("faceplateImagePath", data.faceplateImagePath);
        slotObject->setProperty("thumbnailImagePath", data.thumbnailImagePath);
        slotObject->setProperty("category", data.category);
        slotObject->setProperty("faceplateImageWidth", data.faceplateImageWidth);
        slotObject->setProperty("faceplateImageHeight", data.faceplateImageHeight);

        // Serialize tags
        juce::Array<juce::var> tagsArray;
        for (const auto &tag : data.tags)
        {
            tagsArray.add(tag);
        }
        slotObject->setProperty("tags", tagsArray);

        // Serialize controls
        juce::DynamicObject::Ptr controlsObject = new juce::DynamicObject();
        for (int i = 0; i < data.controls.size(); ++i)
        {
            const auto &control = data.controls[i];
            controlsObject->setProperty(control.name, control.currentValue);
        }
        slotObject->setProperty("controls", controlsObject.get());
    }

    return slotObject.get();
}

bool RackModel::loadSlotFromObject(const juce::DynamicObject &slotObject, SlotData &data)
{
    data.slotIndex = slotObject.getProperty("slotIndex").isVoid() ? -1 : (int)slotObject.getProperty("slotIndex");
    data.isOccupied = slotObject.getProperty("isOccupied").isVoid() ? false : (bool)slotObject.getProperty("isOccupied");

    if (data.isOccupied)
    {
        data.gearId = slotObject.getProperty("gearId").isVoid() ? "" : slotObject.getProperty("gearId").toString();
        data.instanceId = slotObject.getProperty("instanceId").isVoid() ? "" : slotObject.getProperty("instanceId").toString();
        data.gearName = slotObject.getProperty("gearName").isVoid() ? "" : slotObject.getProperty("gearName").toString();
        data.manufacturer = slotObject.getProperty("manufacturer").isVoid() ? "" : slotObject.getProperty("manufacturer").toString();
        data.version = slotObject.getProperty("version").isVoid() ? "" : slotObject.getProperty("version").toString();
        data.description = slotObject.getProperty("description").isVoid() ? "" : slotObject.getProperty("description").toString();
        data.faceplateImagePath = slotObject.getProperty("faceplateImagePath").isVoid() ? "" : slotObject.getProperty("faceplateImagePath").toString();
        data.thumbnailImagePath = slotObject.getProperty("thumbnailImagePath").isVoid() ? "" : slotObject.getProperty("thumbnailImagePath").toString();
        data.category = slotObject.getProperty("category").isVoid() ? "" : slotObject.getProperty("category").toString();
        data.faceplateImageWidth = slotObject.getProperty("faceplateImageWidth").isVoid() ? 0 : (int)slotObject.getProperty("faceplateImageWidth");
        data.faceplateImageHeight = slotObject.getProperty("faceplateImageHeight").isVoid() ? 0 : (int)slotObject.getProperty("faceplateImageHeight");

        // Deserialize tags
        juce::var tagsVar = slotObject.getProperty("tags");
        if (tagsVar.isArray())
        {
            juce::Array<juce::var> *tagsArray = tagsVar.getArray();
            data.tags.clear();
            for (int i = 0; i < tagsArray->size(); ++i)
            {
                data.tags.add(tagsArray->getReference(i).toString());
            }
        }

        // Deserialize controls
        juce::var controlsVar = slotObject.getProperty("controls");
        if (controlsVar.isObject())
        {
            juce::DynamicObject *controlsObject = controlsVar.getDynamicObject();
            if (controlsObject)
            {
                // Note: This is a simplified control deserialization
                // In a full implementation, you'd need to reconstruct the full GearControl objects
                // For now, we'll just store the values
                data.controls.clear();
                // TODO: Implement full control deserialization
            }
        }
    }

    return true;
}

// Control serialization
juce::ValueTree RackModel::serializeControl(const GearControl &control) const
{
    juce::ValueTree controlTree("Control");

    controlTree.setProperty("name", control.name, nullptr);
    controlTree.setProperty("type", static_cast<int>(control.type), nullptr);
    controlTree.setProperty("positionX", control.position.getX(), nullptr);
    controlTree.setProperty("positionY", control.position.getY(), nullptr);
    controlTree.setProperty("positionWidth", control.position.getWidth(), nullptr);
    controlTree.setProperty("positionHeight", control.position.getHeight(), nullptr);
    controlTree.setProperty("currentValue", control.currentValue, nullptr);
    controlTree.setProperty("initialValue", control.initialValue, nullptr);
    controlTree.setProperty("minValue", control.minValue, nullptr);
    controlTree.setProperty("maxValue", control.maxValue, nullptr);
    controlTree.setProperty("stepSize", control.stepSize, nullptr);
    controlTree.setProperty("orientation", static_cast<int>(control.orientation), nullptr);
    controlTree.setProperty("imagePath", control.imagePath, nullptr);
    controlTree.setProperty("currentIndex", control.currentIndex, nullptr);
    controlTree.setProperty("isMomentary", control.isMomentary, nullptr);
    controlTree.setProperty("length", control.length, nullptr);
    controlTree.setProperty("startAngle", control.startAngle, nullptr);
    controlTree.setProperty("endAngle", control.endAngle, nullptr);
    controlTree.setProperty("currentStepIndex", control.currentStepIndex, nullptr);

    // Serialize options
    juce::ValueTree optionsTree("Options");
    for (const auto &option : control.options)
    {
        juce::ValueTree optionTree("Option");
        optionTree.setProperty("value", option, nullptr);
        optionsTree.appendChild(optionTree, nullptr);
    }
    controlTree.appendChild(optionsTree, nullptr);

    // Serialize steps
    juce::ValueTree stepsTree("Steps");
    for (const auto &step : control.steps)
    {
        juce::ValueTree stepTree("Step");
        stepTree.setProperty("value", step, nullptr);
        stepsTree.appendChild(stepTree, nullptr);
    }
    controlTree.appendChild(stepsTree, nullptr);

    return controlTree;
}

bool RackModel::deserializeControl(const juce::ValueTree &controlTree, GearControl &control)
{
    if (!controlTree.isValid() || controlTree.getType() != juce::Identifier("Control"))
        return false;

    control.name = controlTree.getProperty("name", "");
    control.type = static_cast<GearControl::ControlType>(static_cast<int>(controlTree.getProperty("type", 0)));
    control.position = juce::Rectangle<float>(
        controlTree.getProperty("positionX", 0.0f),
        controlTree.getProperty("positionY", 0.0f),
        controlTree.getProperty("positionWidth", 0.0f),
        controlTree.getProperty("positionHeight", 0.0f));
    control.currentValue = controlTree.getProperty("currentValue", 0.0f);
    control.initialValue = controlTree.getProperty("initialValue", 0.0f);
    control.minValue = controlTree.getProperty("minValue", 0.0f);
    control.maxValue = controlTree.getProperty("maxValue", 1.0f);
    control.stepSize = controlTree.getProperty("stepSize", 0.01f);
    control.orientation = static_cast<GearControl::Orientation>(static_cast<int>(controlTree.getProperty("orientation", 0)));
    control.imagePath = controlTree.getProperty("imagePath", "");
    control.currentIndex = controlTree.getProperty("currentIndex", 0);
    control.isMomentary = controlTree.getProperty("isMomentary", false);
    control.length = controlTree.getProperty("length", 100.0f);
    control.startAngle = controlTree.getProperty("startAngle", 0.0f);
    control.endAngle = controlTree.getProperty("endAngle", 360.0f);
    control.currentStepIndex = controlTree.getProperty("currentStepIndex", 0);

    // Deserialize options
    juce::ValueTree optionsTree = controlTree.getChildWithName("Options");
    if (optionsTree.isValid())
    {
        control.options.clear();
        for (int i = 0; i < optionsTree.getNumChildren(); ++i)
        {
            juce::ValueTree optionTree = optionsTree.getChild(i);
            if (optionTree.getType() == juce::Identifier("Option"))
            {
                control.options.add(optionTree.getProperty("value", ""));
            }
        }
    }

    // Deserialize steps
    juce::ValueTree stepsTree = controlTree.getChildWithName("Steps");
    if (stepsTree.isValid())
    {
        control.steps.clear();
        for (int i = 0; i < stepsTree.getNumChildren(); ++i)
        {
            juce::ValueTree stepTree = stepsTree.getChild(i);
            if (stepTree.getType() == juce::Identifier("Step"))
            {
                control.steps.add(static_cast<float>(stepTree.getProperty("value", 0.0)));
            }
        }
    }

    return true;
}
