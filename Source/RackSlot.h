/**
 * @file RackSlot.h
 * @brief Header file for the RackSlot class, which represents a slot in the rack that can contain gear items.
 *
 * The RackSlot class is responsible for:
 * - Managing individual slots in the rack system
 * - Handling drag and drop operations for gear items
 * - Rendering gear items and their controls
 * - Managing control interactions (buttons, switches, faders, knobs)
 * - Supporting instance management for gear items
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>
#include "GearItem.h"

// Forward declaration
class Rack;

/**
 * @class RackSlot
 * @brief Represents a slot in the rack that can contain a gear item.
 *
 * RackSlot is a component that manages a single slot in the rack system. It handles:
 * - Visual representation of gear items and their controls
 * - Drag and drop operations from the gear library
 * - Control interactions (buttons, switches, faders, knobs)
 * - Instance management for gear items
 * - Up/down movement of items within the rack
 */
class RackSlot : public juce::Component,
                 public juce::DragAndDropTarget,
                 public juce::Button::Listener
{
public:
    /**
     * @brief Constructs a new RackSlot with the specified index.
     * @param slotIndex The position of this slot in the rack.
     */
    RackSlot(int slotIndex);

    /**
     * @brief Destructor for RackSlot.
     */
    ~RackSlot() override;

    /**
     * @brief Paints the rack slot and its contents.
     * @param g The graphics context to paint with.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Called when the component is resized.
     * Updates the layout of internal components.
     */
    void resized() override;

    /**
     * @brief Handles button click events.
     * @param button The button that was clicked.
     */
    void buttonClicked(juce::Button *button) override;

    /**
     * @brief Updates the state of the up/down/remove buttons based on current conditions.
     */
    void updateButtonStates();

    /**
     * @brief Moves the current gear item up one position in the rack.
     */
    void moveUp();

    /**
     * @brief Moves the current gear item down one position in the rack.
     */
    void moveDown();

    /**
     * @brief Handles mouse down events for control interaction.
     * @param e The mouse event details.
     */
    void mouseDown(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse drag events for control interaction.
     * @param e The mouse event details.
     */
    void mouseDrag(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse up events for control interaction.
     * @param e The mouse event details.
     */
    void mouseUp(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse double-click events for control interaction.
     * @param e The mouse event details.
     */
    void mouseDoubleClick(const juce::MouseEvent &e) override;

    /**
     * @brief Checks if this slot is interested in a drag source.
     * @param dragSourceDetails Details about the drag source.
     * @return true if this slot can accept the drag source.
     */
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation enters this slot.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation moves within this slot.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation exits this slot.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Called when a drag operation is dropped on this slot.
     * @param dragSourceDetails Details about the drag source.
     */
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    /**
     * @brief Sets a new gear item in this slot.
     * @param newGearItem The gear item to set.
     */
    void setGearItem(GearItem *newGearItem);

    /**
     * @brief Removes the current gear item from this slot.
     */
    void clearGearItem();

    /**
     * @brief Gets the current gear item in this slot.
     * @return Pointer to the current gear item, or nullptr if empty.
     */
    GearItem *getGearItem() const { return gearItem; }

    /**
     * @brief Checks if this slot is available (empty).
     * @return true if the slot is empty.
     */
    bool isAvailable() const { return gearItem == nullptr; }

    /**
     * @brief Sets the highlighted state of this slot.
     * @param shouldHighlight Whether the slot should be highlighted.
     */
    void setHighlighted(bool shouldHighlight);

    /**
     * @brief Gets the index of this slot in the rack.
     * @return The slot's index.
     */
    int getIndex() const { return index; }

    /**
     * @brief Creates an instance of the current gear item.
     */
    void createInstance();

    /**
     * @brief Resets the current gear item to its source state.
     */
    void resetToSource();

    /**
     * @brief Checks if the current gear item is an instance.
     * @return true if the current gear item is an instance.
     */
    bool isInstance() const { return gearItem != nullptr && gearItem->isInstance; }

    /**
     * @brief Gets the instance ID of the current gear item.
     * @return The instance ID, or empty string if not an instance.
     */
    juce::String getInstanceId() const { return gearItem != nullptr ? gearItem->instanceId : juce::String(); }

private:
    /**
     * @brief Draws all controls for the current gear item.
     * @param g The graphics context to draw with.
     * @param faceplateArea The area in which to draw the controls.
     */
    void drawControls(juce::Graphics &g, const juce::Rectangle<int> &faceplateArea);

    /**
     * @brief Draws a switch control.
     * @param g The graphics context to draw with.
     * @param control The control to draw.
     * @param x The x-coordinate to draw at.
     * @param y The y-coordinate to draw at.
     */
    void drawSwitch(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a button control.
     * @param g The graphics context to draw with.
     * @param control The control to draw.
     * @param x The x-coordinate to draw at.
     * @param y The y-coordinate to draw at.
     */
    void drawButton(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a fader control.
     * @param g The graphics context to draw with.
     * @param control The control to draw.
     * @param x The x-coordinate to draw at.
     * @param y The y-coordinate to draw at.
     */
    void drawFader(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Draws a knob control.
     * @param g The graphics context to draw with.
     * @param control The control to draw.
     * @param x The x-coordinate to draw at.
     * @param y The y-coordinate to draw at.
     */
    void drawKnob(juce::Graphics &g, const GearControl &control, int x, int y);

    /**
     * @brief Finds a control at the specified position.
     * @param position The position to check.
     * @param faceplateArea The area containing the controls.
     * @return Pointer to the control at the position, or nullptr if none found.
     */
    GearControl *findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<int> &faceplateArea);

    /**
     * @brief Handles interaction with a switch control.
     * @param control The control to handle.
     */
    void handleSwitchInteraction(GearControl &control);

    /**
     * @brief Handles interaction with a button control.
     * @param control The control to handle.
     */
    void handleButtonInteraction(GearControl &control);

    /**
     * @brief Handles interaction with a fader control.
     * @param control The control to handle.
     * @param mousePos The current mouse position.
     * @param controlBounds The bounds of the control.
     */
    void handleFaderInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds);

    /**
     * @brief Finds the parent Rack component.
     * @return Pointer to the parent Rack component, or nullptr if not found.
     */
    juce::Component *findParentRackComponent() const;

    int index;                            // The slot's position in the rack
    GearItem *gearItem = nullptr;         // The gear item in this slot, if any
    bool highlighted;                     // Whether this slot is highlighted
    bool isDragging;                      // Whether a drag operation is in progress
    float dragStartValue = 0.0f;          // Control value at drag start
    juce::Point<float> dragStartPos;      // Mouse position at drag start
    GearControl *activeControl = nullptr; // Currently active control being manipulated
    float currentFaceplateScale = 1.0f;   // Current scale factor for faceplate rendering

    // Up/down movement buttons
    std::unique_ptr<juce::DrawableButton> upButton;
    std::unique_ptr<juce::DrawableButton> downButton;
    std::unique_ptr<juce::DrawableButton> removeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
};