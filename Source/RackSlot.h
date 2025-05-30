#pragma once

#include <JuceHeader.h>
#include "GearItem.h"

// Forward declaration
class Rack;

/**
 * RackSlot represents a slot in the rack that can contain a gear item.
 * It handles drag and drop from the gear library, and up/down movement via buttons.
 */
class RackSlot : public juce::Component,
                 public juce::DragAndDropTarget,
                 public juce::Button::Listener
{
public:
    RackSlot(int slotIndex);
    ~RackSlot() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Button handling
    void buttonClicked(juce::Button *button) override;
    void updateButtonStates();
    void moveUp();
    void moveDown();

    // Mouse events for control interaction
    void mouseDown(const juce::MouseEvent &e) override;
    void mouseDrag(const juce::MouseEvent &e) override;
    void mouseUp(const juce::MouseEvent &e) override;
    void mouseDoubleClick(const juce::MouseEvent &e) override;

    // DragAndDropTarget implementation - still used for library drops
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    // Gear item management
    void setGearItem(GearItem *newGearItem);
    void clearGearItem();
    GearItem *getGearItem() const { return gearItem; }
    bool isAvailable() const { return gearItem == nullptr; }

    // Visual state
    void setHighlighted(bool shouldHighlight);

    // Utility methods
    int getIndex() const { return index; }

private:
    // Helper methods for control rendering
    void drawControls(juce::Graphics &g, const juce::Rectangle<int> &faceplateArea);
    void drawKnob(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawSwitch(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawButton(juce::Graphics &g, const GearControl &control, int x, int y);
    void drawFader(juce::Graphics &g, const GearControl &control, int x, int y);

    // Helper methods for control interaction
    GearControl *findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<int> &faceplateArea);
    void handleKnobInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds);
    void handleSwitchInteraction(GearControl &control);
    void handleButtonInteraction(GearControl &control);
    void handleFaderInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds);

    // Helper method to find parent Rack
    juce::Component *findParentRackComponent() const;

    int index;                            // The slot's position in the rack
    GearItem *gearItem = nullptr;         // The gear item in this slot, if any
    bool highlighted;                     // Whether this slot is highlighted
    bool isDragging;                      // Whether a drag operation is in progress
    float dragStartValue = 0.0f;          // Control value at drag start
    float dragStartAngle = 0.0f;          // Control angle at drag start
    juce::Point<float> dragStartPos;      // Mouse position at drag start
    GearControl *activeControl = nullptr; // Currently active control being manipulated

    // Up/down movement buttons
    std::unique_ptr<juce::DrawableButton> upButton;
    std::unique_ptr<juce::DrawableButton> downButton;
    std::unique_ptr<juce::DrawableButton> removeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
};