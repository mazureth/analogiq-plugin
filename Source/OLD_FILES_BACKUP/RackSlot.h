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
#include "GearItem.h"

// Forward declaration
class Rack;
class PresetManager;
class GearLibrary;

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
     * @param slotIndex The index of this slot in the rack
     */
    RackSlot(IFileSystem &fileSystem, CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary, int slotIndex = 0);

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

    // DragAndDropTarget implementation - still used for library drops
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
     * @param dragSourceDetails Details about the drag source
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
     * @brief Sets a new gear item in the slot.
     *
     * @param newGearItem Pointer to the new gear item to set
     */
    void setGearItem(GearItem *newGearItem);

    /**
     * @brief Removes the current gear item from the slot.
     */
    void clearGearItem();

    /**
     * @brief Gets the current gear item in the slot.
     *
     * @return Pointer to the current gear item, or nullptr if slot is empty
     */
    GearItem *getGearItem() const { return gearItem; }

    /**
     * @brief Checks if the slot is available (empty).
     *
     * @return true if the slot is empty
     */
    bool isAvailable() const { return gearItem == nullptr; }

    // Visual state
    /**
     * @brief Sets whether the slot should be highlighted.
     *
     * @param shouldHighlight Whether the slot should be highlighted
     */
    void setHighlighted(bool shouldHighlight);

    // Utility methods
    /**
     * @brief Gets the index of this slot in the rack.
     *
     * @return The slot's index
     */
    int getIndex() const { return index; }

    // Instance management
    /**
     * @brief Creates a new instance of the current gear item.
     */
    void createInstance();

    /**
     * @brief Resets an instance to its source gear item.
     */
    void resetToSource();

    bool isInstance() const { return gearItem != nullptr && gearItem->isInstance; }
    juce::String getInstanceId() const { return gearItem != nullptr ? gearItem->instanceId : juce::String(); }

private:
    /**
     * @brief Notifies the rack that a gear item was added to this slot.
     */
    void notifyRackOfGearItemAdded();

    /**
     * @brief Notifies the rack that a gear item was removed from this slot.
     */
    void notifyRackOfGearItemRemoved();

    /**
     * @brief Notifies the rack that a control was changed in this slot.
     *
     * @param controlIndex The index of the control that was changed
     */
    void notifyRackOfControlChanged(int controlIndex);

private:
    // Helper methods for control rendering
    /**
     * @brief Draws all controls for the gear item.
     *
     * @param g The graphics context to paint with
     * @param faceplateArea The area where the faceplate is drawn
     */
    void drawControls(juce::Graphics &g, const juce::Rectangle<int> &faceplateArea);

    /**
     * @brief Draws a switch control.
     *
     * @param g The graphics context to paint with
     * @param control The control to draw
     * @param x The x-coordinate for the control
     * @param y The y-coordinate for the control
     */
    void drawSwitch(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a button control.
     *
     * @param g The graphics context to paint with
     * @param control The control to draw
     * @param x The x-coordinate for the control
     * @param y The y-coordinate for the control
     */
    void drawButton(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a fader control.
     *
     * @param g The graphics context to paint with
     * @param control The control to draw
     * @param x The x-coordinate for the control
     * @param y The y-coordinate for the control
     */
    void drawFader(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a knob control.
     *
     * @param g The graphics context to paint with
     * @param control The control to draw
     * @param x The x-coordinate for the control
     * @param y The y-coordinate for the control
     */
    void drawKnob(juce::Graphics &g, const GearControl &control, int x, int y);

    // Helper methods for control interaction
    /**
     * @brief Finds a control at the specified position.
     *
     * @param position The position to check
     * @param faceplateArea The area where the faceplate is drawn
     * @return Pointer to the control at the position, or nullptr if none found
     */
    GearControl *findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<int> &faceplateArea);

    /**
     * @brief Resets a control to its default value.
     *
     * @param e The mouse event that triggered the reset
     */
    void resetControlToDefault(const juce::MouseEvent &e);

    /**
     * @brief Handles interaction with a switch control.
     *
     * @param control The switch control to handle
     */
    void handleSwitchInteraction(GearControl &control);

    /**
     * @brief Handles interaction with a button control.
     *
     * @param control The button control to handle
     */
    void handleButtonInteraction(GearControl &control);

    /**
     * @brief Handles interaction with a fader control.
     *
     * @param control The fader control to handle
     * @param mousePos The current mouse position
     * @param controlBounds The bounds of the control
     */
    void handleFaderInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds);

    // Helper method to find parent Rack
    /**
     * @brief Finds the parent Rack component.
     *
     * @return Pointer to the parent Rack component, or nullptr if not found
     */
    juce::Component *findParentRackComponent() const;

    int index;                            ///< The slot's position in the rack
    GearItem *gearItem = nullptr;         ///< The gear item in this slot, if any
    bool highlighted;                     ///< Whether this slot is highlighted
    bool isDragging;                      ///< Whether a drag operation is in progress
    float dragStartValue = 0.0f;          ///< Control value at drag start
    juce::Point<float> dragStartPos;      ///< Mouse position at drag start
    GearControl *activeControl = nullptr; ///< Currently active control being manipulated
    float currentFaceplateScale = 1.0f;   ///< Current scale factor for faceplate rendering

    // Up/down movement buttons
    std::unique_ptr<juce::DrawableButton> upButton;     ///< Button for moving item up
    std::unique_ptr<juce::DrawableButton> downButton;   ///< Button for moving item down
    std::unique_ptr<juce::DrawableButton> removeButton; ///< Button for removing item

    IFileSystem &fileSystem;
    CacheManager &cacheManager;
    PresetManager &presetManager;
    GearLibrary &gearLibrary;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
};