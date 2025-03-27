#include "RackSlot.h"
#include "GearLibrary.h"
#include "Rack.h"

RackSlot::RackSlot(int slotIndex)
    : index(slotIndex), highlighted(false), isDragging(false)
{
    setComponentID("RackSlot_" + juce::String(index));
    setInterceptsMouseClicks(true, true);
    DBG("Created RackSlot with index " + juce::String(index));
}

void RackSlot::paint(juce::Graphics& g)
{
    // Draw background
    juce::Colour bgColor = isAvailable() ? juce::Colours::darkgrey : juce::Colours::darkslategrey;
    g.fillAll(bgColor);
    
    // Draw border (highlighted if being dragged over)
    juce::Colour borderColor = highlighted ? juce::Colours::orange : juce::Colours::grey;
    g.setColour(borderColor);
    g.drawRect(getLocalBounds(), 2);
    
    // Draw slot number
    g.setColour(juce::Colours::white);
    g.drawText(juce::String(index), getLocalBounds().reduced(5, 5).removeFromTop(20), 
               juce::Justification::topLeft, true);
    
    // If we have a gear item, draw its name and image
    if (!isAvailable() && gearItem != nullptr)
    {
        // Draw name
        g.setFont(16.0f);
        g.setColour(juce::Colours::white);
        juce::Rectangle<int> nameArea = getLocalBounds().reduced(10, 10);
        g.drawText(gearItem->name, nameArea, juce::Justification::centred, true);
        
        // Draw manufacturer below name
        g.setFont(12.0f);
        g.setColour(juce::Colours::lightgrey);
        juce::Rectangle<int> mfgArea = nameArea.translated(0, 20);
        g.drawText(gearItem->manufacturer, mfgArea, juce::Justification::centred, true);
        
        // Draw image if available
        if (gearItem->image.isValid())
        {
            juce::Rectangle<int> imageArea = getLocalBounds().reduced(20);
            g.drawImageWithin(gearItem->image, imageArea.getX(), imageArea.getY() + 40, 
                             imageArea.getWidth(), imageArea.getHeight() - 40, 
                             juce::RectanglePlacement::centred);
        }
    }
    else
    {
        // Draw "empty slot" text for available slots
        g.setColour(juce::Colours::lightgrey);
        g.setFont(14.0f);
        g.drawText("Empty Slot", getLocalBounds(), juce::Justification::centred, true);
    }
}

void RackSlot::resized()
{
    // Nothing to do here - we draw everything in paint()
}

void RackSlot::mouseDown(const juce::MouseEvent& e)
{
    // Only start drag if this slot has gear
    if (!isAvailable() && gearItem != nullptr)
    {
        DBG("RackSlot::mouseDown - has gear item - preparing for drag");
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        dragger.startDraggingComponent(this, e);
    }
}

void RackSlot::mouseDrag(const juce::MouseEvent& e)
{
    // Only start drag if we have gear and have moved enough
    if (!isAvailable() && gearItem != nullptr && !isDragging && e.getDistanceFromDragStart() > 5)
    {
        isDragging = true;
        DBG("RackSlot::mouseDrag - starting drag for slot " + juce::String(index));
        
        juce::DragAndDropContainer* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this);
        if (dragContainer != nullptr)
        {
            // Create a thumbnail image of this component for the drag
            juce::Image dragImage = createComponentSnapshot(getLocalBounds(), true);
            
            // Center drag image
            juce::Point<int> dragOffset = dragImage.getBounds().getCentre();
            
            // Use the correct startDragging method with correctly typed parameters
            dragContainer->startDragging(
                juce::var(index),    // Description (slot index) 
                this,                // Source component
                juce::ScaledImage(dragImage),  // Image to display during drag (wrapped in ScaledImage)
                true,                // Allow dragging to other windows
                &dragOffset          // Offset for the drag image
            );
            
            DBG("RackSlot::mouseDrag - drag started successfully for slot " + juce::String(index));
            
            // Bring this slot to the front visually during drag
            toFront(true);
        }
        else
        {
            DBG("RackSlot::mouseDrag - NO DRAG CONTAINER FOUND!");
        }
    }
    
    // Update component position during drag for visual feedback
    if (isDragging)
    {
        // Only allow vertical movement by keeping X the same
        auto originalBounds = getBounds();
        dragger.dragComponent(this, e, nullptr);
        
        // Restrict to vertical movement only
        auto newBounds = getBounds();
        setBounds(originalBounds.getX(), newBounds.getY(), originalBounds.getWidth(), originalBounds.getHeight());
    }
}

void RackSlot::mouseUp(const juce::MouseEvent&)
{
    DBG("RackSlot::mouseUp");
    isDragging = false;
    dragSourceDetails.reset();
    setMouseCursor(juce::MouseCursor::NormalCursor);
    
    // When drag ends, let the parent Rack handle final positioning
    juce::Component* parentComponent = findParentRackComponent();
    Rack* parentRack = dynamic_cast<Rack*>(parentComponent);
    if (parentRack != nullptr)
    {
        parentRack->resetSlotPositions();
    }
    else
    {
        auto* container = dynamic_cast<Rack::RackContainer*>(parentComponent);
        if (container != nullptr && container->rack != nullptr)
        {
            container->rack->resetSlotPositions();
        }
    }
}

bool RackSlot::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails)
{
    DBG("RackSlot::isInterestedInDragSource called on slot " + juce::String(index) + 
        ", description=" + sourceDetails.description.toString());
    
    // Accept drag sources with integer data (row indices)
    if (sourceDetails.description.isInt())
    {
        DBG("RackSlot is interested in drag with integer data: " + sourceDetails.description.toString());
        return true;
    }
    
    // Get source component
    auto* sourceComp = sourceDetails.sourceComponent.get();
    
    if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" || 
                       sourceComp->getComponentID() == "GearListBox"))
    {
        DBG("RackSlot is interested in drag from DraggableListBox/GearListBox");
        return true;
    }
    
    // Check if it's another RackSlot (for rearranging)
    if (dynamic_cast<RackSlot*>(sourceComp) != nullptr)
    {
        DBG("RackSlot is interested in drag from another RackSlot");
        return true;
    }
    
    return false;
}

void RackSlot::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    DBG("RackSlot::itemDragEnter on slot " + juce::String(index));
    setHighlighted(true);
}

void RackSlot::itemDragMove(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    DBG("RackSlot::itemDragMove on slot " + juce::String(index));
}

void RackSlot::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    DBG("RackSlot::itemDragExit on slot " + juce::String(index));
    setHighlighted(false);
}

void RackSlot::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    DBG("RackSlot::itemDropped on slot " + juce::String(index) + 
        ", description=" + details.description.toString());
    setHighlighted(false);
    
    // Always try to delegate to the parent Rack for handling
    juce::Component* parentComponent = findParentRackComponent();
    
    if (parentComponent != nullptr)
    {
        DBG("Forwarding drop to parent Rack");
        
        // The parent Rack should handle all drops centrally
        // This ensures consistent behavior whether the drop is on a slot or between slots
        
        // Convert source details to parent's coordinate system
        juce::Point<int> positionInParent;
        
        // Handle case where direct parent is RackContainer or Rack
        if (parentComponent->getComponentID() == "Rack") 
        {
            // Direct parent is the Rack
            Rack* parentRack = dynamic_cast<Rack*>(parentComponent);
            if (parentRack != nullptr)
            {
                positionInParent = parentRack->getLocalPoint(this, details.localPosition);
                
                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    details.description,
                    details.sourceComponent.get(),
                    positionInParent
                );
                
                // Call the parent's itemDropped directly
                parentRack->itemDropped(parentDetails);
            }
            return;
        }
        else if (parentComponent->getComponentID() == "RackContainer")
        {
            // Parent is RackContainer, need to find its Rack parent
            auto* container = dynamic_cast<Rack::RackContainer*>(parentComponent);
            if (container != nullptr && container->rack != nullptr)
            {
                // Convert twice - first to container, then to rack
                auto posInContainer = container->getLocalPoint(this, details.localPosition);
                positionInParent = container->rack->getLocalPoint(container, posInContainer);
                
                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    details.description,
                    details.sourceComponent.get(),
                    positionInParent
                );
                
                // Call the Rack's itemDropped
                container->rack->itemDropped(parentDetails);
                return;
            }
        }
    }
    
    // If we got here, we couldn't delegate to the parent Rack,
    // so handle it locally as a fallback (this should rarely happen)
    DBG("WARNING: Could not forward drop to parent Rack - handling locally");
    
    RackSlot* sourceSlot = dynamic_cast<RackSlot*>(details.sourceComponent.get());
    if (sourceSlot != nullptr && sourceSlot != this)
    {
        // Handle drag from another slot
        GearItem* itemToMove = sourceSlot->getGearItem();
        
        if (itemToMove != nullptr)
        {
            // Move the item to this slot
            if (!isAvailable())
            {
                clearGearItem();
            }
            
            sourceSlot->clearGearItem();
            setGearItem(itemToMove);
            DBG("Locally handled item move from slot " + juce::String(sourceSlot->getIndex()) + 
                " to slot " + juce::String(index));
        }
    }
    else
    {
        // Handle drops from other sources
        DBG("Unhandled drop from non-slot source");
    }
}

void RackSlot::setGearItem(GearItem* newGearItem)
{
    DBG("RackSlot::setGearItem for slot " + juce::String(index));
    gearItem = newGearItem;
    repaint(); // Trigger repaint to show the gear item
}

void RackSlot::clearGearItem()
{
    DBG("RackSlot::clearGearItem for slot " + juce::String(index));
    gearItem = nullptr;
    repaint(); // Trigger repaint to update
}

void RackSlot::setHighlighted(bool shouldHighlight)
{
    highlighted = shouldHighlight;
    repaint();
}

// New helper method to find parent Rack
juce::Component* RackSlot::findParentRackComponent()
{
    juce::Component* parent = getParentComponent();
    while (parent != nullptr)
    {
        if (parent->getComponentID() == "Rack" || parent->getComponentID() == "RackContainer")
        {
            return parent;
        }
        parent = parent->getParentComponent();
    }
    return nullptr;
} 