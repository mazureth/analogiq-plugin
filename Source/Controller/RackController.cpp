/**
 * @file RackController.cpp
 * @brief Implementation of the RackController class.
 *
 * This file implements the rack controller that coordinates between
 * the Rack model and UI components, handling rack operations and state changes.
 */

#include "RackController.h"
#include "../Model/GearItem.h"

/**
 * @brief Constructs a new RackController.
 *
 * @param rack Reference to the rack model
 * @param presetManager Reference to the preset manager
 * @param fileSystem Reference to the file system service
 * @param cacheManager Reference to the cache manager
 */
RackController::RackController(Rack &rack,
                               PresetManager &presetManager,
                               IFileSystem &fileSystem,
                               ICacheManager &cacheManager)
    : rack(rack),
      presetManager(presetManager),
      fileSystem(fileSystem),
      cacheManager(cacheManager)
{
    // Initialize the controller
    updateModifiedState();
}

// Rack Slot Operations

bool RackController::addGearToSlot(int slotIndex, GearItem *gearItem)
{
    if (!isValidSlotIndex(slotIndex) || !gearItem)
        return false;

    // Check if the slot is already occupied
    if (!isSlotEmpty(slotIndex))
        return false;

    // Add the gear to the rack slot using the gear ID
    bool success = rack.addGearToSlot(slotIndex, gearItem->unitId);

    if (success)
    {
        std::cout << "[RackController] Successfully added gear '" << gearItem->name << "' to slot " << slotIndex << std::endl;
        updateModifiedState();
        return true;
    }
    else
    {
        std::cout << "[RackController] Failed to add gear '" << gearItem->name << "' to slot " << slotIndex << std::endl;
        return false;
    }
}

bool RackController::removeGearFromSlot(int slotIndex)
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    // Check if the slot is empty
    if (isSlotEmpty(slotIndex))
        return false;

    // Remove the gear from the rack slot
    bool success = rack.removeGearFromSlot(slotIndex);

    if (success)
    {
        std::cout << "[RackController] Successfully removed gear from slot " << slotIndex << std::endl;
        updateModifiedState();
        return true;
    }
    else
    {
        std::cout << "[RackController] Failed to remove gear from slot " << slotIndex << std::endl;
        return false;
    }
}

bool RackController::moveGearBetweenSlots(int fromSlotIndex, int toSlotIndex)
{
    if (!isValidSlotIndex(fromSlotIndex) || !isValidSlotIndex(toSlotIndex))
        return false;

    if (fromSlotIndex == toSlotIndex)
        return true; // No movement needed

    // Check if source slot has gear and destination slot is empty
    if (isSlotEmpty(fromSlotIndex) || !isSlotEmpty(toSlotIndex))
        return false;

    // Move the gear between slots
    bool success = rack.moveGearBetweenSlots(fromSlotIndex, toSlotIndex);

    if (success)
    {
        std::cout << "[RackController] Successfully moved gear from slot " << fromSlotIndex << " to slot " << toSlotIndex << std::endl;
        updateModifiedState();
        return true;
    }
    else
    {
        std::cout << "[RackController] Failed to move gear from slot " << fromSlotIndex << " to slot " << toSlotIndex << std::endl;
        return false;
    }
}

bool RackController::swapGearBetweenSlots(int slotIndex1, int slotIndex2)
{
    if (!isValidSlotIndex(slotIndex1) || !isValidSlotIndex(slotIndex2))
        return false;

    if (slotIndex1 == slotIndex2)
        return true; // No swap needed

    // Swap the gear between slots by moving them
    // First, get the gear from both slots
    juce::String gear1 = rack.getGearInSlot(slotIndex1);
    juce::String gear2 = rack.getGearInSlot(slotIndex2);

    // Clear both slots
    rack.removeGearFromSlot(slotIndex1);
    rack.removeGearFromSlot(slotIndex2);

    // Add gear to opposite slots
    bool success1 = rack.addGearToSlot(slotIndex2, gear1);
    bool success2 = rack.addGearToSlot(slotIndex1, gear2);

    bool success = success1 && success2;

    if (success)
    {
        std::cout << "[RackController] Successfully swapped gear between slots " << slotIndex1 << " and " << slotIndex2 << std::endl;
        updateModifiedState();
        return true;
    }
    else
    {
        std::cout << "[RackController] Failed to swap gear between slots " << slotIndex1 << " and " << slotIndex2 << std::endl;
        return false;
    }
}

GearItem *RackController::getGearInSlot(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return nullptr;

    // For now, return nullptr
    // This will be enhanced when we implement the full rack integration
    return nullptr;
}

bool RackController::isSlotEmpty(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex))
        return true;

    // For now, assume all slots are empty
    // This will be enhanced when we implement the full rack integration
    return true;
}

int RackController::getOccupiedSlotCount() const
{
    // For now, return 0
    // This will be enhanced when we implement the full rack integration
    return 0;
}

int RackController::getTotalSlotCount() const
{
    // Return the total number of slots in the rack
    return 16; // Default rack size
}

// Rack State Management

bool RackController::saveRackState()
{
    try
    {
        lastSavedState = serializeRackState();
        clearModifiedState();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool RackController::loadRackState(const juce::MemoryBlock &stateData)
{
    try
    {
        bool success = deserializeRackState(stateData);
        if (success)
        {
            clearModifiedState();
        }
        return success;
    }
    catch (...)
    {
        return false;
    }
}

juce::MemoryBlock RackController::getRackState() const
{
    return serializeRackState();
}

bool RackController::clearRack()
{
    // For now, just log the operation
    // This will be enhanced when we implement the full rack integration
    std::cout << "[RackController] Clearing all gear from rack" << std::endl;

    updateModifiedState();
    return true;
}

bool RackController::resetRack()
{
    // For now, just log the operation
    // This will be enhanced when we implement the full rack integration
    std::cout << "[RackController] Resetting rack to default state" << std::endl;

    updateModifiedState();
    return true;
}

// Drag and Drop Operations

void RackController::handleDragStartFromSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    if (!isValidSlotIndex(slotIndex))
        return;

    // For now, just log the operation
    // This will be enhanced when we implement the full drag and drop system
    std::cout << "[RackController] Started dragging from slot " << slotIndex << std::endl;
}

bool RackController::handleDropIntoSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    // For now, just log the operation
    // This will be enhanced when we implement the full drag and drop system
    std::cout << "[RackController] Dropped into slot " << slotIndex << std::endl;

    updateModifiedState();
    return true;
}

bool RackController::isValidDropForSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) const
{
    if (!isValidSlotIndex(slotIndex))
        return false;

    // For now, accept all drops
    // This will be enhanced when we implement the full drag and drop system
    return true;
}

// Rack Validation

bool RackController::validateRackConfiguration(juce::String &errorMessage) const
{
    // For now, assume the rack is always valid
    // This will be enhanced when we implement the full validation system
    errorMessage = "";
    return true;
}

bool RackController::checkForConflicts(juce::StringArray &conflicts) const
{
    // For now, assume no conflicts
    // This will be enhanced when we implement the full conflict checking system
    conflicts.clear();
    return false;
}

bool RackController::validateSlotConfiguration(int slotIndex, juce::String &errorMessage) const
{
    if (!isValidSlotIndex(slotIndex))
    {
        errorMessage = "Invalid slot index";
        return false;
    }

    // For now, assume all slot configurations are valid
    // This will be enhanced when we implement the full validation system
    errorMessage = "";
    return true;
}

// Preset Integration

bool RackController::saveRackToPreset(const juce::String &presetName)
{
    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset integration
        std::cout << "[RackController] Saving rack state to preset: " << presetName << std::endl;

        clearModifiedState();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool RackController::loadRackFromPreset(const juce::String &presetName)
{
    try
    {
        // For now, just log the operation
        // This will be enhanced when we implement the full preset integration
        std::cout << "[RackController] Loading rack state from preset: " << presetName << std::endl;

        clearModifiedState();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool RackController::isRackModified() const
{
    return isModified;
}

void RackController::markRackAsModified()
{
    isModified = true;
}

void RackController::clearModifiedState()
{
    isModified = false;
}

// Rack Information

juce::Array<GearItem *> RackController::getAllGearInRack() const
{
    // For now, return an empty array
    // This will be enhanced when we implement the full rack integration
    return juce::Array<GearItem *>();
}

juce::Array<GearItem *> RackController::getGearByType(GearItem::GearType gearType) const
{
    // For now, return an empty array
    // This will be enhanced when we implement the full rack integration
    return juce::Array<GearItem *>();
}

juce::Array<GearItem *> RackController::getGearByCategory(GearItem::GearCategory category) const
{
    // For now, return an empty array
    // This will be enhanced when we implement the full rack integration
    return juce::Array<GearItem *>();
}

double RackController::getTotalPowerConsumption() const
{
    return calculateTotalPowerConsumption();
}

double RackController::getTotalWeight() const
{
    return calculateTotalWeight();
}

// Private helper methods

bool RackController::isValidSlotIndex(int slotIndex) const
{
    return slotIndex >= 0 && slotIndex < getTotalSlotCount();
}

void RackController::updateModifiedState()
{
    isModified = true;
}

juce::MemoryBlock RackController::serializeRackState() const
{
    // For now, create a simple serialized state
    // This will be enhanced when we implement the full serialization system
    juce::MemoryBlock state;

    // Add a simple header
    juce::String header = "RackState_v1.0";
    state.append(header.toRawUTF8(), header.getNumBytesAsUTF8());

    return state;
}

bool RackController::deserializeRackState(const juce::MemoryBlock &stateData)
{
    // For now, just check if the data has a valid header
    // This will be enhanced when we implement the full deserialization system
    if (stateData.getSize() < 12) // Minimum size for header
        return false;

    juce::String header(static_cast<const char *>(stateData.getData()), 12);
    if (header.startsWith("RackState_v"))
    {
        return true;
    }

    return false;
}

double RackController::calculateTotalPowerConsumption() const
{
    // For now, return 0
    // This will be enhanced when we implement the full power calculation system
    return 0.0;
}

double RackController::calculateTotalWeight() const
{
    // For now, return 0
    // This will be enhanced when we implement the full weight calculation system
    return 0.0;
}
