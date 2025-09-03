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
RackSlot::RackSlot(IFileSystem &fileSystem,
                   ICacheManager &cacheManager,
                   PresetManager &presetManager,
                   GearLibrary &gearLibrary,
                   int slotIndex)
    : index(slotIndex),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(gearLibrary)
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

    // Draw gear item info if present
    if (gearItem != nullptr)
    {
        // Draw faceplate image if available
        if (gearItem->faceplateImage.isValid())
        {
            // Calculate optimal scale for faceplate
            currentFaceplateScale = calculateOptimalFaceplateScale();

            // Calculate scaled dimensions
            int scaledWidth = static_cast<int>(gearItem->faceplateImage.getWidth() * currentFaceplateScale);
            int scaledHeight = static_cast<int>(gearItem->faceplateImage.getHeight() * currentFaceplateScale);

            // Position faceplate to fill the available space (scaling already accounts for padding)
            int buttonRowHeight = 25; // Dedicated space at top for buttons
            int slotPadding = 2;      // 2px padding on left, right, and bottom
            int effectiveWidth = getWidth() - (2 * slotPadding);
            int effectiveHeight = getHeight() - buttonRowHeight - slotPadding;
            int x = slotPadding + (effectiveWidth - scaledWidth) / 2;
            int y = buttonRowHeight + (effectiveHeight - scaledHeight) / 2;

            // Draw the faceplate image
            g.drawImage(gearItem->faceplateImage, x, y, scaledWidth, scaledHeight, 0, 0,
                        gearItem->faceplateImage.getWidth(), gearItem->faceplateImage.getHeight());
        }

        // Draw controls on top of the faceplate
        drawControls(g);
    }
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

    // Calculate total width needed for both buttons
    int totalButtonWidth = (buttonWidth * 2) + buttonSpacing;

    // Up button (left of the pair) in top row
    if (upButton)
        upButton->setBounds(area.getRight() - totalButtonWidth - buttonMargin,
                            buttonMargin,
                            buttonWidth,
                            buttonHeight);

    // Down button (right of the pair) in top row
    if (downButton)
        downButton->setBounds(area.getRight() - buttonWidth - buttonMargin,
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
}

void RackSlot::updateButtonStates()
{
    // For now, always enable both buttons
    // This can be refined later to check if movement is possible
    if (upButton)
        upButton->setEnabled(true);
    if (downButton)
        downButton->setEnabled(true);
}

void RackSlot::moveUp()
{
    // TODO: Implement gear item movement logic
    std::cout << "[RackSlot " << index << "] Move up requested" << std::endl;
}

void RackSlot::moveDown()
{
    // TODO: Implement gear item movement logic
    std::cout << "[RackSlot " << index << "] Move down requested" << std::endl;
}

// Mouse events for control interaction
void RackSlot::mouseDown(const juce::MouseEvent &e)
{
    // TODO: Implement mouse interaction for gear controls
}

void RackSlot::mouseDrag(const juce::MouseEvent &e)
{
    // TODO: Implement mouse drag for gear controls
}

void RackSlot::mouseUp(const juce::MouseEvent &e)
{
    // TODO: Implement mouse up for gear controls
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
    isDragOver = false;
    repaint();

    // Extract gear ID from drag description
    juce::String description = dragSourceDetails.description.toString();

    // Check if this is a gear item drop
    if (description.startsWith("gear:"))
    {
        juce::String gearId = description.substring(5); // Remove "gear:" prefix

        // Get the gear item from the library
        GearItem *gearItem = gearLibrary.getGearItem(gearId);

        if (gearItem)
        {
            // Check if slot is empty
            if (isEmpty())
            {
                // Add to recently used
                cacheManager.addToRecentlyUsed(gearId);

                // Set the gear item in this slot
                setGearItem(gearItem);

                // Log successful drop
                juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Gear item '" + gearItem->name + "' dropped successfully");
            }
            else
            {
                // Slot is occupied - delegate to parent Rack for insertion logic
                juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Slot is occupied, delegating to parent Rack for insertion");

                // Find the parent Rack component and delegate the drop
                juce::Component *parent = getParentComponent();
                while (parent != nullptr)
                {
                    // Check if parent is a Rack (by class type, not just ID)
                    if (parent->getComponentID() == "Rack" || dynamic_cast<Rack *>(parent) != nullptr)
                    {
                        // Convert the drop position to parent coordinates
                        juce::DragAndDropTarget::SourceDetails parentDetails = dragSourceDetails;
                        parentDetails.localPosition = parent->getLocalPoint(this, dragSourceDetails.localPosition);

                        // Delegate to parent's itemDropped method
                        if (auto *rackTarget = dynamic_cast<juce::DragAndDropTarget *>(parent))
                        {
                            juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Found parent Rack, delegating drop");
                            rackTarget->itemDropped(parentDetails);
                        }
                        return;
                    }
                    juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Checking parent: " + parent->getComponentID());
                    parent = parent->getParentComponent();
                }

                // If no parent Rack found, show error
                juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] No parent Rack found, showing error");
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Slot Occupied",
                    "This slot already contains gear. Please choose an empty slot.",
                    "OK");
            }
        }
        else
        {
            juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Invalid gear ID: " + gearId);
        }
    }
}

// Gear item management
void RackSlot::setGearItem(GearItem *item)
{
    gearItem = item;
    repaint(); // Trigger repaint to show the new gear item
}

void RackSlot::clearGearItem()
{
    gearItem = nullptr;
    repaint(); // Trigger repaint to hide the gear item
}

// Faceplate scaling and control rendering
float RackSlot::calculateOptimalFaceplateScale() const
{
    if (!gearItem || !gearItem->faceplateImage.isValid())
        return 1.0f;

    // Get the slot dimensions (dedicated button row at top, with padding)
    int buttonRowHeight = 25;                                     // Dedicated space at top for buttons
    int slotPadding = 2;                                          // 2px padding on left, right, and bottom
    int slotWidth = getWidth() - (2 * slotPadding);               // Use effective width minus horizontal padding
    int slotHeight = getHeight() - buttonRowHeight - slotPadding; // Leave space for button row and bottom padding

    if (slotWidth <= 0 || slotHeight <= 0)
        return 1.0f;

    // Get the faceplate image dimensions
    int imageWidth = gearItem->faceplateImage.getWidth();
    int imageHeight = gearItem->faceplateImage.getHeight();

    if (imageWidth <= 0 || imageHeight <= 0)
        return 1.0f;

    // Calculate scale factors for both dimensions
    float scaleX = static_cast<float>(slotWidth) / static_cast<float>(imageWidth);
    float scaleY = static_cast<float>(slotHeight) / static_cast<float>(imageHeight);

    // Use the smaller scale to maintain aspect ratio
    float optimalScale = juce::jmin(scaleX, scaleY);

    // Constrain the scale to reasonable bounds (0.1 to 2.0)
    return juce::jlimit(0.1f, 2.0f, optimalScale);
}

void RackSlot::drawControls(juce::Graphics &g)
{
    if (!gearItem || gearItem->controls.isEmpty())
        return;

    // Update the current faceplate scale
    currentFaceplateScale = calculateOptimalFaceplateScale();

    // Offset controls by horizontal padding and position relative to available space
    int buttonRowHeight = 25; // Dedicated space at top for buttons
    int slotPadding = 2;      // 2px padding on left, right, and bottom
    int effectiveHeight = getHeight() - buttonRowHeight - slotPadding;
    int scaledHeight = static_cast<int>(gearItem->faceplateImage.getHeight() * currentFaceplateScale);
    int controlY = buttonRowHeight + (effectiveHeight - scaledHeight) / 2;
    g.addTransform(juce::AffineTransform::translation(slotPadding, controlY));

    // Save the current graphics state
    g.addTransform(juce::AffineTransform::scale(currentFaceplateScale));

    // Draw each control with proper scaling
    for (const auto &control : gearItem->controls)
    {
        switch (control.type)
        {
        case GearControl::ControlType::Button:
            drawButtonControl(g, control);
            break;
        case GearControl::ControlType::Fader:
            drawFaderControl(g, control);
            break;
        case GearControl::ControlType::Switch:
            drawSwitchControl(g, control);
            break;
        case GearControl::ControlType::Knob:
            drawKnobControl(g, control);
            break;
        }
    }

    // Restore the graphics state
    g.addTransform(juce::AffineTransform::scale(1.0f / currentFaceplateScale));
    g.addTransform(juce::AffineTransform::translation(0, -buttonRowHeight));
}

void RackSlot::drawButtonControl(juce::Graphics &g, const GearControl &control)
{
    // Calculate scaled position
    juce::Rectangle<float> scaledPos = control.position * currentFaceplateScale;

    // Draw button background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(scaledPos);

    // Draw button border
    g.setColour(juce::Colours::white);
    g.drawEllipse(scaledPos, 1.0f);

    // Draw button label if available
    if (control.name.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(10.0f / currentFaceplateScale); // Scale font inversely
        g.drawText(control.name, scaledPos, juce::Justification::centred);
    }
}

void RackSlot::drawFaderControl(juce::Graphics &g, const GearControl &control)
{
    // Calculate scaled position and size
    juce::Rectangle<float> scaledPos = control.position * currentFaceplateScale;

    // Draw fader track
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(scaledPos);

    // Draw fader handle at current value position
    float normalizedValue = (control.currentValue - control.minValue) / (control.maxValue - control.minValue);
    float handleY = scaledPos.getY() + normalizedValue * scaledPos.getHeight();

    g.setColour(juce::Colours::white);
    g.fillRect(static_cast<int>(scaledPos.getX() - 2), static_cast<int>(handleY - 4),
               static_cast<int>(scaledPos.getWidth() + 4), 8);

    // Draw fader label
    if (control.name.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(10.0f / currentFaceplateScale);
        g.drawText(control.name, static_cast<int>(scaledPos.getX()), static_cast<int>(scaledPos.getY() - 15),
                   static_cast<int>(scaledPos.getWidth()), 12, juce::Justification::centred);
    }
}

void RackSlot::drawSwitchControl(juce::Graphics &g, const GearControl &control)
{
    // Calculate scaled position
    juce::Rectangle<float> scaledPos = control.position * currentFaceplateScale;

    // Draw switch background
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(scaledPos);

    // Draw switch border
    g.setColour(juce::Colours::white);
    g.drawRect(scaledPos, 1.0f);

    // Draw current option label
    if (control.currentIndex >= 0 && control.currentIndex < control.options.size())
    {
        g.setColour(juce::Colours::white);
        g.setFont(10.0f / currentFaceplateScale);
        g.drawText(control.options[control.currentIndex], scaledPos, juce::Justification::centred);
    }
}

void RackSlot::drawKnobControl(juce::Graphics &g, const GearControl &control)
{
    // Calculate scaled position
    juce::Rectangle<float> scaledPos = control.position * currentFaceplateScale;

    // Draw knob background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(scaledPos);

    // Draw knob border
    g.setColour(juce::Colours::white);
    g.drawEllipse(scaledPos, 1.0f);

    // Draw knob value indicator
    float normalizedValue = (control.currentValue - control.minValue) / (control.maxValue - control.minValue);
    float angle = control.startAngle + normalizedValue * (control.endAngle - control.startAngle);

    // Convert angle to radians and draw indicator line
    float radius = scaledPos.getWidth() * 0.4f;
    float centerX = scaledPos.getCentreX();
    float centerY = scaledPos.getCentreY();
    float endX = centerX + radius * std::cos(angle * M_PI / 180.0f);
    float endY = centerY - radius * std::sin(angle * M_PI / 180.0f);

    g.setColour(juce::Colours::white);
    g.drawLine(centerX, centerY, endX, endY, 2.0f);

    // Draw knob label
    if (control.name.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(10.0f / currentFaceplateScale);
        g.drawText(control.name, static_cast<int>(scaledPos.getX()), static_cast<int>(scaledPos.getY() - 15),
                   static_cast<int>(scaledPos.getWidth()), 12, juce::Justification::centred);
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
        juce::Logger::writeToLog("[RackSlot " + juce::String(index) + "] Triggering faceplate loaded callback");
        faceplateLoadedCallback();
    }
}

void RackSlot::setSlotBackgroundColor(juce::Colour color)
{
    slotBackgroundColor = color;
    repaint();
}
