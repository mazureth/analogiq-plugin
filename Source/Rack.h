#pragma once

#include <JuceHeader.h>
#include "GearItem.h"
#include "RackSlot.h"

class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    Rack();
    ~Rack() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // DragAndDropTarget methods
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    // Rack management
    void addGearItem(GearItem* gearItem, int slotIndex);
    void removeGearItem(int slotIndex);
    void rearrangeGear(int sourceSlotIndex, int destSlotIndex);
    
    // Slot finding
    RackSlot* findNearestSlot(const juce::Point<int>& position);

private:
    juce::OwnedArray<RackSlot> slots;
    juce::Viewport rackViewport;
    juce::Component rackContainer;
    
    int series500SlotCount = 8;  // Number of 500-series slots
    int rack19SlotCount = 6;     // Number of 19-inch rack slots (in 1U increments)
    
    void initializeRackSlots();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
}; 