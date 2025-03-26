#include "RackSlot.h"

RackSlot::RackSlot(SlotType type, int index, int size)
    : type(type), index(index), size(size)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

RackSlot::~RackSlot()
{
}

void RackSlot::paint(juce::Graphics& g)
{
    // Draw background
    if (isAvailable())
    {
        // Empty slot
        g.setColour(juce::Colours::darkgrey.darker(0.3f));
        g.fillRect(getLocalBounds());
        
        // Border
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
        
        // Slot identifier
        g.setColour(juce::Colours::lightgrey);
        g.setFont(14.0f);
        
        juce::String slotText;
        if (type == SlotType::Series500)
            slotText = "500 Series #" + juce::String(index + 1);
        else
            slotText = "19\" Rack " + juce::String(size) + "U #" + juce::String(index + 1);
            
        g.drawText(slotText, getLocalBounds().reduced(5), juce::Justification::centred, true);
    }
    else if (gearItem != nullptr)
    {
        // Slot with gear in it
        if (gearItem->image.isNull())
            gearItem->loadImage();
            
        // Draw gear image
        g.drawImage(gearItem->image, getLocalBounds().toFloat(), 
                    juce::RectanglePlacement::centred);
                    
        // Draw name at the bottom
        g.setColour(juce::Colours::black);
        g.fillRect(getLocalBounds().removeFromBottom(20));
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(gearItem->name, getLocalBounds().removeFromBottom(20).reduced(2, 0),
                   juce::Justification::centred, true);
    }
}

void RackSlot::resized()
{
}

void RackSlot::mouseDown(const juce::MouseEvent& e)
{
    if (gearItem != nullptr)
    {
        dragMode = true;
        dragger.startDraggingComponent(this, e);
    }
}

void RackSlot::mouseDrag(const juce::MouseEvent& e)
{
    if (dragMode && gearItem != nullptr)
    {
        // Start a drag-and-drop operation
        juce::DragAndDropContainer* dragContainer = juce::DynamicCast<juce::DragAndDropContainer>(getParentComponent());
        
        if (dragContainer != nullptr)
        {
            juce::var dragData(index);
            dragContainer->startDragging(dragData, this, juce::ScaledImage(),
                                      true, nullptr, juce::Point<int>(-10, -10));
            dragMode = false;
        }
    }
}

bool RackSlot::isCompatibleWithGear(const GearItem* gearItem) const
{
    if (gearItem == nullptr)
        return false;
        
    // Check type compatibility
    bool typeMatch = (type == SlotType::Series500 && gearItem->type == GearType::Series500) ||
                     (type == SlotType::Rack19Inch && gearItem->type == GearType::Rack19Inch);
                     
    // Check size compatibility
    bool sizeMatch = gearItem->slotSize <= size;
    
    return typeMatch && sizeMatch;
}

void RackSlot::setGearItem(GearItem* newGearItem)
{
    gearItem = newGearItem;
    repaint();
} 