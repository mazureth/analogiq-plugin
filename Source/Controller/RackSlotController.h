/**
 * @file RackSlotController.h
 * @brief Header file for the RackSlotController class.
 *
 * This file defines the rack slot controller that coordinates individual
 * slot management operations and state changes.
 */

#pragma once

#include <JuceHeader.h>
#include "../View/RackSlot.h"
#include "../Model/GearItem.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"

// Forward declarations
class Rack;

/**
 * @brief Controller for individual rack slot management operations.
 *
 * The RackSlotController is responsible for:
 * - Coordinating between RackSlot (Model) and UI components
 * - Handling individual slot operations (gear insertion/removal)
 * - Managing slot state and validation
 * - Handling slot-specific user interactions
 * - Coordinating with the parent rack controller
 * - Managing slot-specific error handling
 */
class RackSlotController
{
public:
    /**
     * @brief Constructs a new RackSlotController.
     *
     * @param rackSlot Reference to the rack slot model
     * @param parentRack Reference to the parent rack
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     */
    RackSlotController(RackSlot &rackSlot,
                       Rack &parentRack,
                       IFileSystem &fileSystem,
                       ICacheManager &cacheManager);

    /**
     * @brief Destructor for RackSlotController.
     */
    ~RackSlotController() = default;

    // Slot Gear Operations
    /**
     * @brief Inserts a gear item into the slot.
     *
     * @param gearItem The gear item to insert
     * @return true if successful, false otherwise
     */
    bool insertGear(GearItem *gearItem);

    /**
     * @brief Removes the gear item from the slot.
     *
     * @return Pointer to the removed gear item, or nullptr if slot was empty
     */
    GearItem* removeGear();

    /**
     * @brief Gets the gear item currently in the slot.
     *
     * @return Pointer to the gear item, or nullptr if slot is empty
     */
    GearItem* getGear() const;

    /**
     * @brief Checks if the slot is empty.
     *
     * @return true if the slot is empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Checks if the slot is occupied.
     *
     * @return true if the slot is occupied, false otherwise
     */
    bool isOccupied() const;

    // Slot State Management
    /**
     * @brief Gets the slot index.
     *
     * @return The slot index
     */
    int getSlotIndex() const;

    /**
     * @brief Sets the slot index.
     *
     * @param index The new slot index
     */
    void setSlotIndex(int index);

    /**
     * @brief Gets the slot's current state.
     *
     * @return String representation of the slot state
     */
    juce::String getSlotState() const;

    /**
     * @brief Sets the slot's state.
     *
     * @param state The new state string
     * @return true if successful, false otherwise
     */
    bool setSlotState(const juce::String &state);

    /**
     * @brief Resets the slot to its default state.
     *
     * @return true if successful, false otherwise
     */
    bool resetSlot();

    // Slot Validation
    /**
     * @brief Validates if a gear item can be inserted into this slot.
     *
     * @param gearItem The gear item to validate
     * @param errorMessage Output parameter for error message
     * @return true if the gear can be inserted, false otherwise
     */
    bool canInsertGear(const GearItem *gearItem, juce::String &errorMessage) const;

    /**
     * @brief Validates the current slot configuration.
     *
     * @param errorMessage Output parameter for error message
     * @return true if the slot is valid, false otherwise
     */
    bool validateSlotConfiguration(juce::String &errorMessage) const;

    /**
     * @brief Checks for conflicts with the current slot configuration.
     *
     * @param conflicts Output parameter for list of conflicts
     * @return true if there are conflicts, false otherwise
     */
    bool checkForConflicts(juce::StringArray &conflicts) const;

    // Slot Information
    /**
     * @brief Gets the slot's display name.
     *
     * @return The slot's display name
     */
    juce::String getDisplayName() const;

    /**
     * @brief Gets the slot's tooltip text.
     *
     * @return The slot's tooltip text
     */
    juce::String getTooltipText() const;

    /**
     * @brief Gets the slot's status information.
     *
     * @return The slot's status information
     */
    juce::String getStatusInfo() const;

    // Slot UI Operations
    /**
     * @brief Highlights the slot (for selection feedback).
     */
    void highlightSlot();

    /**
     * @brief Removes highlighting from the slot.
     */
    void removeHighlight();

    /**
     * @brief Checks if the slot is currently highlighted.
     *
     * @return true if highlighted, false otherwise
     */
    bool isHighlighted() const;

    /**
     * @brief Sets the slot's visual state.
     *
     * @param state The visual state to set
     */
    void setVisualState(const juce::String &state);

    /**
     * @brief Gets the slot's current visual state.
     *
     * @return The current visual state
     */
    juce::String getVisualState() const;

    // Slot Navigation
    /**
     * @brief Moves the slot's gear to the next available slot.
     *
     * @return true if successful, false otherwise
     */
    bool moveToNextSlot();

    /**
     * @brief Moves the slot's gear to the previous available slot.
     *
     * @return true if successful, false otherwise
     */
    bool moveToPreviousSlot();

    /**
     * @brief Swaps gear with another slot.
     *
     * @param otherSlotIndex The index of the other slot
     * @return true if successful, false otherwise
     */
    bool swapWithSlot(int otherSlotIndex);

    // Slot Events
    /**
     * @brief Handles a mouse click on the slot.
     *
     * @param event The mouse event
     * @return true if the event was handled, false otherwise
     */
    bool handleMouseClick(const juce::MouseEvent &event);

    /**
     * @brief Handles a mouse double-click on the slot.
     *
     * @param event The mouse event
     * @return true if the event was handled, false otherwise
     */
    bool handleMouseDoubleClick(const juce::MouseEvent &event);

    /**
     * @brief Handles a right-click on the slot.
     *
     * @param event The mouse event
     * @return true if the event was handled, false otherwise
     */
    bool handleRightClick(const juce::MouseEvent &event);

    /**
     * @brief Handles a drag start from the slot.
     *
     * @param event The mouse event
     * @return true if the event was handled, false otherwise
     */
    bool handleDragStart(const juce::MouseEvent &event);

    /**
     * @brief Handles a drop into the slot.
     *
     * @param event The mouse event
     * @return true if the event was handled, false otherwise
     */
    bool handleDrop(const juce::MouseEvent &event);

private:
    // Core dependencies
    RackSlot &rackSlot;
    Rack &parentRack;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;

    // State tracking
    bool isHighlighted_{false};
    juce::String visualState{"normal"};
    juce::String slotState{"empty"};

    // Private helper methods
    /**
     * @brief Updates the slot's state based on current gear.
     */
    void updateSlotState();

    /**
     * @brief Validates that a slot index is within bounds.
     *
     * @param slotIndex The slot index to validate
     * @return true if valid, false otherwise
     */
    bool isValidSlotIndex(int slotIndex) const;

    /**
     * @brief Notifies the parent rack of slot changes.
     */
    void notifyParentRack();

    /**
     * @brief Generates a display name for the slot.
     *
     * @return The generated display name
     */
    juce::String generateDisplayName() const;

    /**
     * @brief Generates tooltip text for the slot.
     *
     * @return The generated tooltip text
     */
    juce::String generateTooltipText() const;

    /**
     * @brief Generates status information for the slot.
     *
     * @return The generated status information
     */
    juce::String generateStatusInfo() const;
};
