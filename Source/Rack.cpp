#include "Rack.h"

Rack::Rack()
{
    // Set up rack container
    rackContainer.setSize(800, 1000);
    rackViewport.setViewedComponent(&rackContainer, false);
    addAndMakeVisible(rackViewport);
    
    // Initialize rack slots
    initializeRackSlots();
}

Rack::~Rack()
{
    slots.clear();
}

void Rack::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void Rack::resized()
{
    rackViewport.setBounds(getLocalBounds());
}

bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // We're interested in drag sources from the gear library or from other slots
    return true;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Highlight potential drop areas
    repaint();
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Update highlighting based on position
    repaint();
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Remove highlighting
    repaint();
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    auto* sourceComponent = dragSourceDetails.sourceComponent.get();
    
    // Find the nearest slot to the drop position
    auto* targetSlot = findNearestSlot(dragSourceDetails.localPosition);
    
    if (targetSlot != nullptr)
    {
        if (dynamic_cast<RackSlot*>(sourceComponent) != nullptr)
        {
            // Dragging from one slot to another (rearranging)
            auto* sourceSlot = dynamic_cast<RackSlot*>(sourceComponent);
            rearrangeGear(sourceSlot->getIndex(), targetSlot->getIndex());
        }
        else
        {
            // Dragging from gear library
            // The var should contain the gear item index in the library
            int gearIndex = (int)dragSourceDetails.description;
            
            // In a real implementation, we would get the GearItem from the library
            // For now, we'll just create a dummy item
            
            GearItem* newItem = new GearItem("Dummy Gear", "Manufacturer", 
                                            targetSlot->getType() == RackSlot::SlotType::Series500 ? 
                                            GearType::Series500 : GearType::Rack19Inch,
                                            GearCategory::Other, 1, "", {});
            
            addGearItem(newItem, targetSlot->getIndex());
        }
    }
    
    repaint();
}

void Rack::addGearItem(GearItem* gearItem, int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < slots.size())
    {
        auto* targetSlot = slots[slotIndex];
        
        if (targetSlot->isCompatibleWithGear(gearItem) && targetSlot->isAvailable())
        {
            targetSlot->setGearItem(gearItem);
        }
    }
}

void Rack::removeGearItem(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < slots.size())
    {
        auto* targetSlot = slots[slotIndex];
        targetSlot->clearGearItem();
    }
}

void Rack::rearrangeGear(int sourceSlotIndex, int destSlotIndex)
{
    if (sourceSlotIndex >= 0 && sourceSlotIndex < slots.size() &&
        destSlotIndex >= 0 && destSlotIndex < slots.size())
    {
        auto* sourceSlot = slots[sourceSlotIndex];
        auto* destSlot = slots[destSlotIndex];
        
        auto* gearItem = sourceSlot->getGearItem();
        
        if (gearItem != nullptr && destSlot->isCompatibleWithGear(gearItem) && destSlot->isAvailable())
        {
            sourceSlot->clearGearItem();
            destSlot->setGearItem(gearItem);
        }
    }
}

RackSlot* Rack::findNearestSlot(const juce::Point<int>& position)
{
    // Convert position to container coordinates
    auto containerPos = rackViewport.getViewPosition() + position;
    
    // Find the slot containing this position
    for (auto* slot : slots)
    {
        if (slot->getBounds().contains(containerPos))
            return slot;
    }
    
    // If no direct hit, find the nearest slot
    RackSlot* nearestSlot = nullptr;
    int nearestDistance = std::numeric_limits<int>::max();
    
    for (auto* slot : slots)
    {
        int distance = slot->getBounds().getCentre().getDistanceFrom(containerPos);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestSlot = slot;
        }
    }
    
    return nearestSlot;
}

void Rack::initializeRackSlots()
{
    // Clear existing slots
    slots.clear();
    
    // Constants for slot sizing
    const int series500Width = 150;
    const int series500Height = 200;
    const int rack19Width = 800;
    const int rack19UnitHeight = 44;  // 1U = 1.75 inches = ~44mm
    
    // Add 500-series slots (horizontal row at the top)
    int xPos = 0;
    int yPos = 20;
    for (int i = 0; i < series500SlotCount; ++i)
    {
        auto* slot = new RackSlot(RackSlot::SlotType::Series500, i);
        slot->setBounds(xPos, yPos, series500Width, series500Height);
        rackContainer.addAndMakeVisible(slot);
        slots.add(slot);
        
        xPos += series500Width + 5;  // 5px spacing
    }
    
    // Add 19" rack slots (vertical stack below 500-series)
    xPos = 0;
    yPos = series500Height + 50;  // 50px gap
    for (int i = 0; i < rack19SlotCount; ++i)
    {
        int slotSize = (i % 3) + 1;  // Alternate between 1U, 2U, and 3U
        
        auto* slot = new RackSlot(RackSlot::SlotType::Rack19Inch, i, slotSize);
        slot->setBounds(xPos, yPos, rack19Width, rack19UnitHeight * slotSize);
        rackContainer.addAndMakeVisible(slot);
        slots.add(slot);
        
        yPos += rack19UnitHeight * slotSize + 5;  // 5px spacing
    }
} 