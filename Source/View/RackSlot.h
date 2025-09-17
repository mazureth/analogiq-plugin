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
#include "../Model/RackModel.h"
#include "../Shared/GearControl.h"

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
     * @param rackModel Reference to the rack model for data access
     * @param slotIndex The index of this slot in the rack
     */
    RackSlot(RackModel &rackModel, int slotIndex = 0);

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
     * @brief Sets the reference to the parent rack.
     *
     * @param parentRack Pointer to the parent rack
     */
    void setRack(Rack *parentRack)
    {
        rack = parentRack;
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

    /**
     * @brief Removes the gear item from this slot.
     */
    void removeGear();

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

    /**
     * @brief Handles mouse wheel events on the rack slot.
     *
     * @param e The mouse wheel event details
     */
    void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override;

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

    // Gear item data now comes from RackModel - no local management needed

    // Faceplate scaling and control rendering
    /**
     * @brief Gets the current faceplate scale for this slot.
     *
     * @return The current scale factor (1.0 = original size)
     */
    float getCurrentFaceplateScale() const { return currentFaceplateScale; }

    /**
     * @brief Calculates the optimal faceplate scale for this slot.
     *
     * @return The calculated scale factor
     */

    /**
     * @brief Draws all controls for the gear item with proper scaling.
     *
     * @param g The graphics context to paint with
     * @param actualImageBounds The actual rendered bounds of the faceplate image
     */
    void drawControls(juce::Graphics &g, const juce::Rectangle<float> &actualImageBounds);

    /**
     * @brief Checks if the slot is empty.
     *
     * @return true if the slot is empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Gets the slot index.
     *
     * @return The index of this slot
     */
    int getSlotIndex() const { return index; }

    /**
     * @brief Sets a callback to be called when this slot's gear item faceplate is loaded.
     *
     * @param callback Function to call when faceplate is loaded
     */
    void setFaceplateLoadedCallback(std::function<void()> callback);

    /**
     * @brief Triggers the faceplate loaded callback for this specific slot.
     */
    void triggerFaceplateLoaded();

    /**
     * @brief Sets the background color for this slot.
     *
     * @param color The background color to use
     */
    void setSlotBackgroundColor(juce::Colour color);

private:
    // Helper methods for drawing individual control types
    void drawButtonControl(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawFaderControl(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawSwitchControl(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawKnobControl(juce::Graphics &g, const GearControl &control, int x, int y);

    // Helper methods for control interaction
    GearControl *findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<float> &actualImageBounds);
    void resetControlToDefault(const juce::MouseEvent &e);
    void handleKnobInteraction(GearControl &control, const juce::MouseEvent &e);
    void handleKnobDrag(GearControl &control, const juce::MouseEvent &e);
    void handleKnobReset(GearControl &control);
    void updateKnobValue(GearControl &control, float deltaAngle, const juce::String &source);
    void handleFaderDrag(GearControl &control, const juce::MouseEvent &e);
    void updateFaderValue(GearControl &control, float deltaValue, const juce::String &source);
    void handleSwitchDrag(GearControl &control, const juce::MouseEvent &e);
    void handleButtonInteraction(GearControl &control);
    void notifyRackOfControlChanged(int controlIndex);

private:
    // Slot information
    int index; ///< Index of this slot in the rack

    // Gear item data now comes from RackModel - no local storage

    // Dependencies
    RackModel &rackModel;
    Rack *rack = nullptr; ///< Reference to the parent rack for movement operations

    // UI Components
    std::unique_ptr<juce::DrawableButton> upButton;     ///< Button to move gear item up
    std::unique_ptr<juce::DrawableButton> downButton;   ///< Button to move gear item down
    std::unique_ptr<juce::DrawableButton> removeButton; ///< Button to remove gear item

    // Drag and drop state
    bool isDragOver = false; ///< Whether a drag operation is currently over this slot

    // Faceplate scaling
    float currentFaceplateScale = 1.0f; ///< Current scale factor for faceplate and controls

    // Faceplate loading callback
    std::function<void()> faceplateLoadedCallback; ///< Callback to trigger when faceplate is loaded

    // Visual properties
    juce::Colour slotBackgroundColor = juce::Colours::transparentBlack; ///< Background color for this slot

    // Control interaction state
    bool isDragging = false;              ///< Whether a control is currently being dragged
    float dragStartValue = 0.0f;          ///< Control value at drag start
    juce::Point<float> dragStartPos;      ///< Mouse position at drag start
    juce::Point<float> lastMousePos;      ///< Previous mouse position for incremental movement
    GearControl *activeControl = nullptr; ///< Currently active control being manipulated

    // Knob interaction sensitivity constants
    static constexpr float KNOB_DRAG_SENSITIVITY = 0.5f;        ///< Sensitivity for mouse drag knob rotation (lower = slower)
    static constexpr float KNOB_WHEEL_SENSITIVITY = 20.0f;      ///< Sensitivity for mouse wheel knob rotation (lower = slower)
    static constexpr float KNOB_WHEEL_SENSITIVITY_STEP = 50.0f; ///< Degrees of movement required before stepped knob changes step (higher = less sensitive)

    // Fader interaction sensitivity constants
    static constexpr float FADER_WHEEL_SENSITIVITY = 0.01f; ///< Sensitivity for mouse wheel fader movement (lower = slower)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
};
