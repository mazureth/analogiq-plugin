#pragma once

#include <JuceHeader.h>
#include "GearItem.h"
#include "RackSlot.h"
#include "GearLibrary.h"

class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    // Forward declaration of the container class
    class RackContainer : public juce::Component
    {
    public:
        // Pass mouse events to the parent Rack
        void mouseDown(const juce::MouseEvent& e) override
        {
            // Forward event to parent rack if it exists
            if (rack != nullptr)
                rack->mouseDown(e.getEventRelativeTo(rack));
        }
        
        void mouseDrag(const juce::MouseEvent& e) override
        {
            // Forward event to parent rack if it exists
            if (rack != nullptr)
                rack->mouseDrag(e.getEventRelativeTo(rack));
        }
        
        void mouseUp(const juce::MouseEvent& e) override
        {
            // Forward event to parent rack if it exists
            if (rack != nullptr)
                rack->mouseUp(e.getEventRelativeTo(rack));
        }
        
        // Pointer to parent Rack for event forwarding
        Rack* rack = nullptr;
    };
    
    Rack();
    ~Rack() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // DragAndDropTarget implementation
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    // Rack management
    void addGearItem(GearItem* gearItem, int slotIndex);
    void removeGearItem(int slotIndex);
    void rearrangeGear(int sourceSlotIndex, int destSlotIndex);
    void rearrangeGearAsSortableList(int sourceIndex, int targetIndex);
    void resetSlotPositions();
    
    // Slot finding
    RackSlot* findNearestSlot(const juce::Point<int>& position);

private:
    void initializeRackSlots();

    // Constants for slot sizing
    static constexpr int slotWidth = 180;
    static constexpr int slotHeight = 120;
    static constexpr int slotSpacing = 10;
    static constexpr int numSlots = 16;
    
    juce::OwnedArray<RackSlot> slots;
    RackContainer* rackContainer = nullptr;
    juce::Viewport* rackViewport = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
}; 