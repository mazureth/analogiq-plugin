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

    // Set up navigation buttons
    upButton.setButtonText("U");
    downButton.setButtonText("D");

    // Add button listeners
    upButton.addListener(this);
    downButton.addListener(this);

    // Add buttons to this component
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);

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
    upButton.removeListener(this);
    downButton.removeListener(this);
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
        g.setColour(juce::Colours::lightblue);
    }
    else if (gearItem != nullptr)
    {
        g.setColour(juce::Colours::lightgrey);
    }
    else
    {
        g.setColour(juce::Colours::darkgrey);
    }
    g.fillAll();

    // Draw slot border
    g.setColour(juce::Colours::white);
    g.drawRect(area, 1);

    // Draw slot index
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("Slot " + juce::String(index + 1), area.removeFromLeft(60), juce::Justification::centred);

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

            // Center the faceplate in the slot
            int x = (getWidth() - scaledWidth) / 2;
            int y = (getHeight() - scaledHeight) / 2;

            // Draw the faceplate image
            g.drawImage(gearItem->faceplateImage, x, y, scaledWidth, scaledHeight, 0, 0,
                        gearItem->faceplateImage.getWidth(), gearItem->faceplateImage.getHeight());
        }

        // Draw gear name
        g.setColour(juce::Colours::black);
        g.setFont(12.0f);
        g.drawText(gearItem->name, area.removeFromLeft(120), juce::Justification::centredLeft);

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

    // Position navigation buttons on the right side
    int buttonWidth = 30;
    int buttonHeight = 20;
    int buttonMargin = 5;

    // Up button at top right
    upButton.setBounds(area.getRight() - buttonWidth - buttonMargin,
                       buttonMargin,
                       buttonWidth,
                       buttonHeight);

    // Down button below up button
    downButton.setBounds(area.getRight() - buttonWidth - buttonMargin,
                         buttonMargin + buttonHeight + 2,
                         buttonWidth,
                         buttonHeight);
}

// Button handling
void RackSlot::buttonClicked(juce::Button *button)
{
    if (button == &upButton)
    {
        moveUp();
    }
    else if (button == &downButton)
    {
        moveDown();
    }
}

void RackSlot::updateButtonStates()
{
    // For now, always enable both buttons
    // This can be refined later to check if movement is possible
    upButton.setEnabled(true);
    downButton.setEnabled(true);
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

    // Get the slot dimensions (excluding margins for buttons)
    int slotWidth = getWidth() - 80;   // Leave space for buttons and margins
    int slotHeight = getHeight() - 20; // Leave space for top/bottom margins

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
