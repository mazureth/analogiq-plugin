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

    juce::Logger::writeToLog("[HORIZONTAL_DEBUG] RackSlot::paint - getLocalBounds()=" + area.toString() +
                             ", slotIndex=" + juce::String(getSlotIndex()));

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
        // Draw faceplate image if available (exactly like old system)
        if (gearItem->faceplateImage.isValid())
        {
            // Calculate faceplate area (exactly like old system)
            juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);

            juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Faceplate area after reduced(10): " + faceplateArea.toString());

            // Remove 20 pixels from top to match old system's faceplate area (without drawing text)
            faceplateArea.removeFromTop(20);

            juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Faceplate area after removeFromTop(20): " + faceplateArea.toString());

            // Calculate scaling factor based on faceplate dimensions (exactly like old system)
            float originalWidth = (float)gearItem->faceplateImage.getWidth();
            float originalHeight = (float)gearItem->faceplateImage.getHeight();
            float targetWidth = (float)faceplateArea.getWidth();
            float targetHeight = (float)faceplateArea.getHeight();

            // Calculate scale factor that maintains aspect ratio
            float scaleX = targetWidth / originalWidth;
            float scaleY = targetHeight / originalHeight;
            float scaleFactor = std::min(scaleX, scaleY); // Use the smaller scale to fit within bounds

            juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Faceplate scaling - originalWidth=" + juce::String(originalWidth, 2) +
                                     ", originalHeight=" + juce::String(originalHeight, 2) +
                                     ", targetWidth=" + juce::String(targetWidth, 2) +
                                     ", targetHeight=" + juce::String(targetHeight, 2) +
                                     ", scaleX=" + juce::String(scaleX, 4) +
                                     ", scaleY=" + juce::String(scaleY, 4) +
                                     ", scaleFactor=" + juce::String(scaleFactor, 4));

            // Store the scale factor for use in drawing controls
            currentFaceplateScale = scaleFactor;

            // Draw the faceplate image (exactly like old system)
            g.drawImageWithin(gearItem->faceplateImage,
                              faceplateArea.getX(), faceplateArea.getY(),
                              faceplateArea.getWidth(), faceplateArea.getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

            // Draw controls on top of the faceplate
            drawControls(g, faceplateArea);
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
            g.drawText(gearItem->name, nameArea, juce::Justification::centred, true);

            // Draw manufacturer below name
            g.setFont(12.0f);
            g.setColour(juce::Colours::lightgrey);
            juce::Rectangle<int> mfgArea = nameArea.translated(0, 20);
            g.drawText(gearItem->manufacturer, mfgArea, juce::Justification::centred, true);
        }
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
            }
            else
            {
                // Slot is occupied - delegate to parent Rack for insertion logic

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
                            rackTarget->itemDropped(parentDetails);
                        }
                        return;
                    }
                    parent = parent->getParentComponent();
                }

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

// Control rendering

void RackSlot::drawControls(juce::Graphics &g, const juce::Rectangle<int> &faceplateArea)
{
    if (gearItem == nullptr)
        return;

    juce::Logger::writeToLog("[HORIZONTAL_DEBUG] drawControls - faceplateArea=" + faceplateArea.toString() +
                             ", controls count=" + juce::String(gearItem->controls.size()));

    for (const auto &control : gearItem->controls)
    {
        // Calculate control position relative to faceplate (exactly like old system)
        int x = faceplateArea.getX() + (int)(control.position.getX() * faceplateArea.getWidth());
        int y = faceplateArea.getY() + (int)(control.position.getY() * faceplateArea.getHeight());

        juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Control '" + control.name + "' - position.getX()=" + juce::String(control.position.getX(), 4) +
                                 ", faceplateArea.getX()=" + juce::String(faceplateArea.getX()) +
                                 ", faceplateArea.getWidth()=" + juce::String(faceplateArea.getWidth()) +
                                 ", calculated x=" + juce::String(x) +
                                 ", position.getY()=" + juce::String(control.position.getY(), 4) +
                                 ", faceplateArea.getY()=" + juce::String(faceplateArea.getY()) +
                                 ", faceplateArea.getHeight()=" + juce::String(faceplateArea.getHeight()) +
                                 ", calculated y=" + juce::String(y));

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

    juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Fader '" + control.name + "' - isVertical=" + juce::String(isVertical ? "true" : "false") +
                             ", control.length=" + juce::String(control.length, 4) +
                             ", currentFaceplateScale=" + juce::String(currentFaceplateScale, 4) +
                             ", faderLength=" + juce::String(faderLength, 2) +
                             ", trackWidth=" + juce::String(trackWidth, 2) +
                             ", control.currentValue=" + juce::String(control.currentValue, 4) +
                             ", handleX=" + juce::String(handleX, 2) +
                             ", handleY=" + juce::String(handleY, 2) +
                             ", trackBounds=" + trackBounds.toString());

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

        juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Knob '" + control.name + "' - originalWidth=" + juce::String(originalWidth, 2) +
                                 ", originalHeight=" + juce::String(originalHeight, 2) +
                                 ", currentFaceplateScale=" + juce::String(currentFaceplateScale, 4) +
                                 ", knobSize=" + juce::String(knobSize, 2));
    }
    else
    {
        // Fallback to standard size if no image
        const float baseKnobSize = 40.0f;
        knobSize = baseKnobSize * currentFaceplateScale;

        juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Knob '" + control.name + "' (fallback) - baseKnobSize=" + juce::String(baseKnobSize, 2) +
                                 ", currentFaceplateScale=" + juce::String(currentFaceplateScale, 4) +
                                 ", knobSize=" + juce::String(knobSize, 2));
    }

    // Create knob bounds using the transformed coordinates and scaled size
    juce::Rectangle<float> knobBounds(x, y, knobSize, knobSize);

    juce::Logger::writeToLog("[HORIZONTAL_DEBUG] Knob '" + control.name + "' final bounds - x=" + juce::String(x) +
                             ", y=" + juce::String(y) +
                             ", knobSize=" + juce::String(knobSize, 2) +
                             ", knobBounds=" + knobBounds.toString());

    // Draw the knob image if available
    if (control.loadedImage.isValid())
    {
        // Save the current graphics state
        g.saveState();

        // Use the control value directly as degrees, but subtract 180 to align with JUCE's coordinate system
        // where 0 is at 12 o'clock and we want 0 to be at 6 o'clock
        float angle = control.currentValue - 180.0f;

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
        float angle = control.currentValue - 180.0f; // Subtract 90 to align with JUCE's coordinate system
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
