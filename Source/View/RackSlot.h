/**
 * @file RackSlot.h
 * @brief Header file for the RackSlot class which represents a single slot in the virtual rack.
 *
 * This file defines the RackSlot class which provides the visual representation and
 * interaction handling for individual slots in the virtual rack. It manages the display
 * of gear items, their controls, and handles user interactions like drag-and-drop and
 * button clicks.
 */

#pragma once

#include <JuceHeader.h>
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Model/PresetManager.h"
#include "../Model/GearLibrary.h"

// Forward declarations
class Rack;
class GearItem;

/**
 * @class RackSlot
 * @brief Represents a slot in the rack that can contain a gear item.
 *
 * The RackSlot class handles the visual representation and interaction with a single
 * slot in the virtual rack. It manages gear items, their controls, and provides
 * functionality for drag-and-drop operations, button interactions, and control
 * manipulation.
 */
class RackSlot : public juce::Component,
                 public juce::DragAndDropTarget,
                 public juce::Button::Listener
{
public:
    /**
     * @brief Constructs a new RackSlot instance.
     *
     * @param fileSystem Reference to the file system
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     * @param gearLibrary Reference to the gear library
     * @param slotIndex The index of this slot in the rack
     */
    RackSlot(IFileSystem &fileSystem,
             ICacheManager &cacheManager,
             PresetManager &presetManager,
             GearLibrary &gearLibrary,
             int slotIndex = 0);

    /**
     * @brief Sets the index of this slot in the rack.
     *
     * @param newIndex The new index for this slot
     */
    void setIndex(int newIndex)
    {
        index = newIndex;
        updateButtonStates();
    }

    /**
     * @brief Destructor for the RackSlot class.
     *
     * Cleans up resources and ensures all images are properly released.
     */
    ~RackSlot() override;

    /**
     * @brief Paints the rack slot and its contents.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handles resizing of the rack slot component.
     */
    void resized() override;

    // Button handling
    /**
     * @brief Handles button click events for the navigation buttons.
     *
     * @param button Pointer to the button that was clicked
     */
    void buttonClicked(juce::Button *button) override;

    /**
     * @brief Updates the enabled state of the navigation buttons.
     */
    void updateButtonStates();

    /**
     * @brief Moves the gear item up one slot in the rack.
     */
    void moveUp();

    /**
     * @brief Moves the gear item down one slot in the rack.
     */
    void moveDown();

    // Mouse events for control interaction
    /**
     * @brief Handles mouse down events on the rack slot.
     *
     * @param e The mouse event details
     */
    void mouseDown(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse drag events on the rack slot.
     *
     * @param e The mouse event details
     */
    void mouseDrag(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse up events on the rack slot.
     *
     * @param e The mouse event details
     */
    void mouseUp(const juce::MouseEvent &e) override;

    // DragAndDropTarget methods
    /**
     * @brief Checks if the slot is interested in a drag source.
     *
     * @param dragSourceDetails Details about the drag source
     * @return true if the slot accepts drops from this source
     */
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item enters the slot.
     *
     * @param dragSourceDetails Details about the drag source
     */
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item moves over the slot.
     *
     * @param dragSourceDetails Details about the drag source and position
     */
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item exits the slot.
     *
     * @param dragSourceDetails Details about the drag source
     */
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Handles when a dragged item is dropped onto the slot.
     *
     * @param dragSourceDetails Details about the drag source and drop position
     */
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    // Gear item management
    /**
     * @brief Sets the gear item for this slot.
     *
     * @param item Pointer to the gear item to set
     */
    void setGearItem(GearItem *item);

    /**
     * @brief Gets the gear item in this slot.
     *
     * @return Pointer to the gear item, or nullptr if no item
     */
    GearItem *getGearItem() const { return gearItem; }

    /**
     * @brief Clears the gear item from this slot.
     */
    void clearGearItem();

    /**
     * @brief Checks if the slot is empty.
     *
     * @return true if the slot is empty, false otherwise
     */
    bool isEmpty() const { return gearItem == nullptr; }

    /**
     * @brief Gets the slot index.
     *
     * @return The index of this slot
     */
    int getSlotIndex() const { return index; }

private:
    // Slot information
    int index; ///< Index of this slot in the rack

    // Gear item
    GearItem *gearItem = nullptr; ///< Pointer to the gear item in this slot

    // Dependencies
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    PresetManager &presetManager;
    GearLibrary &gearLibrary;

    // UI Components
    juce::TextButton upButton{"Up"};     ///< Button to move gear item up
    juce::TextButton downButton{"Down"}; ///< Button to move gear item down

    // Drag and drop state
    bool isDragOver = false; ///< Whether a drag operation is currently over this slot

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
};
