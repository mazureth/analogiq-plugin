#pragma once

#include <JuceHeader.h>
#include "GearItem.h"

class RackSlot : public juce::Component
{
public:
    enum class SlotType
    {
        Series500,
        Rack19Inch
    };

    RackSlot(SlotType type, int index, int size = 1);
    ~RackSlot() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    bool isCompatibleWithGear(const GearItem* gearItem) const;
    bool isAvailable() const { return gearItem == nullptr; }
    
    void setGearItem(GearItem* newGearItem);
    GearItem* getGearItem() const { return gearItem; }
    void clearGearItem() { gearItem = nullptr; repaint(); }
    
    SlotType getType() const { return type; }
    int getIndex() const { return index; }
    int getSize() const { return size; }

private:
    SlotType type;
    int index;
    int size;  // 1 for standard 500-series, or 1-4 for 19" racks (in 1U increments)
    GearItem* gearItem = nullptr;
    
    juce::ComponentDragger dragger;
    bool dragMode = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackSlot)
}; 