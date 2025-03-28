#pragma once

#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"

class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    Rack();
    ~Rack() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // DragAndDropTarget methods
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    // Swap items between two slots (for button-based reordering)
    void rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex);
    
    // Find the nearest slot to a position (for drops from library)
    RackSlot* findNearestSlot(const juce::Point<int>& position);
    
    // Get the number of slots
    int getNumSlots() const { return slots.size(); }
    
    // Set the gear library reference
    void setGearLibrary(GearLibrary* library) { gearLibrary = library; }
    
    // Internal container class for rack slots
    class RackContainer : public juce::Component
    {
    public:
        RackContainer() { setComponentID("RackContainer"); }
        void paint(juce::Graphics& g) override { g.fillAll(juce::Colours::black); }
        Rack* rack = nullptr;
    };
    
private:
    // Configuration
    int numSlots = 16;
    int slotHeight = 150;
    int slotWidth = 300;
    int slotSpacing = 10;
    
    // UI Components
    std::unique_ptr<juce::Viewport> rackViewport;
    std::unique_ptr<RackContainer> rackContainer;
    juce::OwnedArray<RackSlot> slots;
    
    // Reference to the gear library (for drag and drop)
    GearLibrary* gearLibrary = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
}; 