/**
 * @file RackSlot.cpp
 * @brief Implementation of the RackSlot class.
 *
 * This file implements the RackSlot class which provides the visual representation
 * and interaction handling for individual slots in the virtual rack. It manages
 * gear items, their controls, and provides functionality for drag-and-drop
 * operations, button interactions, and control manipulation.
 */

#include "RackSlot.h"
#include "Rack.h"
#include "../Model/GearItem.h"
#include <map>

/**
 * @brief Constructs a new RackSlot instance.
 *
 * Initializes the slot with the specified index and sets up the UI components
 * for navigation and interaction.
 *
 * @param fileSystem Reference to the file system
 * @param cacheManager Reference to the cache manager
 * @param presetManager Reference to the preset manager
 * @param gearLibrary Reference to the gear library
 * @param slotIndex The index of this slot in the rack
 */
RackSlot::RackSlot(RackModel &rackModel, int slotIndex)
    : index(slotIndex),
      rackModel(rackModel)
{

    setComponentID("RackSlot_" + juce::String(index));

    // Create arrow button paths
    auto createArrowPath = [](bool isUpArrow)
    {
        juce::Path arrowPath;
        if (isUpArrow)
        {
            // Up arrow triangle
            arrowPath.addTriangle(10.0f, 2.0f, 2.0f, 18.0f, 18.0f, 18.0f);
        }
        else
        {
            // Down arrow triangle
            arrowPath.addTriangle(10.0f, 18.0f, 2.0f, 2.0f, 18.0f, 2.0f);
        }
        return arrowPath;
    };

    auto createXPath = []()
    {
        juce::Path xPath;
        // Create an X shape
        xPath.addLineSegment(juce::Line<float>(4.0f, 4.0f, 16.0f, 16.0f), 2.0f);
        xPath.addLineSegment(juce::Line<float>(16.0f, 4.0f, 4.0f, 16.0f), 2.0f);
        return xPath;
    };

    // Create drawable objects for the up button
    auto normalUpArrow = std::make_unique<juce::DrawablePath>();
    normalUpArrow->setPath(createArrowPath(true));
    normalUpArrow->setFill(juce::Colours::white.withAlpha(0.8f));

    auto overUpArrow = std::make_unique<juce::DrawablePath>();
    overUpArrow->setPath(createArrowPath(true));
    overUpArrow->setFill(juce::Colours::white);

    // Create drawable objects for the down button
    auto normalDownArrow = std::make_unique<juce::DrawablePath>();
    normalDownArrow->setPath(createArrowPath(false));
    normalDownArrow->setFill(juce::Colours::white.withAlpha(0.8f));

    auto overDownArrow = std::make_unique<juce::DrawablePath>();
    overDownArrow->setPath(createArrowPath(false));
    overDownArrow->setFill(juce::Colours::white);

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

    // Create remove button (red X)
    auto normalX = std::make_unique<juce::DrawablePath>();
    normalX->setPath(createXPath());
    normalX->setFill(juce::Colours::red);

    auto overX = std::make_unique<juce::DrawablePath>();
    overX->setPath(createXPath());
    overX->setFill(juce::Colours::darkred);

    removeButton = std::make_unique<juce::DrawableButton>("RemoveButton", juce::DrawableButton::ButtonStyle::ImageFitted);
    removeButton->setImages(normalX.get(), overX.get());
    removeButton->setTooltip("Remove item");
    removeButton->addListener(this);
    addAndMakeVisible(removeButton.get());

    // Update button states
    updateButtonStates();
}

/**
 * @brief Destructor for the RackSlot class.
 *
 * Cleans up resources and ensures all images are properly released.
 */
RackSlot::~RackSlot()
{
    // Remove button listeners
    if (upButton)
        upButton->removeListener(this);
    if (downButton)
        downButton->removeListener(this);
}

/**
 * @brief Paints the rack slot and its contents.
 *
 * Draws the slot background, borders, and any gear item content.
 *
 * @param g The graphics context to paint with
 */
void RackSlot::paint(juce::Graphics &g)
{
    auto area = getLocalBounds();

    // Draw slot background
    if (isDragOver)
    {
        // Show drag feedback with light blue overlay
        g.setColour(juce::Colours::lightblue.withAlpha(0.3f));
        g.fillAll();
    }
    else if (slotBackgroundColor != juce::Colours::transparentBlack)
    {
        // Use the slot's background color if it's not transparent
        g.setColour(slotBackgroundColor);
        g.fillAll();
    }

    // Draw slot border
    g.setColour(juce::Colours::white);
    g.drawRect(area, 1);

    // Draw gear item info if present - get data from RackModel
    if (auto *slotData = rackModel.getSlotData(index))
    {
        if (slotData->isOccupied)
        {
            // Get gear item from library for image access
            auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);

            // Draw faceplate image if available (exactly like old system)
            if (gearItem && gearItem->faceplateImage.isValid())
            {
                // Calculate faceplate area (exactly like old system)
                juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);

                // Remove 20 pixels from top to match old system's faceplate area (without drawing text)
                faceplateArea.removeFromTop(20);

                // Calculate scaling factor based on faceplate dimensions (exactly like old system)
                float originalWidth = (float)gearItem->faceplateImage.getWidth();
                float originalHeight = (float)gearItem->faceplateImage.getHeight();
                float targetWidth = (float)faceplateArea.getWidth();
                float targetHeight = (float)faceplateArea.getHeight();

                // Calculate scale factor that maintains aspect ratio
                float scaleX = targetWidth / originalWidth;
                float scaleY = targetHeight / originalHeight;
                float scaleFactor = std::min(scaleX, scaleY); // Use the smaller scale to fit within bounds

                // Store the scale factor for use in drawing controls
                currentFaceplateScale = scaleFactor;

                // Calculate actual rendered image bounds (centered within faceplateArea)
                float scaledWidth = originalWidth * scaleFactor;
                float scaledHeight = originalHeight * scaleFactor;
                float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
                float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
                juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

                // Draw the faceplate image (exactly like old system)
                g.drawImageWithin(gearItem->faceplateImage,
                                  faceplateArea.getX(), faceplateArea.getY(),
                                  faceplateArea.getWidth(), faceplateArea.getHeight(),
                                  juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

                // Draw controls on top of the faceplate using actual image bounds
                drawControls(g, actualImageBounds);

                // TEMPORARY: Draw red borders around click targets for debugging
                g.setColour(juce::Colours::red);
                for (const auto &control : gearItem->controls)
                {

                    // Calculate control bounds (same as findControlAtPosition)
                    int x = actualImageBounds.getX() + (int)(control.position.getX() * actualImageBounds.getWidth());
                    int y = actualImageBounds.getY() + (int)(control.position.getY() * actualImageBounds.getHeight());

                    // Calculate actual rendered bounds based on control type
                    juce::Rectangle<float> controlBounds;

                    switch (control.type)
                    {
                    case GearControl::ControlType::Knob:
                    {
                        // Use the same logic as drawKnobControl() to calculate knob size
                        float knobSize;
                        if (control.loadedImage.isValid())
                        {
                            float originalWidth = (float)control.loadedImage.getWidth();
                            float originalHeight = (float)control.loadedImage.getHeight();
                            knobSize = std::max(originalWidth, originalHeight) * currentFaceplateScale;
                        }
                        else
                        {
                            const float baseKnobSize = 40.0f;
                            knobSize = baseKnobSize * currentFaceplateScale;
                        }
                        controlBounds = juce::Rectangle<float>(x, y, knobSize, knobSize);
                        break;
                    }
                    case GearControl::ControlType::Fader:
                    {
                        // DEBUG: Log fader detection
                        juce::Logger::writeToLog("DEBUG: Drawing red border for fader: " + control.name);
                        DBG("DEBUG: Drawing red border for fader: " + control.name);

                        // Use the exact same logic as drawFaderControl()
                        const bool isVertical = control.orientation == GearControl::Orientation::Vertical;
                        const float faderLength = control.length * currentFaceplateScale;

                        // Calculate handle size from the fader image (exactly like drawFaderControl)
                        float handleSize = 20.0f; // Default handle size
                        if (control.faderImage.isValid())
                        {
                            // Scale the handle relative to the scaled faceplate size
                            float imageWidth = (float)control.faderImage.getWidth();
                            float imageHeight = (float)control.faderImage.getHeight();
                            handleSize = std::max(imageWidth, imageHeight) * currentFaceplateScale;
                        }

                        // Scale the handle size based on the image's aspect ratio (exactly like drawFaderControl)
                        float imageWidth = (float)control.faderImage.getWidth();
                        float imageHeight = (float)control.faderImage.getHeight();
                        float aspectRatio = imageWidth / imageHeight;

                        float scaledWidth, scaledHeight;
                        if (isVertical)
                        {
                            scaledHeight = handleSize;
                            scaledWidth = handleSize * aspectRatio;
                        }
                        else
                        {
                            scaledWidth = handleSize;
                            scaledHeight = handleSize / aspectRatio;
                        }

                        // Use the exact scaled image dimensions for click target
                        // The handle can extend half its size beyond each end of the track
                        if (isVertical)
                        {
                            // Vertical fader: extend track length by half handle size at each end
                            float extendedLength = faderLength + scaledHeight;
                            controlBounds = juce::Rectangle<float>(x - scaledWidth / 2, y - scaledHeight / 2, scaledWidth, extendedLength);
                        }
                        else
                        {
                            // Horizontal fader: extend track length by half handle size at each end
                            float extendedLength = faderLength + scaledWidth;
                            controlBounds = juce::Rectangle<float>(x - scaledWidth / 2, y - scaledHeight / 2, extendedLength, scaledHeight);
                        }

                        // DEBUG: Log fader bounds
                        juce::Logger::writeToLog("DEBUG: Fader bounds: " + controlBounds.toString() +
                                                 ", isVertical: " + (isVertical ? "true" : "false") +
                                                 ", faderLength: " + juce::String(faderLength) +
                                                 ", scaledWidth: " + juce::String(scaledWidth) +
                                                 ", scaledHeight: " + juce::String(scaledHeight));
                        break;
                    }
                    case GearControl::ControlType::Button:
                    {
                        // Use the same logic as findControlAtPosition for button bounds
                        float buttonWidth, buttonHeight;
                        if (control.buttonSpriteSheet.isValid() && control.buttonFrames.size() > 0)
                        {
                            // Use the first frame's dimensions as the base size
                            buttonWidth = (float)control.buttonFrames[0].position.getWidth() * currentFaceplateScale;
                            buttonHeight = (float)control.buttonFrames[0].position.getHeight() * currentFaceplateScale;
                        }
                        else
                        {
                            // Fallback to standard size if no sprite sheet
                            const float baseButtonSize = 30.0f;
                            buttonWidth = baseButtonSize * currentFaceplateScale;
                            buttonHeight = baseButtonSize * currentFaceplateScale;
                        }
                        controlBounds = juce::Rectangle<float>(x, y, buttonWidth, buttonHeight);
                        break;
                    }
                    case GearControl::ControlType::Switch:
                    {
                        // Use the same logic as findControlAtPosition for switch bounds
                        float switchWidth, switchHeight;
                        if (control.switchSpriteSheet.isValid() && control.switchFrames.size() > 0)
                        {
                            // Use the first frame's dimensions as the base size
                            switchWidth = (float)control.switchFrames[0].position.getWidth() * currentFaceplateScale;
                            switchHeight = (float)control.switchFrames[0].position.getHeight() * currentFaceplateScale;
                        }
                        else
                        {
                            // Fallback to standard size if no sprite sheet
                            const float baseSwitchWidth = 30.0f;
                            const float baseSwitchHeight = 60.0f;
                            switchWidth = baseSwitchWidth * currentFaceplateScale;
                            switchHeight = baseSwitchHeight * currentFaceplateScale;
                        }
                        controlBounds = juce::Rectangle<float>(x, y, switchWidth, switchHeight);
                        break;
                    }
                    default:
                        // For other control types, use a default size
                        controlBounds = juce::Rectangle<float>(x, y, 40, 40);
                    }

                    // Draw red border around the click target
                    g.drawRect(controlBounds, 2.0f);
                }
            }
            else
            {
                // Reset scale factor when no faceplate
                currentFaceplateScale = 1.0f;

                // No faceplate, use the original rendering with name, manufacturer and thumbnail
                // Draw name
                g.setFont(16.0f);
                g.setColour(juce::Colours::white);
                juce::Rectangle<int> nameArea = getLocalBounds().reduced(10, 10);
                g.drawText(slotData->gearName, nameArea, juce::Justification::centred, true);

                // Draw manufacturer below name
                g.setFont(12.0f);
                g.setColour(juce::Colours::lightgrey);
                juce::Rectangle<int> mfgArea = nameArea.translated(0, 20);
                g.drawText(slotData->manufacturer, mfgArea, juce::Justification::centred, true);
            }
        }
    }
}

bool RackSlot::isEmpty() const
{
    if (auto *slotData = rackModel.getSlotData(index))
    {
        return !slotData->isOccupied;
    }
    return true;
}

/**
 * @brief Handles resizing of the rack slot component.
 *
 * Arranges the navigation buttons and other UI elements within the slot.
 */
void RackSlot::resized()
{
    auto area = getLocalBounds();

    // Position navigation buttons in dedicated top row
    int buttonRowHeight = 25; // Dedicated space at top for buttons
    int buttonWidth = 20;
    int buttonHeight = 18;
    int buttonMargin = 3;
    int buttonSpacing = 1;

    // Calculate total width needed for all three buttons
    int totalButtonWidth = (buttonWidth * 3) + (buttonSpacing * 2);

    // Up button (leftmost)
    if (upButton)
        upButton->setBounds(area.getRight() - totalButtonWidth - buttonMargin,
                            buttonMargin,
                            buttonWidth,
                            buttonHeight);

    // Down button (middle)
    if (downButton)
        downButton->setBounds(area.getRight() - totalButtonWidth + buttonWidth + buttonSpacing - buttonMargin,
                              buttonMargin,
                              buttonWidth,
                              buttonHeight);

    // Remove button (rightmost)
    if (removeButton)
        removeButton->setBounds(area.getRight() - buttonWidth - buttonMargin,
                                buttonMargin,
                                buttonWidth,
                                buttonHeight);
}

// Button handling
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
        removeGear();
    }
}

void RackSlot::updateButtonStates()
{
    if (rack == nullptr)
    {
        // If no rack reference, disable all buttons
        if (upButton)
            upButton->setEnabled(false);
        if (downButton)
            downButton->setEnabled(false);
        if (removeButton)
            removeButton->setEnabled(false);
        return;
    }

    int totalSlots = rackModel.getSlotCount();

    // Disable up button for first slot (index 0)
    if (upButton)
        upButton->setEnabled(index > 0);

    // Disable down button for last slot
    if (downButton)
        downButton->setEnabled(index < totalSlots - 1);

    // Remove button is enabled if there's gear in this slot
    if (removeButton)
        removeButton->setEnabled(!isEmpty());
}

void RackSlot::moveUp()
{
    if (rack == nullptr || index <= 0)
        return;

    // Move gear up by swapping with the slot above
    int targetSlot = index - 1;
    if (rackModel.moveGearBetweenSlots(index, targetSlot))
    {
        // Update indices after successful move
        // updateSlotIndices is now handled by RackModel
        // Recalculate layout to adjust slot heights for new gear items
        rack->resized();
        rack->repaint();
    }
}

void RackSlot::moveDown()
{
    if (rack == nullptr)
        return;

    // Get the total number of slots to check if we're at the last slot
    int totalSlots = rackModel.getSlotCount();
    if (index >= totalSlots - 1)
        return;

    // Move gear down by swapping with the slot below
    int targetSlot = index + 1;
    if (rackModel.moveGearBetweenSlots(index, targetSlot))
    {
        // Update indices after successful move
        // updateSlotIndices is now handled by RackModel
        // Recalculate layout to adjust slot heights for new gear items
        rack->resized();
        rack->repaint();
    }
}

void RackSlot::removeGear()
{
    if (rack == nullptr)
        return;

    // Remove the entire slot from the rack
    rackModel.removeGearFromSlot(index);
    // Note: After this call, this RackSlot object will be destroyed,
    // so we don't need to update indices or repaint here
}

// Mouse events for control interaction
void RackSlot::mouseDown(const juce::MouseEvent &e)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area (same as in paint method)
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate actual rendered image bounds (same as in paint method)
    float originalWidth = (float)gearItem->faceplateImage.getWidth();
    float originalHeight = (float)gearItem->faceplateImage.getHeight();
    float targetWidth = (float)faceplateArea.getWidth();
    float targetHeight = (float)faceplateArea.getHeight();

    float scaleX = targetWidth / originalWidth;
    float scaleY = targetHeight / originalHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    float scaledWidth = originalWidth * scaleFactor;
    float scaledHeight = originalHeight * scaleFactor;
    float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
    float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
    juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

    // Find control at mouse position
    activeControl = findControlAtPosition(e.position, actualImageBounds);
    if (activeControl != nullptr)
    {
        // Store drag start state for knobs, faders, and switches
        if (activeControl->type == GearControl::ControlType::Knob)
        {
            dragStartPos = e.position;
            lastMousePos = e.position; // Initialize for incremental movement
            dragStartValue = activeControl->currentValue;
            isDragging = true;
        }
        else if (activeControl->type == GearControl::ControlType::Fader)
        {
            dragStartPos = e.position;
            dragStartValue = activeControl->currentValue;
            isDragging = true;
        }
        else if (activeControl->type == GearControl::ControlType::Switch)
        {
            dragStartPos = e.position;
            dragStartValue = (float)activeControl->currentIndex;
            isDragging = true;
        }
        else if (activeControl->type == GearControl::ControlType::Button)
        {
            // Handle button click immediately
            handleButtonInteraction(*activeControl);
            repaint();

            // Notify the rack of the control change
            if (gearItem != nullptr)
            {
                for (int i = 0; i < gearItem->controls.size(); ++i)
                {
                    if (&gearItem->controls.getReference(i) == activeControl)
                    {
                        notifyRackOfControlChanged(i);
                        break;
                    }
                }
            }
        }
    }
}

void RackSlot::mouseDrag(const juce::MouseEvent &e)
{
    if (!isDragging || activeControl == nullptr)
        return;

    // Handle knob dragging
    if (activeControl->type == GearControl::ControlType::Knob)
    {
        handleKnobDrag(*activeControl, e);
    }
    // Handle fader dragging
    else if (activeControl->type == GearControl::ControlType::Fader)
    {
        handleFaderDrag(*activeControl, e);
    }
    // Handle switch dragging
    else if (activeControl->type == GearControl::ControlType::Switch)
    {
        handleSwitchDrag(*activeControl, e);
    }
    // Handle button dragging (for momentary buttons that respond to press/release)
    else if (activeControl->type == GearControl::ControlType::Button)
    {
        // For buttons, we don't need special drag handling since they're handled on click
        // But we can add it here if needed for future enhancements
    }
}

void RackSlot::mouseUp(const juce::MouseEvent &e)
{
    // Support both Ctrl/Cmd + Click and Alt/Option + Click for reset
    if (e.mods.isCtrlDown() || e.mods.isCommandDown() || e.mods.isAltDown())
    {
        resetControlToDefault(e);
    }

    // Handle normal mouse up operations (drag completion)
    if (isDragging)
    {
        isDragging = false;
    }
    activeControl = nullptr;
}

void RackSlot::mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area (same as in mouseDown)
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate actual rendered image bounds (same as in mouseDown)
    float originalWidth = (float)gearItem->faceplateImage.getWidth();
    float originalHeight = (float)gearItem->faceplateImage.getHeight();
    float targetWidth = (float)faceplateArea.getWidth();
    float targetHeight = (float)faceplateArea.getHeight();

    float scaleX = targetWidth / originalWidth;
    float scaleY = targetHeight / originalHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    float scaledWidth = originalWidth * scaleFactor;
    float scaledHeight = originalHeight * scaleFactor;
    float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
    float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
    juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

    // Find control at mouse position
    if (auto *control = findControlAtPosition(e.position, actualImageBounds))
    {
        if (control->type == GearControl::ControlType::Knob)
        {
            // Set up drag state for wheel movement (like mouseDown)
            dragStartValue = control->currentValue;
            dragStartPos = e.position;

            // Scale wheel movement to knob sensitivity
            float deltaAngle = wheel.deltaY * KNOB_WHEEL_SENSITIVITY * KNOB_WHEEL_SENSITIVITY_STEP;

            // Use the consolidated method to update the knob value
            updateKnobValue(*control, deltaAngle, "WHEEL");
        }
        else if (control->type == GearControl::ControlType::Fader)
        {
            // Set up drag state for wheel movement (like mouseDown)
            dragStartValue = control->currentValue;
            dragStartPos = e.position;

            // Scale wheel movement to fader sensitivity
            float deltaValue = wheel.deltaY * FADER_WHEEL_SENSITIVITY;

            // Use the consolidated method to update the fader value
            updateFaderValue(*control, deltaValue, "WHEEL");
        }
        else if (control->type == GearControl::ControlType::Switch)
        {
            // Handle switch wheel movement
            int currentIndex = control->currentIndex;
            int numOptions = control->options.size();

            if (numOptions > 1)
            {
                // Determine direction based on wheel movement
                int direction = wheel.deltaY > 0 ? 1 : -1;

                // For vertical switches, invert the direction to match natural scrolling
                if (control->orientation == GearControl::Orientation::Vertical)
                {
                    direction = -direction;
                }

                int newIndex = currentIndex + direction;

                // Clamp to valid range (no looping)
                newIndex = juce::jlimit(0, numOptions - 1, newIndex);

                if (newIndex != currentIndex)
                {
                    control->currentIndex = newIndex;
                    control->currentValue = (float)newIndex;
                    repaint();

                    // Notify the rack of the control change
                    if (gearItem != nullptr)
                    {
                        for (int i = 0; i < gearItem->controls.size(); ++i)
                        {
                            if (&gearItem->controls.getReference(i) == control)
                            {
                                notifyRackOfControlChanged(i);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// DragAndDropTarget methods
bool RackSlot::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Only accept gear item drag sources
    juce::String description = dragSourceDetails.description.toString();
    return description.startsWith("gear:");
}

void RackSlot::itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    isDragOver = true;
    repaint(); // Trigger repaint to show visual feedback
}

void RackSlot::itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Visual feedback can be enhanced here later
}

void RackSlot::itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    isDragOver = false;
    repaint(); // Trigger repaint to hide visual feedback
}

void RackSlot::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    juce::Logger::writeToLog("RackSlot::itemDropped called");

    isDragOver = false;
    repaint();

    // Extract gear ID from drag description
    juce::String description = dragSourceDetails.description.toString();
    juce::Logger::writeToLog("RackSlot::itemDropped - description: " + description);

    // Check if this is a gear item drop
    if (description.startsWith("gear:"))
    {
        juce::Logger::writeToLog("RackSlot::itemDropped - description starts with 'gear:'");
        juce::String gearId = description.substring(5); // Remove "gear:" prefix
        juce::Logger::writeToLog("RackSlot::itemDropped - extracted gearId: " + gearId);

        // Get the gear item from the library
        GearItem *gearItem = rackModel.getGearLibrary().getGearItem(gearId);
        juce::Logger::writeToLog("RackSlot::itemDropped - gearItem found: " + juce::String(gearItem != nullptr ? "YES" : "NO"));

        if (gearItem)
        {
            juce::Logger::writeToLog("RackSlot::itemDropped - checking if slot is empty");
            // Check if slot is empty
            if (isEmpty())
            {
                juce::Logger::writeToLog("RackSlot::itemDropped - slot is empty, proceeding with drop");
                // Set the gear item in this slot
                // Gear item management now handled by RackModel - trigger UI update
                updateButtonStates();
                repaint();
                juce::Logger::writeToLog("RackSlot::itemDropped - setGearItem completed");

                // Log successful drop
            }
            else
            {
                juce::Logger::writeToLog("RackSlot::itemDropped - slot is occupied, delegating to parent");
                // Slot is occupied - delegate to parent Rack for insertion logic

                // Find the parent Rack component and delegate the drop
                juce::Component *parent = getParentComponent();
                while (parent != nullptr)
                {
                    // Check if parent is a Rack (by class type, not just ID)
                    if (parent->getComponentID() == "Rack" || dynamic_cast<Rack *>(parent) != nullptr)
                    {
                        juce::Logger::writeToLog("RackSlot::itemDropped - found parent Rack, delegating drop");
                        // Convert the drop position to parent coordinates
                        juce::DragAndDropTarget::SourceDetails parentDetails = dragSourceDetails;
                        parentDetails.localPosition = parent->getLocalPoint(this, dragSourceDetails.localPosition);

                        // Delegate to parent's itemDropped method
                        if (auto *rackTarget = dynamic_cast<juce::DragAndDropTarget *>(parent))
                        {
                            rackTarget->itemDropped(parentDetails);
                        }
                        return;
                    }
                    parent = parent->getParentComponent();
                }

                juce::Logger::writeToLog("RackSlot::itemDropped - no parent Rack found, showing error");
                // If no parent Rack found, show error
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Slot Occupied",
                    "This slot already contains gear. Please choose an empty slot.",
                    "OK");
            }
        }
        else
        {
            juce::Logger::writeToLog("RackSlot::itemDropped - gearItem not found in library");
        }
    }
    else
    {
        juce::Logger::writeToLog("RackSlot::itemDropped - description does not start with 'gear:'");
    }

    juce::Logger::writeToLog("RackSlot::itemDropped - method completed");
}

// Gear item management now handled by RackModel

// Control rendering

void RackSlot::drawControls(juce::Graphics &g, const juce::Rectangle<float> &actualImageBounds)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr)
        return;

    for (const auto &control : gearItem->controls)
    {
        // Calculate control position relative to actual rendered image bounds
        int x = actualImageBounds.getX() + (int)(control.position.getX() * actualImageBounds.getWidth());
        int y = actualImageBounds.getY() + (int)(control.position.getY() * actualImageBounds.getHeight());

        // Draw control based on type
        switch (control.type)
        {
        case GearControl::ControlType::Switch:
            drawSwitchControl(g, control, x, y);
            break;
        case GearControl::ControlType::Button:
            drawButtonControl(g, control, x, y);
            break;
        case GearControl::ControlType::Fader:
            drawFaderControl(g, control, x, y);
            break;
        case GearControl::ControlType::Knob:
            drawKnobControl(g, control, x, y);
            break;
        }
    }
}

void RackSlot::drawButtonControl(juce::Graphics &g, const GearControl &control, int x, int y)
{
    // Calculate base dimensions from the sprite sheet if available
    float baseWidth, baseHeight;
    juce::Image scaledSpriteSheet; // Move declaration to outer scope

    if (control.buttonSpriteSheet.isValid() && control.buttonFrames.size() > 0)
    {
        // Get the original sprite sheet dimensions
        float originalSpriteWidth = (float)control.buttonSpriteSheet.getWidth();
        float originalSpriteHeight = (float)control.buttonSpriteSheet.getHeight();

        // Scale the sprite sheet dimensions by the faceplate scale
        float scaledSpriteWidth = originalSpriteWidth * currentFaceplateScale;
        float scaledSpriteHeight = originalSpriteHeight * currentFaceplateScale;

        // Create a scaled version of the sprite sheet
        scaledSpriteSheet = control.buttonSpriteSheet.rescaled(
            (int)scaledSpriteWidth,
            (int)scaledSpriteHeight,
            juce::Graphics::ResamplingQuality::highResamplingQuality);

        // Use the first frame's dimensions as the base size
        baseWidth = (float)control.buttonFrames[0].position.getWidth();
        baseHeight = (float)control.buttonFrames[0].position.getHeight();
    }
    else
    {
        // Fallback to standard size if no sprite sheet
        baseWidth = 30.0f;
        baseHeight = 30.0f;
    }

    // Scale dimensions based on faceplate scale
    const float buttonWidth = baseWidth * currentFaceplateScale;
    const float buttonHeight = baseHeight * currentFaceplateScale;

    // Draw the button using sprite sheet if available
    if (control.buttonSpriteSheet.isValid() && control.buttonFrames.size() > 0)
    {
        // Use the currentIndex to determine which frame to use
        int frameIndex = control.currentIndex;

        // Ensure frame index is valid
        if (frameIndex >= control.buttonFrames.size())
        {
            frameIndex = 0;
        }

        // Get the frame data
        const auto &frame = control.buttonFrames[frameIndex];

        // Scale the frame coordinates and dimensions by the faceplate scale
        float scaledFrameX = frame.position.getX() * currentFaceplateScale;
        float scaledFrameY = frame.position.getY() * currentFaceplateScale;
        float scaledFrameWidth = frame.position.getWidth() * currentFaceplateScale;
        float scaledFrameHeight = frame.position.getHeight() * currentFaceplateScale;

        // Create the source rectangle using the scaled coordinates and dimensions
        juce::Rectangle<int> sourceRect(
            (int)scaledFrameX,
            (int)scaledFrameY,
            (int)scaledFrameWidth,
            (int)scaledFrameHeight);

        // Create the destination rectangle at the control position
        juce::Rectangle<float> destRect(x, y, buttonWidth, buttonHeight);

        // Draw the frame from the scaled sprite sheet
        g.drawImage(scaledSpriteSheet,
                    destRect.getX(), destRect.getY(),
                    destRect.getWidth(), destRect.getHeight(),
                    sourceRect.getX(), sourceRect.getY(),
                    sourceRect.getWidth(), sourceRect.getHeight());
    }
    else
    {
        // Fallback to basic button drawing if no sprite sheet is available
        g.setColour(control.currentValue > 0.5f ? juce::Colours::red : juce::Colours::darkgrey);
        g.fillRoundedRectangle(x, y, buttonWidth, buttonHeight, 4.0f);

        g.setColour(juce::Colours::grey);
        g.drawRoundedRectangle(x, y, buttonWidth, buttonHeight, 4.0f, 2.0f);

        // Draw button label
        if (control.name.isNotEmpty())
        {
            g.setColour(juce::Colours::white);
            g.setFont(10.0f);
            g.drawText(control.name, x, y, (int)buttonWidth, (int)buttonHeight, juce::Justification::centred);
        }
    }
}

void RackSlot::drawFaderControl(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const bool isVertical = control.orientation == GearControl::Orientation::Vertical;

    // 1. Scale the track length from the schema length property relative to the scaled faceplate size
    const float faderLength = control.length * currentFaceplateScale;

    // 2. The other track dimension is always 1 pixel
    const float trackWidth = 1.0f;

    // 3. Calculate handle size from the fader image (only for the handle, not the track)
    float handleSize = 20.0f; // Default handle size
    if (control.faderImage.isValid())
    {
        // Scale the handle relative to the scaled faceplate size
        float imageWidth = (float)control.faderImage.getWidth();
        float imageHeight = (float)control.faderImage.getHeight();
        handleSize = std::max(imageWidth, imageHeight) * currentFaceplateScale;
    }

    // Calculate the fader track bounds based on orientation
    juce::Rectangle<float> trackBounds;
    if (isVertical)
    {
        trackBounds = juce::Rectangle<float>(x, y, trackWidth, faderLength);
    }
    else
    {
        trackBounds = juce::Rectangle<float>(x, y, faderLength, trackWidth);
    }

    // 4. Place the handle on the track at the correct percentage represented in the schema as "value"
    float handleX, handleY;
    if (isVertical)
    {
        handleX = x;
        handleY = y + (1.0f - control.currentValue) * faderLength;
    }
    else
    {
        handleX = x + control.currentValue * faderLength;
        handleY = y;
    }

    // Draw the fader handle image at the handle position
    if (control.faderImage.isValid())
    {
        // Scale the handle size based on the image's aspect ratio
        float imageWidth = (float)control.faderImage.getWidth();
        float imageHeight = (float)control.faderImage.getHeight();
        float aspectRatio = imageWidth / imageHeight;

        float scaledWidth, scaledHeight;
        if (isVertical)
        {
            scaledHeight = handleSize;
            scaledWidth = handleSize * aspectRatio;
        }
        else
        {
            scaledWidth = handleSize;
            scaledHeight = handleSize / aspectRatio;
        }

        // Draw the fader handle image centered at the handle position
        g.drawImageWithin(control.faderImage,
                          handleX - scaledWidth / 2,
                          handleY - scaledHeight / 2,
                          scaledWidth,
                          scaledHeight,
                          juce::RectanglePlacement::centred);
    }
    else
    {
        // Fallback to basic handle drawing if no image is provided
        g.setColour(juce::Colours::white);
        if (isVertical)
        {
            g.fillRoundedRectangle(handleX - handleSize / 2, handleY - handleSize / 2,
                                   handleSize, handleSize, 4.0f);
        }
        else
        {
            g.fillRoundedRectangle(handleX - handleSize / 2, handleY - handleSize / 2,
                                   handleSize, handleSize, 4.0f);
        }
    }
}

void RackSlot::drawSwitchControl(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const bool isVertical = control.orientation == GearControl::Orientation::Vertical;
    const int currentIndex = control.currentIndex;

    // If we have a valid sprite sheet and frame data
    if (control.switchSpriteSheet.isValid() && control.switchFrames.size() > 0)
    {
        // Get the current frame
        if (currentIndex >= 0 && currentIndex < control.switchFrames.size())
        {
            const auto &frame = control.switchFrames[currentIndex];

            // Get the original sprite sheet dimensions
            float originalSpriteWidth = (float)control.switchSpriteSheet.getWidth();
            float originalSpriteHeight = (float)control.switchSpriteSheet.getHeight();

            // Scale the sprite sheet dimensions by the faceplate scale
            float scaledSpriteWidth = originalSpriteWidth * currentFaceplateScale;
            float scaledSpriteHeight = originalSpriteHeight * currentFaceplateScale;

            // Create a scaled version of the sprite sheet
            juce::Image scaledSpriteSheet = control.switchSpriteSheet.rescaled(
                (int)scaledSpriteWidth,
                (int)scaledSpriteHeight,
                juce::Graphics::ResamplingQuality::highResamplingQuality);

            // Scale the frame coordinates and dimensions by the faceplate scale
            float scaledFrameX = frame.position.getX() * currentFaceplateScale;
            float scaledFrameY = frame.position.getY() * currentFaceplateScale;
            float scaledFrameWidth = frame.position.getWidth() * currentFaceplateScale;
            float scaledFrameHeight = frame.position.getHeight() * currentFaceplateScale;

            // Create the source rectangle using the scaled coordinates and dimensions
            juce::Rectangle<int> sourceRect(
                (int)scaledFrameX,
                (int)scaledFrameY,
                (int)scaledFrameWidth,
                (int)scaledFrameHeight);

            // Create the destination rectangle at the control position
            juce::Rectangle<float> destRect(x, y, scaledFrameWidth, scaledFrameHeight);

            // Draw the frame from the scaled sprite sheet
            g.drawImage(scaledSpriteSheet,
                        destRect.getX(), destRect.getY(),
                        destRect.getWidth(), destRect.getHeight(),
                        sourceRect.getX(), sourceRect.getY(),
                        sourceRect.getWidth(), sourceRect.getHeight());
        }
    }
    else
    {
        // Fallback to basic switch drawing if no sprite sheet is available
        const int switchWidth = 30;
        const int switchHeight = 60;

        g.setColour(control.currentValue > 0.5f ? juce::Colours::red : juce::Colours::darkgrey);
        g.fillRoundedRectangle(x, y, switchWidth, switchHeight, 4.0f);

        g.setColour(juce::Colours::grey);
        g.drawRoundedRectangle(x, y, switchWidth, switchHeight, 4.0f, 2.0f);

        // Draw position indicator
        if (control.options.size() > 0)
        {
            float indicatorX = x + (currentIndex * (switchWidth / control.options.size()));
            g.fillRoundedRectangle(indicatorX + 4, y + 4, (switchWidth / control.options.size()) - 8, switchHeight - 8, 2.0f);
        }
    }
}

void RackSlot::drawKnobControl(juce::Graphics &g, const GearControl &control, int x, int y)
{
    // Calculate knob size based on faceplate scale factor
    float knobSize;
    if (control.loadedImage.isValid())
    {
        // Use the original image dimensions as the base size
        float originalWidth = (float)control.loadedImage.getWidth();
        float originalHeight = (float)control.loadedImage.getHeight();
        // Use the larger dimension to ensure the knob is properly sized
        knobSize = std::max(originalWidth, originalHeight) * currentFaceplateScale;
    }
    else
    {
        // Fallback to standard size if no image
        const float baseKnobSize = 40.0f;
        knobSize = baseKnobSize * currentFaceplateScale;
    }

    // Create knob bounds using the transformed coordinates and scaled size
    juce::Rectangle<float> knobBounds(x, y, knobSize, knobSize);

    // Draw the knob image if available
    if (control.loadedImage.isValid())
    {
        // Save the current graphics state
        g.saveState();

        // Use the control value directly as degrees
        // Convert to JUCE coordinate system (add 180° to align with real knob behavior)
        float angle = control.currentValue + 180.0f;

        // Translate to the center of the knob
        g.addTransform(juce::AffineTransform::translation(knobBounds.getCentreX(), knobBounds.getCentreY()));

        // Rotate around the center
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(angle)));

        // Translate back and draw the image centered
        g.addTransform(juce::AffineTransform::translation(-knobBounds.getCentreX(), -knobBounds.getCentreY()));

        // Draw the image
        g.drawImageWithin(control.loadedImage,
                          knobBounds.getX(), knobBounds.getY(),
                          knobBounds.getWidth(), knobBounds.getHeight(),
                          juce::RectanglePlacement::centred);

        // Restore the graphics state
        g.restoreState();
    }
    else
    {
        // Fallback to basic drawing if no image is provided
        g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colours::black);
        g.drawEllipse(knobBounds, 1.0f);

        // Draw position indicator
        g.setColour(juce::Colours::white);
        // Use the control value directly as degrees (same as image drawing)
        float angle = control.currentValue + 180.0f; // Convert to JUCE coordinate system
        float radius = knobBounds.getWidth() * 0.4f;
        float centreX = knobBounds.getCentreX();
        float centreY = knobBounds.getCentreY();

        // Convert angle to radians
        float angleRad = angle * (juce::MathConstants<float>::pi / 180.0f);
        float endX = centreX + radius * std::cos(angleRad);
        float endY = centreY + radius * std::sin(angleRad);

        g.drawLine(centreX, centreY, endX, endY, 2.0f);
    }
}

void RackSlot::setFaceplateLoadedCallback(std::function<void()> callback)
{
    faceplateLoadedCallback = callback;
}

void RackSlot::triggerFaceplateLoaded()
{
    if (faceplateLoadedCallback)
    {
        faceplateLoadedCallback();
    }
}

void RackSlot::setSlotBackgroundColor(juce::Colour color)
{
    slotBackgroundColor = color;
    repaint();
}

// Helper methods for control interaction
GearControl *RackSlot::findControlAtPosition(const juce::Point<float> &position, const juce::Rectangle<float> &actualImageBounds)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return nullptr;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr)
        return nullptr;

    for (auto &control : gearItem->controls)
    {
        // Calculate control bounds
        int x = actualImageBounds.getX() + (int)(control.position.getX() * actualImageBounds.getWidth());
        int y = actualImageBounds.getY() + (int)(control.position.getY() * actualImageBounds.getHeight());

        // Calculate actual rendered bounds based on control type
        juce::Rectangle<float> controlBounds;

        switch (control.type)
        {
        case GearControl::ControlType::Knob:
        {
            // Use the same logic as drawKnobControl() to calculate knob size
            float knobSize;
            if (control.loadedImage.isValid())
            {
                // Use the original image dimensions as the base size (same as drawing)
                float originalWidth = (float)control.loadedImage.getWidth();
                float originalHeight = (float)control.loadedImage.getHeight();
                // Use the larger dimension to ensure the knob is properly sized
                knobSize = std::max(originalWidth, originalHeight) * currentFaceplateScale;
            }
            else
            {
                // Fallback to standard size if no image (same as drawing)
                const float baseKnobSize = 40.0f;
                knobSize = baseKnobSize * currentFaceplateScale;
            }
            controlBounds = juce::Rectangle<float>(x, y, knobSize, knobSize);
            break;
        }
        case GearControl::ControlType::Fader:
        {
            // Use the exact same logic as drawFaderControl()
            const bool isVertical = control.orientation == GearControl::Orientation::Vertical;
            const float faderLength = control.length * currentFaceplateScale;

            // Calculate handle size from the fader image (exactly like drawFaderControl)
            float handleSize = 20.0f; // Default handle size
            if (control.faderImage.isValid())
            {
                // Scale the handle relative to the scaled faceplate size
                float imageWidth = (float)control.faderImage.getWidth();
                float imageHeight = (float)control.faderImage.getHeight();
                handleSize = std::max(imageWidth, imageHeight) * currentFaceplateScale;
            }

            // Scale the handle size based on the image's aspect ratio (exactly like drawFaderControl)
            float imageWidth = (float)control.faderImage.getWidth();
            float imageHeight = (float)control.faderImage.getHeight();
            float aspectRatio = imageWidth / imageHeight;

            float scaledWidth, scaledHeight;
            if (isVertical)
            {
                scaledHeight = handleSize;
                scaledWidth = handleSize * aspectRatio;
            }
            else
            {
                scaledWidth = handleSize;
                scaledHeight = handleSize / aspectRatio;
            }

            // Use the exact scaled image dimensions for click target
            // The handle can extend half its size beyond each end of the track
            if (isVertical)
            {
                // Vertical fader: extend track length by half handle size at each end
                float extendedLength = faderLength + scaledHeight;
                controlBounds = juce::Rectangle<float>(x - scaledWidth / 2, y - scaledHeight / 2, scaledWidth, extendedLength);
            }
            else
            {
                // Horizontal fader: extend track length by half handle size at each end
                float extendedLength = faderLength + scaledWidth;
                controlBounds = juce::Rectangle<float>(x - scaledWidth / 2, y - scaledHeight / 2, extendedLength, scaledHeight);
            }
            break;
        }
        case GearControl::ControlType::Button:
        {
            // Use the same logic as drawButtonControl() to calculate button size
            float buttonWidth, buttonHeight;
            if (control.buttonSpriteSheet.isValid() && control.buttonFrames.size() > 0)
            {
                // Use the first frame's dimensions as the base size (same as drawing)
                buttonWidth = (float)control.buttonFrames[0].position.getWidth() * currentFaceplateScale;
                buttonHeight = (float)control.buttonFrames[0].position.getHeight() * currentFaceplateScale;
            }
            else
            {
                // Fallback to standard size if no sprite sheet (same as drawing)
                const float baseButtonSize = 30.0f;
                buttonWidth = baseButtonSize * currentFaceplateScale;
                buttonHeight = baseButtonSize * currentFaceplateScale;
            }
            controlBounds = juce::Rectangle<float>(x, y, buttonWidth, buttonHeight);
            break;
        }
        case GearControl::ControlType::Switch:
        {
            // Use the same logic as drawSwitchControl() to calculate switch size
            float switchWidth, switchHeight;
            if (control.switchSpriteSheet.isValid() && control.switchFrames.size() > 0)
            {
                // Use the first frame's dimensions as the base size (same as drawing)
                switchWidth = (float)control.switchFrames[0].position.getWidth() * currentFaceplateScale;
                switchHeight = (float)control.switchFrames[0].position.getHeight() * currentFaceplateScale;
            }
            else
            {
                // Fallback to standard size if no sprite sheet (same as drawing)
                const float baseSwitchWidth = 30.0f;
                const float baseSwitchHeight = 60.0f;
                switchWidth = baseSwitchWidth * currentFaceplateScale;
                switchHeight = baseSwitchHeight * currentFaceplateScale;
            }
            controlBounds = juce::Rectangle<float>(x, y, switchWidth, switchHeight);
            break;
        }
        default:
            // For other control types, use a default size
            controlBounds = juce::Rectangle<float>(x, y, 40, 40);
        }

        // Check if position is within the actual rendered bounds
        if (controlBounds.contains(position))
            return &control;
    }

    return nullptr;
}

void RackSlot::resetControlToDefault(const juce::MouseEvent &e)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area (same as in mouseDown)
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate actual rendered image bounds (same as in mouseDown)
    float originalWidth = (float)gearItem->faceplateImage.getWidth();
    float originalHeight = (float)gearItem->faceplateImage.getHeight();
    float targetWidth = (float)faceplateArea.getWidth();
    float targetHeight = (float)faceplateArea.getHeight();

    float scaleX = targetWidth / originalWidth;
    float scaleY = targetHeight / originalHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    float scaledWidth = originalWidth * scaleFactor;
    float scaledHeight = originalHeight * scaleFactor;
    float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
    float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
    juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

    // Find control at mouse position
    if (auto *control = findControlAtPosition(e.position, actualImageBounds))
    {
        juce::Logger::writeToLog("RackSlot::resetControlToDefault - Resetting control: " + control->name + " to default value: " + juce::String(control->initialValue));

        // Reset control to default value
        switch (control->type)
        {
        case GearControl::ControlType::Knob:
            control->currentValue = control->initialValue;
            break;
        case GearControl::ControlType::Fader:
            control->currentValue = control->initialValue;
            break;
        case GearControl::ControlType::Button:
            control->currentValue = control->initialValue;
            control->currentIndex = (int)control->initialValue;
            break;
        case GearControl::ControlType::Switch:
            control->currentValue = control->initialValue;
            control->currentIndex = (int)control->initialValue;
            break;
        default:
            // For other control types, reset as needed
            control->currentValue = control->initialValue;
            break;
        }
        repaint();
    }
}

void RackSlot::handleKnobInteraction(GearControl &control, const juce::MouseEvent &e)
{
    // This method can be used for other knob interactions if needed
    juce::Logger::writeToLog("RackSlot::handleKnobInteraction - Control: " + control.name);
}

void RackSlot::handleKnobDrag(GearControl &control, const juce::MouseEvent &e)
{
    // Calculate vertical movement since drag start
    float deltaY = dragStartPos.y - e.position.y;

    // Scale the movement to control sensitivity
    float deltaAngle = deltaY * KNOB_DRAG_SENSITIVITY;

    // Use the consolidated method to update the knob value
    updateKnobValue(control, deltaAngle, "DRAG");
}

void RackSlot::handleKnobReset(GearControl &control)
{
    control.currentValue = control.initialValue;
    juce::Logger::writeToLog("RackSlot::handleKnobReset - Control: " + control.name + " reset to: " + juce::String(control.initialValue));
    repaint();
}

void RackSlot::updateKnobValue(GearControl &control, float deltaAngle, const juce::String &source)
{
    // Update the control value (angle in degrees)
    float newValue = dragStartValue + deltaAngle;

    // Clamp the value between startAngle and endAngle
    newValue = juce::jlimit(control.startAngle, control.endAngle, newValue);

    // If this is a stepped knob, snap to the nearest step
    if (!control.steps.isEmpty())
    {
        float closestStep = control.steps[0];
        float minDistance = std::abs(newValue - closestStep);

        // Find the closest step angle
        for (float step : control.steps)
        {
            float distance = std::abs(newValue - step);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestStep = step;
            }
        }

        newValue = closestStep;
    }

    control.currentValue = newValue;
    repaint();
}

void RackSlot::handleFaderDrag(GearControl &control, const juce::MouseEvent &e)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    // Calculate faceplate area (same as in paint method)
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate actual rendered image bounds (same as in paint method)
    float originalWidth = (float)gearItem->faceplateImage.getWidth();
    float originalHeight = (float)gearItem->faceplateImage.getHeight();
    float targetWidth = (float)faceplateArea.getWidth();
    float targetHeight = (float)faceplateArea.getHeight();

    float scaleX = targetWidth / originalWidth;
    float scaleY = targetHeight / originalHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    float scaledWidth = originalWidth * scaleFactor;
    float scaledHeight = originalHeight * scaleFactor;
    float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
    float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
    juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

    // Calculate control bounds
    int x = actualImageBounds.getX() + (int)(control.position.getX() * actualImageBounds.getWidth());
    int y = actualImageBounds.getY() + (int)(control.position.getY() * actualImageBounds.getHeight());

    const bool isVertical = control.orientation == GearControl::Orientation::Vertical;
    const float faderLength = control.length * scaleFactor;
    float newValue;

    if (isVertical)
    {
        // For vertical faders, use Y position relative to fader track
        // Account for the handle being centered at the track position
        float trackY = y;
        float trackHeight = faderLength;
        float normalizedY = 1.0f - (float)(e.position.y - trackY) / trackHeight;
        newValue = juce::jlimit(0.0f, 1.0f, normalizedY);
    }
    else
    {
        // For horizontal faders, use X position relative to fader track
        // Account for the handle being centered at the track position
        float trackX = x;
        float trackWidth = faderLength;
        float normalizedX = (float)(e.position.x - trackX) / trackWidth;
        newValue = juce::jlimit(0.0f, 1.0f, normalizedX);
    }

    control.currentValue = newValue;
    repaint();
}

void RackSlot::handleSwitchDrag(GearControl &control, const juce::MouseEvent &e)
{
    // Get gear item from RackModel
    auto *slotData = rackModel.getSlotData(index);
    if (!slotData || !slotData->isOccupied)
        return;

    auto *gearItem = rackModel.getGearLibrary().getGearItem(slotData->gearId);
    if (gearItem == nullptr || !gearItem->faceplateImage.isValid())
        return;

    const bool isVertical = control.orientation == GearControl::Orientation::Vertical;
    const int numOptions = control.options.size();

    if (numOptions <= 1)
        return;

    // Calculate faceplate area (same as in paint method)
    juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);
    faceplateArea.removeFromTop(20); // Remove space for name

    // Calculate actual rendered image bounds (same as in paint method)
    float originalWidth = (float)gearItem->faceplateImage.getWidth();
    float originalHeight = (float)gearItem->faceplateImage.getHeight();
    float targetWidth = (float)faceplateArea.getWidth();
    float targetHeight = (float)faceplateArea.getHeight();

    float scaleX = targetWidth / originalWidth;
    float scaleY = targetHeight / originalHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    float scaledWidth = originalWidth * scaleFactor;
    float scaledHeight = originalHeight * scaleFactor;
    float imageX = faceplateArea.getX() + (faceplateArea.getWidth() - scaledWidth) / 2;
    float imageY = faceplateArea.getY() + (faceplateArea.getHeight() - scaledHeight) / 2;
    juce::Rectangle<float> actualImageBounds(imageX, imageY, scaledWidth, scaledHeight);

    // Calculate control bounds
    int x = actualImageBounds.getX() + (int)(control.position.getX() * actualImageBounds.getWidth());
    int y = actualImageBounds.getY() + (int)(control.position.getY() * actualImageBounds.getHeight());

    // Calculate the drag distance along the orientation axis
    float dragDistance;
    if (isVertical)
    {
        dragDistance = e.position.y - dragStartPos.y;
        // For vertical switches, invert the drag direction to match natural movement
        dragDistance = -dragDistance;
    }
    else
    {
        dragDistance = e.position.x - dragStartPos.x;
    }

    // Calculate the total range of movement using a reasonable switch length
    // For switches, we'll use a fixed length since they don't have a length property like faders
    const float switchLength = 120.0f * scaleFactor; // Increased switch length for better sensitivity
    float totalRange = switchLength;
    float optionSize = totalRange / numOptions;

    // Calculate the new index based on drag distance
    float newIndex = dragStartValue + (dragDistance / optionSize);

    // Clamp the index to valid range and round to nearest option
    newIndex = juce::jlimit(0.0f, (float)(numOptions - 1), newIndex);
    int newIndexInt = juce::roundToInt(newIndex);

    // Update the control
    if (newIndexInt != control.currentIndex)
    {
        control.currentIndex = newIndexInt;
        control.currentValue = (float)newIndexInt;
        repaint();

        // Notify the rack of the control change
        if (gearItem != nullptr)
        {
            for (int i = 0; i < gearItem->controls.size(); ++i)
            {
                if (&gearItem->controls.getReference(i) == &control)
                {
                    notifyRackOfControlChanged(i);
                    break;
                }
            }
        }
    }
}

void RackSlot::updateFaderValue(GearControl &control, float deltaValue, const juce::String &source)
{
    // Calculate new value using dragStartValue for all faders
    float newValue = dragStartValue + deltaValue;

    // Clamp the value between 0.0 and 1.0
    newValue = juce::jlimit(0.0f, 1.0f, newValue);

    control.currentValue = newValue;
    repaint();
}

void RackSlot::handleButtonInteraction(GearControl &control)
{
    if (control.isMomentary)
    {
        // For momentary buttons, toggle between on (1.0) and off (0.0) states
        control.currentValue = control.currentValue > 0.5f ? 0.0f : 1.0f;
        control.currentIndex = (int)control.currentValue;
    }
    else
    {
        // For latching buttons, cycle through all available options
        if (control.buttonFrames.size() > 0)
        {
            control.currentIndex = (control.currentIndex + 1) % control.buttonFrames.size();
            control.currentValue = (float)control.currentIndex;
        }
        else if (control.options.size() > 0)
        {
            // Fallback to options array if no frames available
            control.currentIndex = (control.currentIndex + 1) % control.options.size();
            control.currentValue = (float)control.currentIndex;
        }
    }
}

void RackSlot::notifyRackOfControlChanged(int controlIndex)
{
    // Notify the parent rack that a control has changed
    if (rack != nullptr)
    {
        // This would typically call a method on the rack to handle the control change
        // the actual implementation would depend on the Rack class
    }
}
