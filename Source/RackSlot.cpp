#include "RackSlot.h"
#include "GearLibrary.h"
#include "Rack.h"

RackSlot::RackSlot(int slotIndex)
    : index(slotIndex), highlighted(false), isDragging(false)
{
    setComponentID("RackSlot_" + juce::String(index));
    setInterceptsMouseClicks(true, true);

    // Create arrow buttons for moving items up and down
    auto createArrowPath = [](bool isUpArrow)
    {
        juce::Path arrowPath;
        if (isUpArrow)
        {
            // Up arrow
            arrowPath.addTriangle(10.0f, 2.0f, 2.0f, 18.0f, 18.0f, 18.0f);
        }
        else
        {
            // Down arrow
            arrowPath.addTriangle(10.0f, 18.0f, 2.0f, 2.0f, 18.0f, 2.0f);
        }
        return arrowPath;
    };

    // Create X button path
    auto createXPath = []()
    {
        juce::Path xPath;
        xPath.addLineSegment(juce::Line<float>(2.0f, 2.0f, 18.0f, 18.0f), 2.0f);
        xPath.addLineSegment(juce::Line<float>(2.0f, 18.0f, 18.0f, 2.0f), 2.0f);
        return xPath;
    };

    // Create drawable objects for the buttons
    auto normalUpArrow = std::make_unique<juce::DrawablePath>();
    normalUpArrow->setPath(createArrowPath(true));
    normalUpArrow->setFill(juce::Colours::white.withAlpha(0.8f));

    auto overUpArrow = std::make_unique<juce::DrawablePath>();
    overUpArrow->setPath(createArrowPath(true));
    overUpArrow->setFill(juce::Colours::white);

    auto normalDownArrow = std::make_unique<juce::DrawablePath>();
    normalDownArrow->setPath(createArrowPath(false));
    normalDownArrow->setFill(juce::Colours::white.withAlpha(0.8f));

    auto overDownArrow = std::make_unique<juce::DrawablePath>();
    overDownArrow->setPath(createArrowPath(false));
    overDownArrow->setFill(juce::Colours::white);

    auto normalX = std::make_unique<juce::DrawablePath>();
    normalX->setPath(createXPath());
    normalX->setFill(juce::Colours::red.withAlpha(0.8f));

    auto overX = std::make_unique<juce::DrawablePath>();
    overX->setPath(createXPath());
    overX->setFill(juce::Colours::red);

    // Create the buttons
    upButton = std::make_unique<juce::DrawableButton>("UpButton", juce::DrawableButton::ButtonStyle::ImageFitted);
    upButton->setImages(normalUpArrow.get(), overUpArrow.get());
    upButton->setTooltip("Move item up");
    upButton->addListener(this);
    addAndMakeVisible(upButton.get());

    downButton = std::make_unique<juce::DrawableButton>("DownButton", juce::DrawableButton::ButtonStyle::ImageFitted);
    downButton->setImages(normalDownArrow.get(), overDownArrow.get());
    downButton->setTooltip("Move item down");
    downButton->addListener(this);
    addAndMakeVisible(downButton.get());

    removeButton = std::make_unique<juce::DrawableButton>("RemoveButton", juce::DrawableButton::ButtonStyle::ImageFitted);
    removeButton->setImages(normalX.get(), overX.get());
    removeButton->setTooltip("Remove item from rack");
    removeButton->addListener(this);
    addAndMakeVisible(removeButton.get());

    // Initial button state
    updateButtonStates();

    DBG("Created RackSlot with index " + juce::String(index));
}

RackSlot::~RackSlot()
{
    // Make sure to remove listeners before buttons are destroyed
    if (upButton != nullptr)
        upButton->removeListener(this);
    if (downButton != nullptr)
        downButton->removeListener(this);
    if (removeButton != nullptr)
        removeButton->removeListener(this);
}

void RackSlot::paint(juce::Graphics &g)
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
    g.drawText(juce::String(index + 1), getLocalBounds().reduced(5, 5).removeFromTop(20),
               juce::Justification::topLeft, true);

    // If we have a gear item, draw its details and image
    if (!isAvailable() && gearItem != nullptr)
    {
        // Debug info about the gear item
        DBG("RackSlot::paint - Slot " + juce::String(index) + " painting gear item: " + gearItem->name);
        DBG("  - Has faceplate image: " + juce::String(gearItem->faceplateImage.isValid() ? "YES" : "NO"));
        if (gearItem->faceplateImage.isValid())
        {
            DBG("  - Faceplate dimensions: " +
                juce::String(gearItem->faceplateImage.getWidth()) + "x" +
                juce::String(gearItem->faceplateImage.getHeight()));
        }
        DBG("  - Has thumbnail image: " + juce::String(gearItem->image.isValid() ? "YES" : "NO"));

        // Check if we have a faceplate image
        bool hasFaceplate = gearItem->faceplateImage.isValid();

        if (hasFaceplate)
        {
            DBG("Drawing faceplate image for " + gearItem->name + " in slot " + juce::String(index));

            // If we have a faceplate image, use it instead of the default rendering
            juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);

            // Draw name above the faceplate (for better identification)
            g.setFont(12.0f);
            g.setColour(juce::Colours::white);
            juce::Rectangle<int> nameArea = faceplateArea.removeFromTop(20);
            g.drawText(gearItem->name, nameArea, juce::Justification::centred, true);

            // Draw the faceplate image
            g.drawImageWithin(gearItem->faceplateImage,
                              faceplateArea.getX(), faceplateArea.getY(),
                              faceplateArea.getWidth(), faceplateArea.getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            DBG("No faceplate image available for " + gearItem->name + " in slot " + juce::String(index) + ", using standard display");

            // No faceplate, use the original rendering with name, manufacturer and thumbnail

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

            // Draw thumbnail image if available
            if (gearItem->image.isValid())
            {
                juce::Rectangle<int> imageArea = getLocalBounds().reduced(20);
                g.drawImageWithin(gearItem->image,
                                  imageArea.getX(), imageArea.getY() + 40,
                                  imageArea.getWidth(), imageArea.getHeight() - 40,
                                  juce::RectanglePlacement::centred);
            }
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
    // Position the arrow buttons in the top-right corner on a single line
    const int buttonSize = 20;
    const int margin = 5;

    // Place buttons side by side horizontally
    upButton->setBounds(getWidth() - (buttonSize * 3) - margin - 2, margin, buttonSize, buttonSize);
    downButton->setBounds(getWidth() - (buttonSize * 2) - margin, margin, buttonSize, buttonSize);
    removeButton->setBounds(getWidth() - buttonSize - margin, margin, buttonSize, buttonSize);
}

void RackSlot::buttonClicked(juce::Button *button)
{
    if (button == upButton.get())
    {
        moveUp();
    }
    else if (button == downButton.get())
    {
        moveDown();
    }
    else if (button == removeButton.get())
    {
        clearGearItem();
    }
}

void RackSlot::updateButtonStates()
{
    // Disable buttons if there's no gear item
    bool hasGear = !isAvailable() && gearItem != nullptr;

    // Make sure buttons exist before using them
    if (upButton == nullptr || downButton == nullptr || removeButton == nullptr)
        return;

    // Up button should be disabled for the first slot
    upButton->setEnabled(hasGear && index > 0);

    // Down button should be disabled for the last slot
    // We need to check the total number of slots from the parent Rack
    Rack *parentRack = dynamic_cast<Rack *>(findParentRackComponent());
    if (parentRack != nullptr)
    {
        int totalSlots = parentRack->getNumSlots();
        downButton->setEnabled(hasGear && index < totalSlots - 1);
    }
    else
    {
        downButton->setEnabled(hasGear); // Fallback if we can't get the rack
    }

    // Remove button is only enabled when there's a gear item
    removeButton->setEnabled(hasGear);
}

void RackSlot::moveUp()
{
    if (index <= 0 || isAvailable() || gearItem == nullptr)
    {
        return; // Can't move up if we're the first slot or have no gear
    }

    // Find the parent Rack to handle the movement
    Rack *parentRack = nullptr;
    juce::Component *parentComponent = findParentRackComponent();

    if (parentComponent == nullptr)
    {
        DBG("ERROR: Could not find parent component in moveUp()");
        return;
    }

    if (parentComponent->getComponentID() == "Rack")
    {
        parentRack = dynamic_cast<Rack *>(parentComponent);
    }
    else if (parentComponent->getComponentID() == "RackContainer")
    {
        auto *container = dynamic_cast<Rack::RackContainer *>(parentComponent);
        if (container != nullptr && container->rack != nullptr)
        {
            parentRack = container->rack;
        }
    }

    if (parentRack != nullptr)
    {
        // Move the item up by 1 slot
        parentRack->rearrangeGearAsSortableList(index, index - 1);
    }
    else
    {
        DBG("ERROR: Could not find parent Rack in moveUp()");
    }
}

void RackSlot::moveDown()
{
    if (isAvailable() || gearItem == nullptr)
    {
        return; // Can't move if we have no gear
    }

    // Find the parent Rack to handle the movement
    Rack *parentRack = nullptr;
    juce::Component *parentComponent = findParentRackComponent();

    if (parentComponent == nullptr)
    {
        DBG("ERROR: Could not find parent component in moveDown()");
        return;
    }

    if (parentComponent->getComponentID() == "Rack")
    {
        parentRack = dynamic_cast<Rack *>(parentComponent);
    }
    else if (parentComponent->getComponentID() == "RackContainer")
    {
        auto *container = dynamic_cast<Rack::RackContainer *>(parentComponent);
        if (container != nullptr && container->rack != nullptr)
        {
            parentRack = container->rack;
        }
    }

    if (parentRack != nullptr)
    {
        int totalSlots = parentRack->getNumSlots();
        if (index < totalSlots - 1)
        {
            // Move the item down by 1 slot
            parentRack->rearrangeGearAsSortableList(index, index + 1);
        }
    }
    else
    {
        DBG("ERROR: Could not find parent Rack in moveDown()");
    }
}

// Keeping the mouse event handlers but simplifying them to do nothing
void RackSlot::mouseDown(const juce::MouseEvent &e)
{
    // We're now using buttons for navigation instead of drag-and-drop
}

void RackSlot::mouseDrag(const juce::MouseEvent &e)
{
    // We're now using buttons for navigation instead of drag-and-drop
}

void RackSlot::mouseUp(const juce::MouseEvent &)
{
    // We're now using buttons for navigation instead of drag-and-drop
}

// The rest of the existing implementation follows...

bool RackSlot::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &sourceDetails)
{
    // We're not using drag-and-drop for reordering anymore, but keeping the code for GearLibrary drags

    // Accept drag sources from the GearLibrary list box (legacy)
    if (sourceDetails.description.isInt())
    {
        auto *sourceComp = sourceDetails.sourceComponent.get();
        if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" ||
                           sourceComp->getComponentID() == "GearListBox"))
        {
            return true;
        }
    }

    // Accept drag sources from TreeView (new hierarchical view)
    if (sourceDetails.description.isString())
    {
        juce::String desc = sourceDetails.description.toString();
        if (desc.startsWith("GEAR:"))
        {
            auto *sourceComp = sourceDetails.sourceComponent.get();
            if (sourceComp && dynamic_cast<juce::TreeView *>(sourceComp) != nullptr)
            {
                return true;
            }
        }
    }

    return false;
}

void RackSlot::itemDragEnter(const juce::DragAndDropTarget::SourceDetails & /*details*/)
{
    setHighlighted(true);
}

void RackSlot::itemDragMove(const juce::DragAndDropTarget::SourceDetails & /*details*/)
{
    // Nothing needed here
}

void RackSlot::itemDragExit(const juce::DragAndDropTarget::SourceDetails & /*details*/)
{
    setHighlighted(false);
}

void RackSlot::itemDropped(const juce::DragAndDropTarget::SourceDetails &details)
{
    setHighlighted(false);

    // Only handle drops from GearLibrary - delegate to parent Rack
    juce::Component *parentComponent = findParentRackComponent();

    if (parentComponent != nullptr)
    {
        // Convert source details to parent's coordinate system
        juce::Point<int> positionInParent;

        // Handle case where direct parent is RackContainer or Rack
        if (parentComponent->getComponentID() == "Rack")
        {
            // Direct parent is the Rack
            Rack *parentRack = dynamic_cast<Rack *>(parentComponent);
            if (parentRack != nullptr)
            {
                positionInParent = parentRack->getLocalPoint(this, details.localPosition);

                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    details.description,
                    details.sourceComponent.get(),
                    positionInParent);

                // Call the parent's itemDropped directly
                parentRack->itemDropped(parentDetails);
            }
        }
        else if (parentComponent->getComponentID() == "RackContainer")
        {
            // Parent is RackContainer, need to find its Rack parent
            auto *container = dynamic_cast<Rack::RackContainer *>(parentComponent);
            if (container != nullptr && container->rack != nullptr)
            {
                // Convert twice - first to container, then to rack
                auto posInContainer = container->getLocalPoint(this, details.localPosition);
                positionInParent = container->rack->getLocalPoint(container, posInContainer);

                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    details.description,
                    details.sourceComponent.get(),
                    positionInParent);

                // Call the Rack's itemDropped
                container->rack->itemDropped(parentDetails);
            }
        }
    }
}

void RackSlot::setGearItem(GearItem *newGearItem)
{
    DBG("RackSlot::setGearItem for slot " + juce::String(index));
    gearItem = newGearItem;
    updateButtonStates(); // Update button states when gear changes
    repaint();            // Trigger repaint to show the gear item
}

void RackSlot::clearGearItem()
{
    DBG("RackSlot::clearGearItem for slot " + juce::String(index));
    gearItem = nullptr;
    updateButtonStates(); // Update button states when gear is removed
    repaint();            // Trigger repaint to update
}

void RackSlot::setHighlighted(bool shouldHighlight)
{
    highlighted = shouldHighlight;
    repaint();
}

// New helper method to find parent Rack
juce::Component *RackSlot::findParentRackComponent()
{
    juce::Component *parent = getParentComponent();
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