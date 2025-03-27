#include "RackSlot.h"
#include "GearLibrary.h"

RackSlot::RackSlot(SlotType slotType, int slotIndex, int slotSize)
    : type(slotType), index(slotIndex), size(slotSize)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setComponentID("RackSlot_" + juce::String(slotIndex));
    
    // CRITICAL: Enable drag-and-drop reception
    setInterceptsMouseClicks(true, true);
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
        
        // Border color based on highlight state
        g.setColour(highlighted ? juce::Colours::green : juce::Colours::black);
        g.drawRect(getLocalBounds(), highlighted ? 3 : 1);
        
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
        
        // Draw border based on highlight state (useful for when rearranging gear)
        if (highlighted)
        {
            g.setColour(juce::Colours::orange);
            g.drawRect(getLocalBounds(), 3);
        }
    }
}

void RackSlot::resized()
{
}

void RackSlot::mouseDown(const juce::MouseEvent& e)
{
    DBG("RackSlot::mouseDown on slot " + juce::String(index));
    
    if (gearItem != nullptr)
    {
        dragMode = true;
        dragger.startDraggingComponent(this, e);
        DBG("Started dragging component in slot " + juce::String(index));
    }
}

void RackSlot::mouseDrag(const juce::MouseEvent& /*e*/)
{
    if (dragMode && gearItem != nullptr)
    {
        DBG("RackSlot::mouseDrag on slot " + juce::String(index));
        
        // Start a drag-and-drop operation
        juce::Component* comp = this;
        juce::DragAndDropContainer* dragContainer = nullptr;
        
        while (comp != nullptr)
        {
            dragContainer = dynamic_cast<juce::DragAndDropContainer*>(comp);
            if (dragContainer != nullptr)
                break;
            comp = comp->getParentComponent();
            
            if (comp != nullptr)
                DBG("Looking for DragAndDropContainer in parent: " + comp->getComponentID());
        }
        
        if (dragContainer != nullptr)
        {
            DBG("Found DragAndDropContainer: " + dragContainer->getComponentID());
            
            juce::var dragData(index);
            this->toFront(true);
            dragContainer->startDragging(dragData, this, juce::ScaledImage(), false);
            dragMode = false;
            
            DBG("Started drag operation with data: " + juce::String(index));
        }
        else
        {
            DBG("No DragAndDropContainer found in hierarchy");
        }
    }
}

bool RackSlot::isCompatibleWithGear(const GearItem* itemToCheck) const
{
    if (itemToCheck == nullptr)
        return false;
        
    // Check type compatibility
    bool typeMatch = (type == SlotType::Series500 && itemToCheck->type == GearType::Series500) ||
                     (type == SlotType::Rack19Inch && itemToCheck->type == GearType::Rack19Inch);
                     
    // Check size compatibility
    bool sizeMatch = itemToCheck->slotSize <= size;
    
    return typeMatch && sizeMatch;
}

void RackSlot::setGearItem(GearItem* newGearItem)
{
    gearItem = newGearItem;
    repaint();
}

// DragAndDropTarget implementation
bool RackSlot::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& sourceDetails)
{
    // Check if the source is a DraggableListBox from the GearLibrary
    DBG("RackSlot::isInterestedInDragSource called on slot " + juce::String(index) + 
        ", description=" + sourceDetails.description.toString());
    
    // CRITICAL FIX: Always accept drag sources with integer data (row indices)
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
    if (dynamic_cast<RackSlot*>(sourceComp) != nullptr && sourceComp != this)
    {
        DBG("RackSlot is interested in drag from another RackSlot");
        return true;
    }
    
    // For maximum compatibility, accept all drags during development
    DBG("RackSlot accepting all drags during development");
    return true;
}

void RackSlot::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("RackSlot::itemDragEnter on slot " + juce::String(index) + 
        ", description=" + dragSourceDetails.description.toString());
    setHighlighted(true);
}

void RackSlot::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Keep highlighted
    DBG("RackSlot::itemDragMove on slot " + juce::String(index) + 
        ", description=" + dragSourceDetails.description.toString());
}

void RackSlot::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*dragSourceDetails*/)
{
    DBG("RackSlot::itemDragExit on slot " + juce::String(index));
    setHighlighted(false);
}

void RackSlot::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    DBG("RackSlot::itemDropped on slot " + juce::String(index) + 
        ", description=" + dragSourceDetails.description.toString());
    setHighlighted(false);
    
    // CRITICAL: Get the gear index directly from the drag data
    if (dragSourceDetails.description.isInt())
    {
        int gearIndex = static_cast<int>(dragSourceDetails.description);
        DBG("Dropped gear index: " + juce::String(gearIndex));
        
        // Find the GearLibrary component - search all window hierarchy
        GearLibrary* gearLibrary = nullptr;
        
        // Start with the top-level window
        juce::Component* topLevelComponent = getTopLevelComponent();
        
        if (topLevelComponent != nullptr)
        {
            DBG("Found top level component: " + topLevelComponent->getComponentID());
            
            // APPROACH 1: Try a direct search through all children of top-level component
            for (int i = 0; i < topLevelComponent->getNumChildComponents(); ++i)
            {
                juce::Component* child = topLevelComponent->getChildComponent(i);
                if (child != nullptr)
                {
                    DBG("Examining top-level child: " + child->getComponentID());
                    
                    // See if this is a GearLibrary
                    if (dynamic_cast<GearLibrary*>(child) != nullptr)
                    {
                        gearLibrary = dynamic_cast<GearLibrary*>(child);
                        DBG("Found GearLibrary directly");
                        break;
                    }
                    
                    // Check the child's children (one level deeper)
                    for (int j = 0; j < child->getNumChildComponents(); ++j)
                    {
                        juce::Component* grandchild = child->getChildComponent(j);
                        if (grandchild != nullptr && dynamic_cast<GearLibrary*>(grandchild) != nullptr)
                        {
                            gearLibrary = dynamic_cast<GearLibrary*>(grandchild);
                            DBG("Found GearLibrary at grandchild level");
                            break;
                        }
                    }
                    
                    if (gearLibrary != nullptr)
                        break;
                }
            }
            
            // APPROACH 2: If not found, try direct ID match at any level
            if (gearLibrary == nullptr)
            {
                DBG("Trying direct ID match approach");
                juce::Component* target = nullptr;
                
                // Find component by ID recursively with a helper lambda
                std::function<void(juce::Component*)> findComponent = 
                    [&target, &findComponent](juce::Component* comp) {
                        if (comp->getComponentID() == "GearLibrary")
                        {
                            target = comp;
                            return;
                        }
                        
                        for (int i = 0; i < comp->getNumChildComponents(); ++i)
                        {
                            if (target == nullptr)
                                findComponent(comp->getChildComponent(i));
                        }
                    };
                
                findComponent(topLevelComponent);
                
                if (target != nullptr)
                {
                    gearLibrary = dynamic_cast<GearLibrary*>(target);
                    DBG("Found GearLibrary through ID match");
                }
            }
        }
        
        if (gearLibrary != nullptr)
        {
            // Get the gear item from the library
            DBG("Attempting to get gear item at index " + juce::String(gearIndex));
            GearItem* originalItem = gearLibrary->getGearItem(gearIndex);
            
            if (originalItem != nullptr)
            {
                DBG("Found gear item: " + originalItem->name);
                
                // Create a new copy of the item for this slot
                GearItem* newItem = new GearItem(
                    originalItem->name,
                    originalItem->manufacturer,
                    originalItem->type,
                    originalItem->category,
                    originalItem->slotSize,
                    originalItem->imageUrl,
                    originalItem->controls
                );
                
                // Check if the item is compatible with this slot
                if (isCompatibleWithGear(newItem) && isAvailable())
                {
                    // Add the item to this slot
                    setGearItem(newItem);
                    DBG("SUCCESS: Added gear item '" + newItem->name + "' to slot " + juce::String(index));
                }
                else
                {
                    // Clean up if not compatible
                    delete newItem;
                    DBG("ERROR: Item not compatible with slot " + juce::String(index) + 
                        ". Type match: " + juce::String(originalItem->type == GearType::Series500 ? 
                                                     "Series500" : "Rack19Inch") + 
                        ", Slot type: " + juce::String(type == SlotType::Series500 ? 
                                                    "Series500" : "Rack19Inch"));
                }
            }
            else
            {
                DBG("ERROR: Could not find gear item at index " + juce::String(gearIndex));
            }
        }
        else
        {
            DBG("ERROR: Could not find GearLibrary component");
        }
    }
    else if (auto* sourceSlot = dynamic_cast<RackSlot*>(dragSourceDetails.sourceComponent.get()))
    {
        // Handle drag from another slot - this is rearranging
        DBG("Handling drag from RackSlot " + juce::String(sourceSlot->getIndex()));
        
        // Get the gear item from the source slot
        auto* itemToMove = sourceSlot->getGearItem();
        
        // Check if the item is compatible with this slot
        if (itemToMove != nullptr && isCompatibleWithGear(itemToMove) && isAvailable())
        {
            // Move the item to this slot
            sourceSlot->clearGearItem();
            setGearItem(itemToMove);
            DBG("SUCCESS: Moved gear item from slot " + juce::String(sourceSlot->getIndex()) + 
                " to slot " + juce::String(index));
        }
        else
        {
            DBG("ERROR: Cannot move item - incompatible or slot not available");
        }
    }
    else
    {
        DBG("ERROR: Unrecognized drag source type");
    }
} 