#include "RackSlot.h"
#include "GearLibrary.h"
#include "Rack.h"
#include <fstream>

// Add at the top of the file, after includes
static std::ofstream logFile("/tmp/rack_slot.log");

// Helper function for logging
static void logToFile(const juce::String &message)
{
    if (logFile.is_open())
    {
        logFile << message << std::endl;
        logFile.flush();
    }
}

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

            // Draw controls on top of the faceplate
            drawControls(g, faceplateArea);
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

void RackSlot::drawControls(juce::Graphics &g, const juce::Rectangle<int> &faceplateArea)
{
    if (gearItem == nullptr)
        return;

    for (const auto &control : gearItem->controls)
    {
        // Calculate control position relative to faceplate
        int x = faceplateArea.getX() + (int)(control.position.getX() * faceplateArea.getWidth());
        int y = faceplateArea.getY() + (int)(control.position.getY() * faceplateArea.getHeight());

        // Draw control based on type
        switch (control.type)
        {
        case GearControl::Type::Knob:
            drawKnob(g, control, x, y);
            break;
        case GearControl::Type::Switch:
            drawSwitch(g, control, x, y);
            break;
        case GearControl::Type::Button:
            drawButton(g, control, x, y);
            break;
        case GearControl::Type::Fader:
            drawFader(g, control, x, y);
            break;
        }
    }
}

void RackSlot::drawKnob(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const int knobSize = 40;
    const float centerX = x + knobSize / 2.0f;
    const float centerY = y + knobSize / 2.0f;
    const float radius = knobSize / 2.0f;

    // Draw knob background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(x, y, knobSize, knobSize);

    // Draw knob border
    g.setColour(juce::Colours::grey);
    g.drawEllipse(x, y, knobSize, knobSize, 2.0f);

    // Calculate rotation angle based on value
    float angle;
    if (control.steps.size() > 0)
    {
        // For stepped knobs, use the current step
        angle = control.steps[control.currentStepIndex];
    }
    else
    {
        // For continuous knobs, map the normalized value (0-1) to the angle range
        angle = control.startAngle + control.value * (control.endAngle - control.startAngle);
    }

    // Convert angle to radians and draw indicator line
    float radians = juce::degreesToRadians(angle + 90.0f); // Add 90 degrees to make 0 at 6 o'clock
    float endX = centerX + (radius - 4) * std::cos(radians);
    float endY = centerY + (radius - 4) * std::sin(radians);

    g.setColour(juce::Colours::white);
    g.drawLine(centerX, centerY, endX, endY, 2.0f);

    // Draw label
    g.setFont(10.0f);
    g.drawText(control.name, x, y + knobSize + 2, knobSize, 15, juce::Justification::centred);
}

void RackSlot::drawSwitch(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const int switchWidth = 30;
    const int switchHeight = 60;
    const bool isVertical = control.orientation == "vertical";

    // Draw switch background
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(x, y, switchWidth, switchHeight, 4.0f);

    // Draw switch border
    g.setColour(juce::Colours::grey);
    g.drawRoundedRectangle(x, y, switchWidth, switchHeight, 4.0f, 2.0f);

    // Draw switch position indicator
    g.setColour(juce::Colours::white);
    if (isVertical)
    {
        float indicatorY = y + (control.currentIndex * (switchHeight / control.options.size()));
        g.fillRoundedRectangle(x + 4, indicatorY + 4, switchWidth - 8, (switchHeight / control.options.size()) - 8, 2.0f);
    }
    else
    {
        float indicatorX = x + (control.currentIndex * (switchWidth / control.options.size()));
        g.fillRoundedRectangle(indicatorX + 4, y + 4, (switchWidth / control.options.size()) - 8, switchHeight - 8, 2.0f);
    }

    // Draw label
    g.setFont(10.0f);
    g.drawText(control.name, x, y + switchHeight + 2, switchWidth, 15, juce::Justification::centred);
}

void RackSlot::drawButton(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const int buttonSize = 30;

    // Draw button background
    g.setColour(control.value > 0.5f ? juce::Colours::red : juce::Colours::darkgrey);
    g.fillRoundedRectangle(x, y, buttonSize, buttonSize, 4.0f);

    // Draw button border
    g.setColour(juce::Colours::grey);
    g.drawRoundedRectangle(x, y, buttonSize, buttonSize, 4.0f, 2.0f);

    // Draw label
    g.setFont(10.0f);
    g.drawText(control.name, x, y + buttonSize + 2, buttonSize, 15, juce::Justification::centred);
}

void RackSlot::drawFader(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const int faderWidth = 20;
    const int faderHeight = 100;

    // Draw fader track
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(x, y, faderWidth, faderHeight, 2.0f);

    // Draw fader handle
    float handleY = y + (1.0f - control.value) * faderHeight;
    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(x - 5, handleY - 5, faderWidth + 10, 10, 4.0f);

    // Draw label
    g.setFont(10.0f);
    g.drawText(control.name, x, y + faderHeight + 2, faderWidth, 15, juce::Justification::centred);
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

void RackSlot::mouseDown(const juce::MouseEvent &e)
{
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Find control at mouse position
    activeControl = findControlAtPosition(e.position, faceplateArea);
    if (activeControl != nullptr)
    {
        logToFile("Mouse down on control: " + activeControl->name + " type: " + juce::String((int)activeControl->type));

        // Calculate control bounds
        int x = faceplateArea.getX() + (int)(activeControl->position.getX() * faceplateArea.getWidth());
        int y = faceplateArea.getY() + (int)(activeControl->position.getY() * faceplateArea.getHeight());
        juce::Rectangle<int> controlBounds(x, y, 40, 40); // Default size, adjust based on control type

        // Store drag start state for knobs and faders
        if (activeControl->type == GearControl::Type::Knob || activeControl->type == GearControl::Type::Fader)
        {
            dragStartPos = e.position;
            dragStartValue = activeControl->value;

            // For knobs, calculate the initial angle
            if (activeControl->type == GearControl::Type::Knob)
            {
                float centerX = controlBounds.getCentreX();
                float centerY = controlBounds.getCentreY();
                float initialAngle = std::atan2(e.position.y - centerY, e.position.x - centerX);
                initialAngle = juce::radiansToDegrees(initialAngle) + 90.0f;
                if (initialAngle < 0)
                    initialAngle += 360.0f;
                dragStartAngle = initialAngle;
                logToFile("Drag started - Initial angle: " + juce::String(initialAngle) +
                          " Value: " + juce::String(activeControl->value));
            }

            isDragging = true;
        }

        // Handle interaction based on control type (for click/tap)
        switch (activeControl->type)
        {
        case GearControl::Type::Knob:
        case GearControl::Type::Fader:
            // Don't update value on click, wait for drag
            break;
        case GearControl::Type::Switch:
            handleSwitchInteraction(*activeControl);
            repaint();
            break;
        case GearControl::Type::Button:
            handleButtonInteraction(*activeControl);
            repaint();
            break;
        }
    }
}

void RackSlot::mouseDrag(const juce::MouseEvent &e)
{
    if (!isDragging || activeControl == nullptr || gearItem == nullptr || !gearItem->faceplateImage.isValid())
    {
        logToFile(juce::String("Drag ignored - isDragging: ") + (isDragging ? "true" : "false") +
                  juce::String(" activeControl: ") + (activeControl != nullptr ? "true" : "false"));
        return;
    }

    logToFile("Mouse drag - Current position: " + e.position.toString());

    // Calculate faceplate area
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate control bounds
    int x = faceplateArea.getX() + (int)(activeControl->position.getX() * faceplateArea.getWidth());
    int y = faceplateArea.getY() + (int)(activeControl->position.getY() * faceplateArea.getHeight());
    juce::Rectangle<int> controlBounds(x, y, 40, 40); // Default size, adjust based on control type

    switch (activeControl->type)
    {
    case GearControl::Type::Knob:
    {
        // Calculate angle from center for knob
        float centerX = controlBounds.getCentreX();
        float centerY = controlBounds.getCentreY();
        float angle = std::atan2(e.position.y - centerY, e.position.x - centerX);
        angle = juce::radiansToDegrees(angle) + 90.0f; // Adjust to match our coordinate system
        if (angle < 0)
            angle += 360.0f;

        logToFile("Knob angle: " + juce::String(angle) +
                  " startAngle: " + juce::String(activeControl->startAngle) +
                  " endAngle: " + juce::String(activeControl->endAngle));

        if (activeControl->steps.size() > 0)
        {
            // For stepped knobs, find nearest step
            float bestAngle = activeControl->steps[0];
            float bestDiff = std::abs(angle - bestAngle);
            int bestStep = 0;

            for (int i = 1; i < activeControl->steps.size(); ++i)
            {
                float diff = std::abs(angle - activeControl->steps[i]);
                if (diff < bestDiff)
                {
                    bestDiff = diff;
                    bestAngle = activeControl->steps[i];
                    bestStep = i;
                }
            }
            activeControl->currentStepIndex = bestStep;
            activeControl->value = (float)bestStep / (activeControl->steps.size() - 1);
            logToFile("Stepped knob - Step: " + juce::String(bestStep) +
                      " Value: " + juce::String(activeControl->value));
        }
        else
        {
            // For continuous knobs, calculate relative change
            float angleDiff = angle - dragStartAngle;
            if (angleDiff > 180.0f)
                angleDiff -= 360.0f;
            else if (angleDiff < -180.0f)
                angleDiff += 360.0f;

            // Convert angle difference to value change
            float angleRange = activeControl->endAngle - activeControl->startAngle;
            if (angleRange < 0)
                angleRange += 360.0f;

            // Calculate the normalized angle position
            float normalizedAngle = (angle - activeControl->startAngle) / angleRange;
            if (normalizedAngle < 0)
                normalizedAngle += 1.0f;

            // Clamp to valid range
            float newValue = juce::jlimit(0.0f, 1.0f, normalizedAngle);

            activeControl->value = newValue;
            logToFile("Continuous knob - Raw angle: " + juce::String(angle) +
                      " Normalized angle: " + juce::String(normalizedAngle) +
                      " New value: " + juce::String(newValue));
        }
        repaint();
        break;
    }
    case GearControl::Type::Fader:
    {
        // Map vertical position to value
        float normalizedY = 1.0f - (float)(e.position.y - controlBounds.getY()) / controlBounds.getHeight();
        activeControl->value = juce::jlimit(0.0f, 1.0f, normalizedY);
        logToFile("Fader - Value: " + juce::String(activeControl->value) +
                  " Y: " + juce::String(e.position.y) +
                  " Bounds Y: " + juce::String(controlBounds.getY()) +
                  " Height: " + juce::String(controlBounds.getHeight()));
        repaint();
        break;
    }
    default:
        break; // Other controls don't need drag handling
    }
}

void RackSlot::mouseUp(const juce::MouseEvent &)
{
    if (isDragging)
    {
        logToFile("Mouse up - Final value: " + (activeControl ? juce::String(activeControl->value) : "no control"));
        isDragging = false;
    }
    activeControl = nullptr;
}

void RackSlot::mouseDoubleClick(const juce::MouseEvent &e)
{
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Find control at mouse position
    if (auto *control = findControlAtPosition(e.position, faceplateArea))
    {
        // Double-click resets control to default value
        switch (control->type)
        {
        case GearControl::Type::Knob:
            if (control->steps.size() > 0)
                control->currentStepIndex = 0;
            else
                control->value = 0.0f;
            break;
        case GearControl::Type::Switch:
            control->currentIndex = 0;
            break;
        case GearControl::Type::Button:
            control->value = 0.0f;
            break;
        case GearControl::Type::Fader:
            control->value = 0.0f;
            break;
        }
        repaint();
    }
}

GearControl *RackSlot::findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<int> &faceplateArea)
{
    if (gearItem == nullptr)
        return nullptr;

    for (auto &control : gearItem->controls)
    {
        // Calculate control bounds
        int x = faceplateArea.getX() + (int)(control.position.getX() * faceplateArea.getWidth());
        int y = faceplateArea.getY() + (int)(control.position.getY() * faceplateArea.getHeight());

        // Adjust size based on control type
        int width, height;
        switch (control.type)
        {
        case GearControl::Type::Knob:
            width = height = 40;
            break;
        case GearControl::Type::Switch:
            width = 30;
            height = 60;
            break;
        case GearControl::Type::Button:
            width = height = 30;
            break;
        case GearControl::Type::Fader:
            width = 20;
            height = 100;
            break;
        default:
            width = height = 40;
        }

        juce::Rectangle<int> controlBounds(x, y, width, height);
        if (controlBounds.contains(position.toInt()))
            return &control;
    }

    return nullptr;
}

void RackSlot::handleKnobInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds)
{
    // Calculate angle from center
    float centerX = controlBounds.getCentreX();
    float centerY = controlBounds.getCentreY();
    float angle = std::atan2(mousePos.y - centerY, mousePos.x - centerX);
    angle = juce::radiansToDegrees(angle) + 90.0f; // Adjust to match our coordinate system
    if (angle < 0)
        angle += 360.0f;

    if (control.steps.size() > 0)
    {
        // For stepped knobs, find nearest step
        float bestAngle = control.steps[0];
        float bestDiff = std::abs(angle - bestAngle);

        for (int i = 1; i < control.steps.size(); ++i)
        {
            float diff = std::abs(angle - control.steps[i]);
            if (diff < bestDiff)
            {
                bestDiff = diff;
                bestAngle = control.steps[i];
                control.currentStepIndex = i;
            }
        }
        control.value = (float)control.currentStepIndex / (control.steps.size() - 1);
    }
    else
    {
        // For continuous knobs, map angle to value
        float normalizedAngle = (angle - control.startAngle) / (control.endAngle - control.startAngle);
        control.value = juce::jlimit(0.0f, 1.0f, normalizedAngle);
    }
}

void RackSlot::handleSwitchInteraction(GearControl &control)
{
    // Toggle between options
    control.currentIndex = (control.currentIndex + 1) % control.options.size();
    control.value = (float)control.currentIndex / (control.options.size() - 1);
}

void RackSlot::handleButtonInteraction(GearControl &control)
{
    // Toggle button state
    control.value = control.value > 0.5f ? 0.0f : 1.0f;
}

void RackSlot::handleFaderInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds)
{
    // Map vertical position to value
    float normalizedY = 1.0f - (float)(mousePos.y - controlBounds.getY()) / controlBounds.getHeight();
    control.value = juce::jlimit(0.0f, 1.0f, normalizedY);
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

// Helper method to find parent Rack
juce::Component *RackSlot::findParentRackComponent() const
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
