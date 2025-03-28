#include "Rack.h"
#include "GearLibrary.h"

Rack::Rack()
{
    DBG("Rack constructor");
    setComponentID("Rack");
    
    // Create viewport and container
    rackViewport = std::make_unique<juce::Viewport>();
    rackContainer = std::make_unique<RackContainer>();
    rackViewport->setViewedComponent(rackContainer.get(), false); // false = don't delete when viewport is deleted
    addAndMakeVisible(rackViewport.get());
    
    // Set up the container
    rackContainer->rack = this;
    
    // Create rack slots
    DBG("Creating " + juce::String(numSlots) + " rack slots");
    for (int i = 0; i < numSlots; ++i)
    {
        RackSlot* newSlot = new RackSlot(i);
        slots.add(newSlot);
        rackContainer->addAndMakeVisible(newSlot);
    }
    
    // Set up this component as a drag-and-drop target
    setInterceptsMouseClicks(true, true);
}

Rack::~Rack()
{
    DBG("Rack destructor");
    // unique_ptr members will be automatically deleted
}

void Rack::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void Rack::resized()
{
    DBG("Rack::resized");
    
    // Size the viewport to fill our area
    auto area = getLocalBounds();
    rackViewport->setBounds(area);
    
    // Get the available width from the viewport
    const int availableWidth = rackViewport->getWidth();
    
    // Size the container to fit all slots with spacing
    const int containerHeight = numSlots * (slotHeight + slotSpacing) + slotSpacing;
    const int containerWidth = availableWidth; // Use full viewport width
    rackContainer->setSize(containerWidth, containerHeight);
    
    // Calculate the slot width based on container width minus margins
    const int effectiveSlotWidth = containerWidth - (2 * slotSpacing);
    
    // Position the slots within the container
    for (int i = 0; i < slots.size(); ++i)
    {
        auto* slot = slots[i];
        slot->setBounds(
            slotSpacing,
            slotSpacing + i * (slotHeight + slotSpacing),
            effectiveSlotWidth,
            slotHeight
        );
    }
    
    DBG("Rack resized: viewport=" + rackViewport->getBounds().toString() +
        ", container=" + rackContainer->getBounds().toString());
}

bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Accept drops from DraggableListBox (GearLibrary) or other RackSlots
    auto* sourceComp = dragSourceDetails.sourceComponent.get();
    
    if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" || 
                       sourceComp->getComponentID() == "GearListBox" ||
                       dynamic_cast<RackSlot*>(sourceComp) != nullptr))
    {
        return true;
    }
    
    return false;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& /*dragSourceDetails*/)
{
    // Nothing to do here
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails& details)
{
    // This is only used for dragging from GearLibrary now, not for reordering
    RackSlot* nearestSlot = findNearestSlot(details.localPosition);
    
    // Highlight the nearest slot
    for (auto* slot : slots)
    {
        slot->setHighlighted(slot == nearestSlot);
    }
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*dragSourceDetails*/)
{
    // Clear all highlights
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    DBG("Rack::itemDropped");
    
    // Find the nearest slot to the drop position
    RackSlot* targetSlot = findNearestSlot(details.localPosition);
    if (targetSlot == nullptr)
    {
        DBG("No target slot found for drop position");
        return;
    }
    
    // Clear highlights
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    // Handle drops from GearLibrary only (not for rearranging items)
    if (details.description.isInt())
    {
        auto* sourceComp = details.sourceComponent.get();
        if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" || 
                         sourceComp->getComponentID() == "GearListBox"))
        {
            // This is a drop from the GearLibrary
            int gearIndex = details.description.toString().getIntValue();
            DBG("Drop from GearLibrary, gear index: " + juce::String(gearIndex));
            
            // Check if gearLibrary is valid before accessing it
            if (gearLibrary == nullptr)
            {
                DBG("ERROR: gearLibrary is null, cannot get gear item");
                return;
            }
            
            // Get the gear item from the library
            GearItem* gearItem = gearLibrary->getGearItem(gearIndex);
            if (gearItem != nullptr)
            {
                // If the target slot already has an item, clear it first
                targetSlot->clearGearItem();
                
                // Set the new gear item in the slot
                targetSlot->setGearItem(gearItem);
                DBG("Set gear item " + gearItem->getName() + " in slot " + juce::String(targetSlot->getIndex()));
            }
            else
            {
                DBG("Could not find gear item with index " + juce::String(gearIndex));
            }
        }
    }
}

void Rack::rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex)
{
    DBG("===============================================");
    DBG("Rack::rearrangeGearAsSortableList - sourceIndex: " + juce::String(sourceSlotIndex) + 
        ", targetIndex: " + juce::String(targetSlotIndex));
    
    // Validate indices
    if (sourceSlotIndex < 0 || sourceSlotIndex >= slots.size() ||
        targetSlotIndex < 0 || targetSlotIndex >= slots.size() ||
        sourceSlotIndex == targetSlotIndex)
    {
        DBG("Invalid source or target index, or they are the same. Aborting rearrangement.");
        return;
    }
    
    // Get pointers to the source and target slots
    RackSlot* sourceSlot = slots[sourceSlotIndex];
    RackSlot* targetSlot = slots[targetSlotIndex];
    
    if (sourceSlot == nullptr || targetSlot == nullptr)
    {
        DBG("Source or target slot is null. Aborting rearrangement.");
        return;
    }
    
    // Get the gear items from both slots
    GearItem* sourceGearItem = sourceSlot->getGearItem();
    GearItem* targetGearItem = targetSlot->getGearItem();
    
    if (sourceGearItem == nullptr)
    {
        DBG("Source gear item is null. Cannot move an empty slot. Aborting rearrangement.");
        return;
    }
    
    // Simply swap the two slots
    DBG("Swapping gear items between slot " + juce::String(sourceSlotIndex) + 
        " and slot " + juce::String(targetSlotIndex));
    
    // First clear both slots
    sourceSlot->clearGearItem();
    targetSlot->clearGearItem();
    
    // Then set the items in their new positions
    targetSlot->setGearItem(sourceGearItem);
    
    // If the target slot had an item, move it to the source slot
    if (targetGearItem != nullptr)
    {
        sourceSlot->setGearItem(targetGearItem);
    }
    
    // Update the rack view
    rackContainer->resized();
    
    DBG("Gear items successfully swapped.");
    DBG("===============================================");
}

RackSlot* Rack::findNearestSlot(const juce::Point<int>& position)
{
    // This method is kept for compatibility with drops from GearLibrary
    // but we've simplified it since we're not using it for reordering
    
    // Convert global position to container coordinates
    auto containerPos = rackContainer->getLocalPoint(this, position);
    
    // First check for a direct hit
    for (auto* slot : slots)
    {
        if (slot->getBounds().contains(containerPos))
        {
            return slot;
        }
    }
    
    // If no direct hit, find the closest slot
    RackSlot* bestSlot = nullptr;
    int bestDistance = std::numeric_limits<int>::max();
    
    for (auto* slot : slots)
    {
        juce::Rectangle<int> slotBounds = slot->getBounds();
        int distance = slotBounds.getCentre().getDistanceFrom(containerPos);
        
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestSlot = slot;
        }
    }
    
    return bestSlot;
} 