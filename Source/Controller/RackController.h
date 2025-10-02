/**
 * @file RackController.h
 * @brief Header file for the RackController class.
 *
 * This file defines the rack controller that coordinates between
 * the Rack model and UI components, handling rack operations and state changes.
 */

#pragma once

#include <JuceHeader.h>
#include "../View/Rack.h"
#include "../Model/RackModel.h"
#include "../Model/PresetManager.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"

// Forward declarations
class GearItem;

/**
 * @brief Controller for rack operations and state changes.
 *
 * The RackController is responsible for:
 * - Coordinating between Rack (Model) and UI components
 * - Handling rack slot operations (add/remove/move gear)
 * - Managing rack state and persistence
 * - Handling drag-and-drop operations
 * - Coordinating with preset management
 * - Managing rack validation and error handling
 */
class RackController
{
public:
    /**
     * @brief Constructs a new RackController.
     *
     * @param rack Reference to the rack model
     * @param presetManager Reference to the preset manager
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     */
    RackController(RackModel &rackModel,
                   PresetManager &presetManager,
                   IFileSystem &fileSystem,
                   ICacheManager &cacheManager);

    /**
     * @brief Destructor for RackController.
     */
    ~RackController() = default;

    // Rack Slot Operations
    /**
     * @brief Adds a gear item to a specific rack slot.
     *
     * @param slotIndex The index of the slot to add gear to
     * @param gearItem The gear item to add
     * @return true if successful, false otherwise
     */
    bool addGearToSlot(int slotIndex, GearItem *gearItem);

    /**
     * @brief Removes a gear item from a specific rack slot.
     *
     * @param slotIndex The index of the slot to remove gear from
     * @return true if successful, false otherwise
     */
    bool removeGearFromSlot(int slotIndex);

    /**
     * @brief Moves a gear item from one slot to another.
     *
     * @param fromSlotIndex The source slot index
     * @param toSlotIndex The destination slot index
     * @return true if successful, false otherwise
     */
    bool moveGearBetweenSlots(int fromSlotIndex, int toSlotIndex);

    /**
     * @brief Swaps gear items between two slots.
     *
     * @param slotIndex1 The first slot index
     * @param slotIndex2 The second slot index
     * @return true if successful, false otherwise
     */
    bool swapGearBetweenSlots(int slotIndex1, int slotIndex2);

    /**
     * @brief Gets the gear item in a specific slot.
     *
     * @param slotIndex The slot index
     * @return Pointer to the gear item, or nullptr if empty
     */
    GearItem *getGearInSlot(int slotIndex) const;

    /**
     * @brief Checks if a slot is empty.
     *
     * @param slotIndex The slot index
     * @return true if the slot is empty, false otherwise
     */
    bool isSlotEmpty(int slotIndex) const;

    /**
     * @brief Gets the number of occupied slots.
     *
     * @return Number of slots containing gear
     */
    int getOccupiedSlotCount() const;

    /**
     * @brief Gets the total number of slots in the rack.
     *
     * @return Total number of slots
     */
    int getTotalSlotCount() const;

    // Rack State Management
    /**
     * @brief Saves the current rack state.
     *
     * @return true if successful, false otherwise
     */
    bool saveRackState();

    /**
     * @brief Loads a rack state.
     *
     * @param stateData The state data to load
     * @return true if successful, false otherwise
     */
    bool loadRackState(const juce::MemoryBlock &stateData);

    /**
     * @brief Gets the current rack state as a memory block.
     *
     * @return Memory block containing the rack state
     */
    juce::MemoryBlock getRackState() const;

    /**
     * @brief Clears all gear from the rack.
     *
     * @return true if successful, false otherwise
     */
    bool clearRack();

    /**
     * @brief Resets the rack to its default state.
     *
     * @return true if successful, false otherwise
     */
    bool resetRack();

    // Drag and Drop Operations
    /**
     * @brief Handles the start of a drag operation from a slot.
     *
     * @param slotIndex The slot index being dragged from
     * @param dragSourceDetails Details about the drag source
     */
    void handleDragStartFromSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails);

    /**
     * @brief Handles a drop operation into a slot.
     *
     * @param slotIndex The target slot index
     * @param dragSourceDetails Details about the drag source
     * @return true if the drop was successful, false otherwise
     */
    bool handleDropIntoSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails);

    /**
     * @brief Checks if a drop operation is valid for a specific slot.
     *
     * @param slotIndex The target slot index
     * @param dragSourceDetails Details about the drag source
     * @return true if the drop is valid, false otherwise
     */
    bool isValidDropForSlot(int slotIndex, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) const;

    // Rack Validation
    /**
     * @brief Validates the current rack configuration.
     *
     * @param errorMessage Output parameter for error message
     * @return true if the rack is valid, false otherwise
     */
    bool validateRackConfiguration(juce::String &errorMessage) const;

    /**
     * @brief Checks if the rack has any configuration conflicts.
     *
     * @param conflicts Output parameter for list of conflicts
     * @return true if there are conflicts, false otherwise
     */
    bool checkForConflicts(juce::StringArray &conflicts) const;

    /**
     * @brief Validates a specific slot configuration.
     *
     * @param slotIndex The slot index to validate
     * @param errorMessage Output parameter for error message
     * @return true if the slot is valid, false otherwise
     */
    bool validateSlotConfiguration(int slotIndex, juce::String &errorMessage) const;

    // Preset Integration
    /**
     * @brief Saves the rack state to a preset.
     *
     * @param presetName The name of the preset
     * @return true if successful, false otherwise
     */
    bool saveRackToPreset(const juce::String &presetName);

    /**
     * @brief Loads a rack state from a preset.
     *
     * @param presetName The name of the preset
     * @return true if successful, false otherwise
     */
    bool loadRackFromPreset(const juce::String &presetName);

    /**
     * @brief Checks if the rack state has been modified since last save.
     *
     * @return true if modified, false otherwise
     */
    bool isRackModified() const;

    /**
     * @brief Marks the rack as modified.
     */
    void markRackAsModified();

    /**
     * @brief Clears the modified state.
     */
    void clearModifiedState();

    // Rack Information
    /**
     * @brief Gets a list of all gear items in the rack.
     *
     * @return Array of gear items
     */
    juce::Array<GearItem *> getAllGearInRack() const;

    /**
     * @brief Gets a list of gear items by type.
     *
     * @param gearType The gear type to filter by
     * @return Array of matching gear items
     */
    juce::Array<GearItem *> getGearByType(GearItem::GearType gearType) const;

    /**
     * @brief Gets a list of gear items by category.
     *
     * @param category The category to filter by
     * @return Array of matching gear items
     */
    juce::Array<GearItem *> getGearByCategory(GearItem::GearCategory category) const;

    /**
     * @brief Gets the rack's total power consumption.
     *
     * @return Power consumption in watts
     */
    double getTotalPowerConsumption() const;

    /**
     * @brief Gets the rack's total weight.
     *
     * @return Weight in pounds
     */
    double getTotalWeight() const;

private:
    // Core dependencies
    RackModel &rackModel;
    PresetManager &presetManager;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;

    // State tracking
    bool isModified{false};
    juce::MemoryBlock lastSavedState;

    // Private helper methods
    /**
     * @brief Validates that a slot index is within bounds.
     *
     * @param slotIndex The slot index to validate
     * @return true if valid, false otherwise
     */
    bool isValidSlotIndex(int slotIndex) const;

    /**
     * @brief Updates the rack's modified state.
     */
    void updateModifiedState();

    /**
     * @brief Serializes the rack state to a memory block.
     *
     * @return Memory block containing the serialized state
     */
    juce::MemoryBlock serializeRackState() const;

    /**
     * @brief Deserializes a memory block to restore rack state.
     *
     * @param stateData The state data to deserialize
     * @return true if successful, false otherwise
     */
    bool deserializeRackState(const juce::MemoryBlock &stateData);

    /**
     * @brief Calculates the total power consumption of all gear in the rack.
     *
     * @return Total power consumption in watts
     */
    double calculateTotalPowerConsumption() const;

    /**
     * @brief Calculates the total weight of all gear in the rack.
     *
     * @return Total weight in pounds
     */
    double calculateTotalWeight() const;
};
