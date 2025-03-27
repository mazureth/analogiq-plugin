#include "Rack.h"
#include "GearLibrary.h"

Rack::Rack()
{
    // Set component ID for debugging
    setComponentID("Rack");
    
    // Create a viewport and container for the rack slots
    rackContainer = new RackContainer();
    rackContainer->setComponentID("RackContainer");
    rackViewport = new juce::Viewport();
    rackViewport->setViewedComponent(rackContainer, false); // false = don't delete when viewport is deleted
    addAndMakeVisible(rackViewport);
    
    // Set up drag-and-drop
    setInterceptsMouseClicks(true, true);
    
    // Important: Make sure both the viewport and container are set up to 
    // allow drag and drop events to propagate through to the Rack
    rackViewport->setInterceptsMouseClicks(false, true);
    rackContainer->setInterceptsMouseClicks(false, true);
    
    // We must add the rack container as a child of the viewport
    // but still keep the rack as the DragAndDropTarget
    rackContainer->rack = this;
    
    // Initialize rack slots in the rack container
    initializeRackSlots();
}

Rack::~Rack()
{
    // Clear the slots array first
    slots.clear();
    
    // Clean up container and viewport
    if (rackViewport != nullptr)
    {
        rackViewport->setViewedComponent(nullptr, false);
        delete rackViewport;
        rackViewport = nullptr;
    }
    
    if (rackContainer != nullptr)
    {
        delete rackContainer;
        rackContainer = nullptr;
    }
}

void Rack::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void Rack::resized()
{
    DBG("Rack::resized");
    
    // Make viewport fill the entire component
    rackViewport->setBounds(getLocalBounds());
    
    // Make sure container is the right width
    rackContainer->setSize(getWidth() - 20, rackContainer->getHeight());
    
    // If slots exist, reposition them to match the new width
    if (!slots.isEmpty())
    {
        int availableWidth = rackContainer->getWidth() - 20; // 20px margin
        
        for (int i = 0; i < slots.size(); ++i)
        {
            auto* slot = slots[i];
            // Update width but keep y position
            juce::Rectangle<int> bounds = slot->getBounds();
            bounds.setWidth(availableWidth);
            slot->setBounds(bounds);
        }
    }
}

bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails)
{
    DBG("Rack::isInterestedInDragSource called, description: " + sourceDetails.description.toString());
    
    // Accept drags with integer data (row indices)
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
    
    return false;
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
    
    // Find the nearest slot to the current drag position
    auto* targetSlot = findNearestSlot(dragSourceDetails.localPosition);
    if (targetSlot == nullptr)
        return;
        
    int targetIndex = targetSlot->getIndex();
    
    // Clear all highlighting first
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    // Highlight the target slot
    targetSlot->setHighlighted(true);
    
    // Determine if we're dragging from another slot
    RackSlot* sourceSlot = dynamic_cast<RackSlot*>(dragSourceDetails.sourceComponent.get());
    bool dragFromLibrary = false;
    
    // If not from a slot, check if it's from the gear library
    if (sourceSlot == nullptr) 
    {
        auto* sourceComp = dragSourceDetails.sourceComponent.get();
        dragFromLibrary = sourceComp && (sourceComp->getComponentID() == "DraggableListBox" || 
                                          sourceComp->getComponentID() == "GearListBox");
    }
    
    // Calculate insertion position for preview
    auto insertionY = targetSlot->getBounds().getY();
    
    // Only do preview animation for slot-to-slot drag or library drag
    if ((sourceSlot != nullptr && sourceSlot != targetSlot) || dragFromLibrary)
    {
        int sourceIndex = sourceSlot ? sourceSlot->getIndex() : -1;
        
        // Create visual feedback by shifting slot positions temporarily
        juce::Array<juce::Rectangle<int>> originalBounds;
        juce::Array<juce::Rectangle<int>> targetBounds;
        
        for (auto* slot : slots)
        {
            originalBounds.add(slot->getBounds());
            targetBounds.add(slot->getBounds());
        }
        
        // For library drag, we always insert at target position and shift down
        if (dragFromLibrary)
        {
            // If we're dropping from the library, simulate shifting all slots below the target down
            for (int i = slots.size() - 1; i > targetIndex; i--)
            {
                if (!slots[i]->isAvailable() || !slots[i-1]->isAvailable())
                {
                    // Only shift occupied slots or ones that would receive items
                    targetBounds.getReference(i).setY(originalBounds[i-1].getY() + slotHeight + slotSpacing);
                }
            }
        }
        // For slot-to-slot drag, simulate proper reordering
        else if (sourceIndex >= 0)
        {
            if (sourceIndex < targetIndex)
            {
                // Moving downward
                for (int i = sourceIndex + 1; i <= targetIndex; i++)
                {
                    // Move slots up
                    targetBounds.getReference(i).setY(originalBounds[i - 1].getY());
                }
            }
            else // sourceIndex > targetIndex
            {
                // Moving upward
                for (int i = targetIndex; i < sourceIndex; i++)
                {
                    // Move slots down
                    targetBounds.getReference(i).setY(originalBounds[i + 1].getY());
                }
            }
            
            // Set the dragged slot position to the target position
            targetBounds.getReference(sourceIndex).setY(insertionY);
        }
        
        // Apply the new positions to create visual animation
        for (int i = 0; i < slots.size(); i++)
        {
            // Don't move the source slot if it's being dragged
            if (sourceSlot == nullptr || i != sourceIndex)
            {
                slots[i]->setBounds(targetBounds[i]);
            }
        }
    }
    
    repaint();
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*dragSourceDetails*/)
{
    DBG("Rack::itemDragExit called");
    
    // Clear all highlighting
    for (auto* slot : slots)
    {
        slot->setHighlighted(false);
    }
    
    // Reset slot positions
    resetSlotPositions();
    
    repaint();
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("Rack::itemDropped called, description=" + dragSourceDetails.description.toString());
    
    // Find which slot the drop occurred in
    RackSlot* targetSlot = findNearestSlot(dragSourceDetails.localPosition);
    if (targetSlot == nullptr)
    {
        DBG("No target slot found for drop");
        resetSlotPositions();
        return;
    }
    
    const int targetIndex = targetSlot->getIndex();
    DBG("Drop target slot index: " + juce::String(targetIndex));
    
    // Check if it's a rearrangement from another slot
    auto* sourceComponent = dragSourceDetails.sourceComponent.get();
    RackSlot* sourceSlot = dynamic_cast<RackSlot*>(sourceComponent);
    
    if (sourceSlot != nullptr)
    {
        const int sourceIndex = sourceSlot->getIndex();
        DBG("Rearranging from slot " + juce::String(sourceIndex) + " to slot " + juce::String(targetIndex));
        
        if (sourceIndex != targetIndex)
        {
            rearrangeGearAsSortableList(sourceIndex, targetIndex);
        }
        else
        {
            DBG("Source and target slots are the same, no rearrangement needed");
        }
    }
    else
    {
        // Handle drops from GearLibrary or other sources
        DBG("Handling drop from Gear Library");
        
        if (dragSourceDetails.description.isInt())
        {
            int gearIndex = static_cast<int>(dragSourceDetails.description);
            DBG("Dropped gear index: " + juce::String(gearIndex));
            
            // Find the GearLibrary component
            GearLibrary* gearLibrary = nullptr;
            juce::Component* topLevel = getTopLevelComponent();
            
            if (topLevel != nullptr)
            {
                // Search for GearLibrary in the component hierarchy
                std::function<void(juce::Component*)> findLibrary = 
                    [&gearLibrary, &findLibrary](juce::Component* comp) {
                        if (comp->getComponentID() == "GearLibrary")
                        {
                            gearLibrary = dynamic_cast<GearLibrary*>(comp);
                            return;
                        }
                        
                        for (int i = 0; i < comp->getNumChildComponents(); ++i)
                        {
                            if (gearLibrary == nullptr)
                                findLibrary(comp->getChildComponent(i));
                        }
                    };
                
                findLibrary(topLevel);
            }
            
            if (gearLibrary != nullptr)
            {
                DBG("Found GearLibrary, getting gear item at index: " + juce::String(gearIndex));
                GearItem* originalItem = gearLibrary->getGearItem(gearIndex);
                
                if (originalItem != nullptr)
                {
                    DBG("Found gear item: " + originalItem->name);
                    
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
                    
                    // If we dropped on an occupied slot, make room first
                    if (!targetSlot->isAvailable())
                    {
                        // Find the first empty slot from end to beginning
                        int firstEmptyIndex = -1;
                        for (int i = slots.size() - 1; i >= 0; i--)
                        {
                            if (slots[i]->isAvailable())
                            {
                                firstEmptyIndex = i;
                                break;
                            }
                        }
                        
                        if (firstEmptyIndex >= 0)
                        {
                            // Shift items down to make room
                            for (int i = firstEmptyIndex; i > targetIndex; i--)
                            {
                                auto* currentItem = slots[i-1]->getGearItem();
                                if (currentItem != nullptr)
                                {
                                    slots[i-1]->clearGearItem();
                                    slots[i]->setGearItem(currentItem);
                                }
                            }
                        }
                        else
                        {
                            // No empty slots
                            DBG("No empty slots available, cannot add gear");
                            delete newItem;
                            resetSlotPositions();
                            return;
                        }
                    }
                    
                    // Add the new item to the target slot
                    targetSlot->setGearItem(newItem);
                    DBG("Added new gear item to slot " + juce::String(targetIndex));
                }
                else
                {
                    DBG("Could not find gear item at index " + juce::String(gearIndex));
                }
            }
            else
            {
                DBG("Could not find GearLibrary component");
            }
        }
        else
        {
            DBG("Invalid drag description: " + dragSourceDetails.description.toString());
        }
    }
    
    // Make sure all slots are back in their proper positions
    resetSlotPositions();
    repaint();
}

// Helper function to reset slot positions to their proper layout
void Rack::resetSlotPositions()
{
    int yPos = 20;
    int xPos = 20;
    int availableWidth = rackContainer->getWidth() - (xPos * 2);
    
    for (int i = 0; i < slots.size(); ++i)
    {
        auto* slot = slots[i];
        slot->setBounds(xPos, yPos, availableWidth, slotHeight);
        yPos += slotHeight + slotSpacing;
    }
}

// New method to handle sortable list behavior
void Rack::rearrangeGearAsSortableList(int sourceIndex, int targetIndex)
{
    DBG("Rack::rearrangeGearAsSortableList from " + juce::String(sourceIndex) + " to " + juce::String(targetIndex));
    
    if (sourceIndex < 0 || sourceIndex >= slots.size() || 
        targetIndex < 0 || targetIndex >= slots.size())
    {
        DBG("Invalid source or target index");
        return;
    }
    
    // Get the source item
    auto* sourceSlot = slots[sourceIndex];
    auto* itemToMove = sourceSlot->getGearItem();
    
    if (itemToMove == nullptr)
    {
        DBG("No item to move from source slot");
        return;
    }
    
    DBG("Moving " + itemToMove->getName() + " from slot " + juce::String(sourceIndex) + 
        " to slot " + juce::String(targetIndex));
    
    // First remove the item from its source slot (but keep the pointer)
    sourceSlot->clearGearItem();
    
    // If we're moving an item to a slot that already has an item, we need to shift items
    if (sourceIndex < targetIndex)
    {
        // Moving down: shift items up
        for (int i = sourceIndex + 1; i <= targetIndex; i++)
        {
            auto* currentSlot = slots[i];
            auto* nextSlot = slots[i - 1];
            
            if (!currentSlot->isAvailable())
            {
                auto* item = currentSlot->getGearItem();
                currentSlot->clearGearItem();
                nextSlot->setGearItem(item);
                DBG("Shifted item from slot " + juce::String(i) + " to slot " + juce::String(i - 1));
            }
        }
    }
    else // sourceIndex > targetIndex
    {
        // Moving up: shift items down
        for (int i = sourceIndex - 1; i >= targetIndex; i--)
        {
            auto* currentSlot = slots[i];
            auto* nextSlot = slots[i + 1];
            
            if (!currentSlot->isAvailable())
            {
                auto* item = currentSlot->getGearItem();
                currentSlot->clearGearItem();
                nextSlot->setGearItem(item);
                DBG("Shifted item from slot " + juce::String(i) + " to slot " + juce::String(i + 1));
            }
        }
    }
    
    // Finally, place the moved item in the target slot
    slots[targetIndex]->setGearItem(itemToMove);
    DBG("Successfully moved item to target slot " + juce::String(targetIndex));
}

void Rack::addGearItem(GearItem* gearItem, int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < slots.size())
    {
        auto* targetSlot = slots[slotIndex];
        
        if (targetSlot->isAvailable())
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
        
        if (gearItem != nullptr && destSlot->isAvailable())
        {
            sourceSlot->clearGearItem();
            destSlot->setGearItem(gearItem);
        }
    }
}

RackSlot* Rack::findNearestSlot(const juce::Point<int>& position)
{
    // Convert global position to local rack coordinates
    auto rackRelativePos = getLocalPoint(nullptr, position);
    
    // Then convert to the container's coordinate system, accounting for viewport's scroll position
    auto containerPos = rackContainer->getLocalPoint(this, rackRelativePos) + rackViewport->getViewPosition();
    
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
    
    // Set fixed horizontal position with padding on both sides
    int xPos = 20;
    int yPos = 20;
    
    // Calculate the slot width to fill most of the container width
    int availableWidth = rackContainer->getWidth() - (xPos * 2);
    
    for (int i = 0; i < numSlots; ++i)
    {
        auto* slot = new RackSlot(i);
        // Make slots span most of the container width
        slot->setBounds(xPos, yPos, availableWidth, slotHeight);
        rackContainer->addAndMakeVisible(slot);
        slots.add(slot);
        
        // Always move to the next row
        yPos += slotHeight + slotSpacing;
    }
    
    // Update container height to fit all slots
    int totalHeight = (slotHeight + slotSpacing) * numSlots + 20;
    rackContainer->setSize(rackContainer->getWidth(), totalHeight);
} 