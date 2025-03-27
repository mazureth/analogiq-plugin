#include "Rack.h"
#include "GearLibrary.h"

Rack::Rack()
{
    // Set component ID for debugging
    setComponentID("Rack");
    
    // Set up rack container
    rackContainer.setSize(800, 1000);
    rackContainer.setComponentID("RackContainer");
    
    // CRITICAL FIX: Make the rack container also handle mouse events properly
    // This is essential since it's the actual parent of the slots
    rackContainer.setInterceptsMouseClicks(false, true);
    
    // CRITICAL FIX: Configure viewport to properly handle drag and drop
    rackViewport.setViewedComponent(&rackContainer, false);
    rackViewport.setInterceptsMouseClicks(false, true); // Let events pass through to container
    rackViewport.setComponentID("RackViewport");
    addAndMakeVisible(rackViewport);
    
    // Initialize rack slots
    initializeRackSlots();
    
    // Make sure the rack accepts drag and drop
    setInterceptsMouseClicks(false, true); // CHANGED FROM TRUE to FALSE - let events reach container
    
    // CRITICAL: Register the rack itself as a drag target
    setWantsKeyboardFocus(true);
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

bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails)
{
    DBG("Rack::isInterestedInDragSource called, description: " + sourceDetails.description.toString());
    
    // CRITICAL FIX: Always accept drags with integer data (row indices)
    if (sourceDetails.description.isInt())
    {
        DBG("Rack is interested in drag with integer data: " + sourceDetails.description.toString());
        return true;
    }
    
    // Check if the source is a DraggableListBox from the GearLibrary
    auto* sourceComp = sourceDetails.sourceComponent.get();
    if (sourceComp && sourceComp->getComponentID() == "DraggableListBox")
    {
        DBG("Interested in drag from DraggableListBox");
        return true;
    }
    
    // Check if it's a RackSlot (for rearranging)
    if (dynamic_cast<RackSlot*>(sourceComp) != nullptr)
    {
        DBG("Interested in drag from RackSlot");
        return true;
    }
    
    DBG("Not interested in this drag source: " + 
        (sourceComp ? sourceComp->getComponentID() : "unknown"));
        
    // For maximum compatibility, accept all drags during development
    return true;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("Rack::itemDragEnter called");
    
    // Highlight potential drop areas
    auto* nearestSlot = findNearestSlot(dragSourceDetails.localPosition);
    if (nearestSlot)
    {
        nearestSlot->setHighlighted(true);
    }
    
    repaint();
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("Rack::itemDragMove called at " + dragSourceDetails.localPosition.toString());
    
    // Clear all highlighting
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    // Highlight the slot under the cursor
    auto* nearestSlot = findNearestSlot(dragSourceDetails.localPosition);
    if (nearestSlot)
    {
        nearestSlot->setHighlighted(true);
    }
    
    repaint();
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("Rack::itemDragExit called");
    
    // Clear all highlighting
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    repaint();
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("Rack::itemDropped called");
    DBG("Drop position: " + dragSourceDetails.localPosition.toString());
    DBG("Description: " + dragSourceDetails.description.toString());
    
    // Clear all highlighting
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    auto* sourceComponent = dragSourceDetails.sourceComponent.get();
    DBG("Source component: " + (sourceComponent != nullptr ? sourceComponent->getComponentID() : "nullptr"));
    
    // Find the nearest slot to the drop position
    auto* targetSlot = findNearestSlot(dragSourceDetails.localPosition);
    
    if (targetSlot != nullptr)
    {
        DBG("Target slot found: " + juce::String(targetSlot->getIndex()));
        
        if (dynamic_cast<RackSlot*>(sourceComponent) != nullptr)
        {
            // Dragging from one slot to another (rearranging)
            DBG("Rearranging from slot to slot");
            auto* sourceSlot = dynamic_cast<RackSlot*>(sourceComponent);
            rearrangeGear(sourceSlot->getIndex(), targetSlot->getIndex());
        }
        else if (sourceComponent != nullptr && 
                (sourceComponent->getComponentID() == "DraggableListBox"))
        {
            // Dragging from gear library
            DBG("Dragging from DraggableListBox");
            
            // The var should contain the gear item index in the library
            int gearIndex = -1;
            
            if (dragSourceDetails.description.isInt())
            {
                gearIndex = (int)dragSourceDetails.description;
                DBG("Gear index from drag description: " + juce::String(gearIndex));
            }
            else
            {
                DBG("Invalid drag description. Expected integer, got: " + 
                    dragSourceDetails.description.toString());
                return;
            }
            
            // Find the GearLibrary component in the component hierarchy
            GearLibrary* gearLibrary = nullptr;
            
            // Start with our parent component and search upward
            juce::Component* parent = getParentComponent();
            while (parent != nullptr && gearLibrary == nullptr)
            {
                // Try all siblings of this parent
                for (int i = 0; i < parent->getNumChildComponents(); ++i)
                {
                    auto* child = parent->getChildComponent(i);
                    if (child->getComponentID() == "GearLibrary")
                    {
                        gearLibrary = dynamic_cast<GearLibrary*>(child);
                        if (gearLibrary != nullptr)
                        {
                            DBG("Found GearLibrary at component: " + child->getComponentID());
                            break;
                        }
                    }
                }
                
                parent = parent->getParentComponent();
            }
            
            if (gearLibrary != nullptr && gearIndex >= 0)
            {
                DBG("Getting gear item from library");
                // Get the actual gear item from the library
                GearItem* originalItem = gearLibrary->getGearItem(gearIndex);
                
                if (originalItem != nullptr)
                {
                    DBG("Creating new gear item: " + originalItem->name);
                    // Create a new copy of the item for the rack
                    GearItem* newItem = new GearItem(
                        originalItem->name,
                        originalItem->manufacturer,
                        originalItem->type,
                        originalItem->category,
                        originalItem->slotSize,
                        originalItem->imageUrl,
                        originalItem->controls
                    );
                    
                    if (targetSlot->isCompatibleWithGear(newItem))
                    {
                        DBG("Adding gear to slot " + juce::String(targetSlot->getIndex()));
                        addGearItem(newItem, targetSlot->getIndex());
                    }
                    else
                    {
                        DBG("Gear incompatible with slot");
                        delete newItem; // Clean up if not compatible
                    }
                }
                else
                {
                    DBG("Original item not found in library");
                }
            }
            else
            {
                DBG("GearLibrary not found or invalid gear index: " + juce::String(gearIndex));
            }
        }
        else
        {
            DBG("Source component is neither RackSlot nor DraggableListBox: " + 
                (sourceComponent ? sourceComponent->getComponentID() : "nullptr"));
        }
    }
    else
    {
        DBG("No target slot found");
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
    // Debug output
    DBG("Looking for slot at position: " + position.toString());
    
    // FIXED COORDINATE TRANSFORMATION:
    // 1. Convert global position to local rack coordinates
    auto rackRelativePos = getLocalPoint(nullptr, position);
    
    // 2. Then convert to the container's coordinate system, accounting for viewport's scroll position
    auto containerPos = rackContainer.getLocalPoint(this, rackRelativePos) + rackViewport.getViewPosition();
    
    DBG("Transformed coordinates: global -> " + position.toString() + 
        ", rack local -> " + rackRelativePos.toString() + 
        ", container -> " + containerPos.toString());
    
    // First check for direct hit
    for (auto* slot : slots)
    {
        if (slot->getBounds().contains(containerPos))
        {
            DBG("Direct hit on slot at index: " + juce::String(slot->getIndex()));
            return slot;
        }
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
    
    if (nearestSlot != nullptr)
    {
        DBG("Nearest slot at index: " + juce::String(nearestSlot->getIndex()) + 
            " distance: " + juce::String(nearestDistance));
    }
    else
    {
        DBG("No nearest slot found");
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