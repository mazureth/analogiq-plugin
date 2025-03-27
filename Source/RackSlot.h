#pragma once

#include <JuceHeader.h>
#include "GearItem.h"

class RackSlot : public juce::Component,
                 public juce::DragAndDropTarget
{
public:
    RackSlot(int slotIndex);
    ~RackSlot() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // DragAndDropTarget implementation
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    // Getters and setters
    int getIndex() const { return index; }
    bool isAvailable() const { return gearItem == nullptr; }
    GearItem* getGearItem() const { return gearItem; }
    void setGearItem(GearItem* newGearItem);
    void clearGearItem();
    void setHighlighted(bool shouldHighlight);
    bool isHighlighted() const { return highlighted; }

    // New helper method
    juce::Component* findParentRackComponent();

private:
    int index;
    GearItem* gearItem = nullptr;
    bool highlighted = false;
    bool isDragging = false;
    std::unique_ptr<juce::DragAndDropTarget::SourceDetails> dragSourceDetails;
    juce::ComponentDragger dragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
}; 