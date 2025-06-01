/**
 * @file RackSlot.cpp
 * @brief Implementation of the RackSlot class, which manages individual slots in the rack system.
 *
 * This file implements the functionality for:
 * - Rendering gear items and their controls
 * - Handling drag and drop operations
 * - Managing control interactions (buttons, switches, faders, knobs)
 * - Supporting instance management
 * - Up/down movement of items within the rack
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#include "RackSlot.h"
#include "GearLibrary.h"
#include "Rack.h"
#include <fstream>

// Add at the top of the file, after includes
static std::ofstream logFile("/tmp/rack_slot.log");

/**
 * @brief Helper function for logging messages to a file.
 * @param message The message to log.
 */
static void logToFile(const juce::String &message)
{
    if (logFile.is_open())
    {
        logFile << message << std::endl;
        logFile.flush();
    }
}

/**
 * @brief Constructs a new RackSlot with the specified index.
 *
 * Initializes the slot with:
 * - Component ID and mouse intercept settings
 * - Up/down movement buttons with custom arrow graphics
 * - Remove button with X graphic
 * - Initial button states
 *
 * @param slotIndex The position of this slot in the rack.
 */
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

/**
 * @brief Destructor for RackSlot.
 *
 * Ensures proper cleanup by:
 * - Removing button listeners
 * - Allowing smart pointers to clean up button resources
 */
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

/**
 * @brief Paints the rack slot and its contents.
 *
 * Renders:
 * - Background and border
 * - Slot number
 * - Gear item details (if present)
 * - Faceplate image (if available)
 * - Controls on top of faceplate
 *
 * @param g The graphics context to paint with.
 */
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

            // Calculate faceplate area
            juce::Rectangle<int> faceplateArea = getLocalBounds().reduced(10);

            // Draw name above the faceplate (for better identification)
            g.setFont(12.0f);
            g.setColour(juce::Colours::white);
            juce::Rectangle<int> nameArea = faceplateArea.removeFromTop(20);
            g.drawText(gearItem->name, nameArea, juce::Justification::centred, true);

            // Calculate scaling factor based on faceplate dimensions
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
            // Reset scale factor when no faceplate
            currentFaceplateScale = 1.0f;

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
        // Reset scale factor for empty slots
        currentFaceplateScale = 1.0f;

        // Draw "empty slot" text for available slots
        g.setColour(juce::Colours::lightgrey);
        g.setFont(14.0f);
        g.drawText("Empty Slot", getLocalBounds(), juce::Justification::centred, true);
    }
}

/**
 * @brief Draws all controls for the current gear item.
 *
 * Iterates through the gear item's controls and draws each one
 * based on its type (switch, button, fader, or knob).
 *
 * @param g The graphics context to draw with.
 * @param faceplateArea The area in which to draw the controls.
 */
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
        case GearControl::Type::Switch:
            drawSwitch(g, control, x, y);
            break;
        case GearControl::Type::Button:
            drawButton(g, control, x, y);
            break;
        case GearControl::Type::Fader:
            drawFader(g, control, x, y);
            break;
        case GearControl::Type::Knob:
            drawKnob(g, control, x, y);
            break;
        }
    }
}

/**
 * @brief Draws a switch control.
 *
 * Renders a switch using its sprite sheet if available,
 * with proper scaling based on the faceplate scale.
 * Falls back to basic drawing if no sprite sheet is available.
 *
 * @param g The graphics context to draw with.
 * @param control The control to draw.
 * @param x The x-coordinate to draw at.
 * @param y The y-coordinate to draw at.
 */
void RackSlot::drawSwitch(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const bool isVertical = control.orientation == "vertical";
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
            float scaledFrameX = frame.x * currentFaceplateScale;
            float scaledFrameY = frame.y * currentFaceplateScale;
            float scaledFrameWidth = frame.width * currentFaceplateScale;
            float scaledFrameHeight = frame.height * currentFaceplateScale;

            logToFile("--- Sprite sheet dimensions: " +
                      juce::String(originalSpriteWidth) + "x" + juce::String(originalSpriteHeight) +
                      " scaled to: " + juce::String(scaledSpriteWidth) + "x" + juce::String(scaledSpriteHeight));

            logToFile("---- Frame dimensions: " +
                      juce::String(frame.x) + "," + juce::String(frame.y) + " " +
                      juce::String(frame.width) + "x" + juce::String(frame.height) +
                      " scaled to: " + juce::String(scaledFrameX) + "," + juce::String(scaledFrameY) + " " +
                      juce::String(scaledFrameWidth) + "x" + juce::String(scaledFrameHeight));

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

            logToFile("---- Drawing switch frame " + juce::String(currentIndex) +
                      " for control: " + control.name +
                      "\n----- at position: " + juce::String(x) + "," + juce::String(y) +
                      "\n----- with dimensions: " + juce::String(scaledFrameWidth) + "x" + juce::String(scaledFrameHeight) +
                      "\n----- faceplate scale: " + juce::String(currentFaceplateScale));
        }
    }
    else
    {
        // Fallback to basic drawing if no sprite sheet is available
        const int switchWidth = 30;
        const int switchHeight = 60;

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
            float indicatorY = y + (currentIndex * (switchHeight / control.options.size()));
            g.fillRoundedRectangle(x + 4, indicatorY + 4, switchWidth - 8, (switchHeight / control.options.size()) - 8, 2.0f);
        }
        else
        {
            float indicatorX = x + (currentIndex * (switchWidth / control.options.size()));
            g.fillRoundedRectangle(indicatorX + 4, y + 4, (switchWidth / control.options.size()) - 8, switchHeight - 8, 2.0f);
        }
    }
}

/**
 * @brief Draws a button control.
 *
 * Renders a button using its sprite sheet if available,
 * with proper scaling based on the faceplate scale.
 * Falls back to basic drawing if no sprite sheet is available.
 *
 * @param g The graphics context to draw with.
 * @param control The control to draw.
 * @param x The x-coordinate to draw at.
 * @param y The y-coordinate to draw at.
 */
void RackSlot::drawButton(juce::Graphics &g, const GearControl &control, int x, int y)
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
        baseWidth = (float)control.buttonFrames[0].width;
        baseHeight = (float)control.buttonFrames[0].height;
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

    // Debug logging for button sprite sheet
    logToFile("Drawing button: " + control.name);
    logToFile("  Has sprite sheet: " + juce::String(control.buttonSpriteSheet.isValid() ? "YES" : "NO"));
    logToFile("  Number of frames: " + juce::String(control.buttonFrames.size()));
    logToFile("  Is momentary: " + juce::String(control.momentary ? "YES" : "NO"));

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
        float scaledFrameX = frame.x * currentFaceplateScale;
        float scaledFrameY = frame.y * currentFaceplateScale;
        float scaledFrameWidth = frame.width * currentFaceplateScale;
        float scaledFrameHeight = frame.height * currentFaceplateScale;

        logToFile("  Frame dimensions: " +
                  juce::String(frame.x) + "," + juce::String(frame.y) + " " +
                  juce::String(frame.width) + "x" + juce::String(frame.height) +
                  " scaled to: " + juce::String(scaledFrameX) + "," + juce::String(scaledFrameY) + " " +
                  juce::String(scaledFrameWidth) + "x" + juce::String(scaledFrameHeight));

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
        g.setColour(control.value > 0.5f ? juce::Colours::red : juce::Colours::darkgrey);
        g.fillRoundedRectangle(x, y, buttonWidth, buttonHeight, 4.0f);

        g.setColour(juce::Colours::grey);
        g.drawRoundedRectangle(x, y, buttonWidth, buttonHeight, 4.0f, 2.0f);
    }
}

/**
 * @brief Draws a fader control.
 *
 * Renders a fader using its sprite sheet if available,
 * with proper scaling based on the faceplate scale.
 * Falls back to basic drawing if no sprite sheet is available.
 *
 * @param g The graphics context to draw with.
 * @param control The control to draw.
 * @param x The x-coordinate to draw at.
 * @param y The y-coordinate to draw at.
 */
void RackSlot::drawFader(juce::Graphics &g, const GearControl &control, int x, int y)
{
    const bool isVertical = control.orientation == "vertical";
    const float faderLength = control.length * currentFaceplateScale;

    // Calculate base dimensions from the image if available, otherwise use defaults
    float baseWidth, baseHeight;
    if (control.faderImage.isValid())
    {
        baseWidth = (float)control.faderImage.getWidth();
        baseHeight = (float)control.faderImage.getHeight();
    }
    else
    {
        baseWidth = 20.0f;
        baseHeight = 20.0f;
    }

    const float faderWidth = baseWidth * currentFaceplateScale;
    const float handleSize = std::max(baseWidth, baseHeight) * currentFaceplateScale;

    // Debug logging for fader image
    logToFile("Drawing fader: " + control.name);
    logToFile("  Has fader image: " + juce::String(control.faderImage.isValid() ? "YES" : "NO"));
    if (control.faderImage.isValid())
    {
        logToFile("  Image dimensions: " +
                  juce::String(control.faderImage.getWidth()) + "x" +
                  juce::String(control.faderImage.getHeight()));
        logToFile("  Image format: " + juce::String(control.faderImage.getFormat()));
    }

    // Calculate the fader bounds based on orientation
    juce::Rectangle<float> faderBounds;
    if (isVertical)
    {
        faderBounds = juce::Rectangle<float>(x, y, faderWidth, faderLength);
    }
    else
    {
        faderBounds = juce::Rectangle<float>(x, y, faderLength, faderWidth);
    }

    // Calculate the handle position based on the control value
    float handleX, handleY;
    if (isVertical)
    {
        handleX = x;
        handleY = y + (1.0f - control.value) * faderLength;
    }
    else
    {
        handleX = x + control.value * faderLength;
        handleY = y;
    }

    // Draw the fader image at the handle position
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

        logToFile("  Drawing image at: " +
                  juce::String(handleX - scaledWidth / 2) + "," +
                  juce::String(handleY - scaledHeight / 2) +
                  " with size: " +
                  juce::String(scaledWidth) + "x" +
                  juce::String(scaledHeight));

        // Draw the fader image centered at the handle position
        g.drawImageWithin(control.faderImage,
                          handleX - scaledWidth / 2,
                          handleY - scaledHeight / 2,
                          scaledWidth,
                          scaledHeight,
                          juce::RectanglePlacement::centred);
    }
    else
    {
        logToFile("  Using fallback white handle");
        // Fallback to basic handle drawing if no image is provided
        g.setColour(juce::Colours::white);
        if (isVertical)
        {
            g.fillRoundedRectangle(x - handleSize / 2, handleY - handleSize / 2,
                                   faderWidth + handleSize, handleSize, 4.0f);
        }
        else
        {
            g.fillRoundedRectangle(handleX - handleSize / 2, y - handleSize / 2,
                                   handleSize, faderWidth + handleSize, 4.0f);
        }
    }
}

/**
 * @brief Draws a knob control.
 *
 * Renders a knob using its sprite sheet if available,
 * with proper scaling based on the faceplate scale.
 * Falls back to basic drawing if no sprite sheet is available.
 *
 * @param g The graphics context to draw with.
 * @param control The control to draw.
 * @param x The x-coordinate to draw at.
 * @param y The y-coordinate to draw at.
 */
void RackSlot::drawKnob(juce::Graphics &g, const GearControl &control, int x, int y)
{
    DBG("drawKnob - Control: " + control.name +
        " Value: " + juce::String(control.value) +
        " StartAngle: " + juce::String(control.startAngle) +
        " EndAngle: " + juce::String(control.endAngle));

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
        DBG("Drawing knob image for control: " + control.name +
            " at position: " + juce::String(x) + "," + juce::String(y) +
            " with dimensions: " + juce::String(control.loadedImage.getWidth()) + "x" +
            juce::String(control.loadedImage.getHeight()) +
            " scaled to: " + juce::String(knobSize) + "x" + juce::String(knobSize) +
            " angle: " + juce::String(control.value));

        // Save the current graphics state
        g.saveState();

        // Use the control value directly as degrees, but subtract 180 to align with JUCE's coordinate system
        // where 0 is at 12 o'clock and we want 0 to be at 6 o'clock
        float angle = control.value - 180.0f;
        DBG("Drawing knob at angle: " + juce::String(angle));

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
        DBG("No valid knob image for control: " + control.name + ", using fallback drawing");
        // Fallback to basic drawing if no image is provided
        g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colours::black);
        g.drawEllipse(knobBounds, 1.0f);

        // Draw position indicator
        g.setColour(juce::Colours::white);
        float angle = control.value - 180.0f; // Subtract 90 to align with JUCE's coordinate system
        DBG("Drawing fallback knob at angle: " + juce::String(angle));
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

/**
 * @brief Handles button click events.
 *
 * Processes clicks on the up/down/remove buttons:
 * - Up button: moves item up in rack
 * - Down button: moves item down in rack
 * - Remove button: removes item from rack
 *
 * @param button The button that was clicked.
 */
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

/**
 * @brief Updates the state of the up/down/remove buttons.
 *
 * Enables/disables buttons based on:
 * - Slot position in rack
 * - Whether slot is empty
 * - Whether item is an instance
 */
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

/**
 * @brief Moves the current gear item up one position in the rack.
 *
 * Swaps the current item with the item in the slot above,
 * updating both slots' visual states.
 */
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

/**
 * @brief Moves the current gear item down one position in the rack.
 *
 * Swaps the current item with the item in the slot below,
 * updating both slots' visual states.
 */
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

/**
 * @brief Handles mouse down events for control interaction.
 *
 * Initiates control interaction by:
 * - Finding the control under the mouse position
 * - Storing initial state for drag operations
 * - Setting up active control tracking
 *
 * @param e The mouse event details.
 */
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

        // Store drag start state for faders, knobs, and switches
        if (activeControl->type == GearControl::Type::Fader ||
            activeControl->type == GearControl::Type::Knob ||
            activeControl->type == GearControl::Type::Switch)
        {
            dragStartPos = e.position;
            dragStartValue = activeControl->value;
            isDragging = true;
        }

        // Handle interaction based on control type (for click/tap)
        switch (activeControl->type)
        {
        case GearControl::Type::Fader:
            // Don't update value on click, wait for drag
            break;
        case GearControl::Type::Switch:
            // Don't update value on click, wait for drag
            break;
        case GearControl::Type::Button:
            handleButtonInteraction(*activeControl);
            repaint();
            break;
        case GearControl::Type::Knob:
            // Don't update value on click, wait for drag
            break;
        }
    }
}

/**
 * @brief Handles mouse drag events for control interaction.
 *
 * Updates control state during drag operations:
 * - Updates fader position based on drag distance
 * - Updates knob rotation based on drag angle
 * - Triggers repaints for visual feedback
 *
 * @param e The mouse event details.
 */
void RackSlot::mouseDrag(const juce::MouseEvent &e)
{
    if (!isDragging || activeControl == nullptr)
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

    switch (activeControl->type)
    {
    case GearControl::Type::Switch:
    {
        const bool isVertical = activeControl->orientation == "vertical";
        const int numOptions = activeControl->options.size();
        const float faderLength = activeControl->length * currentFaceplateScale;

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

        // Calculate the total range of movement using fader length
        float totalRange = faderLength;
        float optionSize = totalRange / numOptions;

        // Calculate the new index based on drag distance
        float newIndex = dragStartValue + (dragDistance / optionSize);

        // Clamp the index to valid range and round to nearest option
        newIndex = juce::jlimit(0.0f, (float)(numOptions - 1), newIndex);
        int newIndexInt = juce::roundToInt(newIndex);

        // Update the control
        if (newIndexInt != activeControl->currentIndex)
        {
            activeControl->currentIndex = newIndexInt;
            activeControl->value = (float)newIndexInt;
            repaint();
        }
        break;
    }
    case GearControl::Type::Fader:
    {
        const bool isVertical = activeControl->orientation == "vertical";
        const float faderLength = activeControl->length * currentFaceplateScale;
        float newValue;

        if (isVertical)
        {
            // For vertical faders, use Y position relative to fader track
            float trackY = y;
            float trackHeight = faderLength;
            float normalizedY = 1.0f - (float)(e.position.y - trackY) / trackHeight;
            newValue = juce::jlimit(0.0f, 1.0f, normalizedY);
        }
        else
        {
            // For horizontal faders, use X position relative to fader track
            float trackX = x;
            float trackWidth = faderLength;
            float normalizedX = (float)(e.position.x - trackX) / trackWidth;
            newValue = juce::jlimit(0.0f, 1.0f, normalizedX);
        }

        activeControl->value = newValue;
        logToFile("Fader - Value: " + juce::String(activeControl->value) +
                  " Position: " + e.position.toString());
        repaint();
        break;
    }
    case GearControl::Type::Knob:
    {
        // Calculate vertical movement since drag start
        float deltaY = dragStartPos.y - e.position.y;

        // Scale the movement to control sensitivity
        const float sensitivity = 0.5f; // Adjust this value to change knob sensitivity
        float deltaAngle = deltaY * sensitivity;

        // Update the control value (angle in degrees)
        float newValue = dragStartValue + deltaAngle;

        // Clamp the value between startAngle and endAngle
        newValue = juce::jlimit(activeControl->startAngle, activeControl->endAngle, newValue);

        // If this is a stepped knob, snap to the nearest step
        if (!activeControl->steps.isEmpty())
        {
            float closestStep = activeControl->steps[0];
            float minDistance = std::abs(newValue - closestStep);

            // Find the closest step angle
            for (float step : activeControl->steps)
            {
                float distance = std::abs(newValue - step);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestStep = step;
                }
            }

            newValue = closestStep;
            DBG("Stepped knob - Snapped to step: " + juce::String(newValue) + " degrees");
        }

        activeControl->value = newValue;

        logToFile("Knob - Angle: " + juce::String(activeControl->value) +
                  " Delta Y: " + juce::String(deltaY) +
                  " Delta Angle: " + juce::String(deltaAngle));
        repaint();
        break;
    }
    case GearControl::Type::Button:
        handleButtonInteraction(*activeControl);
        repaint();
        break;
    default:
        break; // Other controls don't need drag handling
    }
}

/**
 * @brief Handles mouse up events for control interaction.
 *
 * Finalizes control interaction by:
 * - Resetting drag state
 * - Clearing active control
 * - Triggering final repaint
 *
 * @param e The mouse event details.
 */
void RackSlot::mouseUp(const juce::MouseEvent &)
{
    if (isDragging)
    {
        logToFile("Mouse up - Final value: " + (activeControl ? juce::String(activeControl->value) : "no control"));
        isDragging = false;
    }
    activeControl = nullptr;
}

/**
 * @brief Handles mouse double-click events for control interaction.
 *
 * Used for special control interactions:
 * - Resetting controls to default values
 * - Toggling between states
 *
 * @param e The mouse event details.
 */
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
        case GearControl::Type::Switch:
            control->currentIndex = 0;
            break;
        case GearControl::Type::Button:
            control->value = 0.0f;
            break;
        case GearControl::Type::Fader:
            control->value = control->initialValue;
            break;
        case GearControl::Type::Knob:
            DBG("Resetting knob to initial value: " + juce::String(control->initialValue));
            control->value = control->initialValue;
            break;
        }
        repaint();
    }
}

/**
 * @brief Finds a control at the specified position.
 *
 * Searches through the gear item's controls to find one
 * that contains the given position, taking into account
 * the faceplate scaling.
 *
 * @param position The position to check.
 * @param faceplateArea The area containing the controls.
 * @return Pointer to the control at the position, or nullptr if none found.
 */
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
        case GearControl::Type::Switch:
            width = 30;
            height = 60;
            break;
        case GearControl::Type::Button:
            width = height = 30;
            break;
        case GearControl::Type::Fader:
        {
            const bool isVertical = control.orientation == "vertical";
            const float faderLength = control.length * currentFaceplateScale;
            const float faderWidth = 20.0f * currentFaceplateScale;
            const float handleSize = 10.0f * currentFaceplateScale;

            // Calculate handle position based on current value
            float handleX, handleY;
            if (isVertical)
            {
                handleX = x;
                handleY = y + (1.0f - control.value) * faderLength;
            }
            else
            {
                handleX = x + control.value * faderLength;
                handleY = y;
            }

            // Create handle bounds
            juce::Rectangle<float> handleBounds(
                handleX - handleSize / 2,
                handleY - handleSize / 2,
                isVertical ? faderWidth + handleSize : handleSize,
                isVertical ? handleSize : faderWidth + handleSize);

            // Check if click is within handle bounds
            if (handleBounds.contains(position))
                return &control;
            continue; // Skip to next control if not clicking handle
        }
        case GearControl::Type::Knob:
            width = height = 40;
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

/**
 * @brief Handles interaction with a switch control.
 *
 * Updates switch state based on interaction:
 * - Cycles through available options
 * - Updates visual state
 * - Triggers repaint
 *
 * @param control The control to handle.
 */
void RackSlot::handleSwitchInteraction(GearControl &control)
{
    // Toggle between options
    control.currentIndex = (control.currentIndex + 1) % control.options.size();
    control.value = (float)control.currentIndex; // Simply use the index as the value
}

/**
 * @brief Handles interaction with a button control.
 *
 * Updates button state based on interaction:
 * - For momentary buttons: toggles between off (0) and on (1)
 * - For latching buttons: cycles through all options
 * - Updates visual state
 * - Triggers repaint
 *
 * @param control The control to handle.
 */
void RackSlot::handleButtonInteraction(GearControl &control)
{
    if (control.momentary)
    {
        // For momentary buttons, use value 1.0 for "on" state (index 1) and 0.0 for "off" state (index 0)
        control.value = control.value > 0.5f ? 0.0f : 1.0f;
        control.currentIndex = (int)control.value;
    }
    else
    {
        // For latching buttons, cycle through options like switches
        control.currentIndex = (control.currentIndex + 1) % control.options.size();
        control.value = (float)control.currentIndex;
    }
}

/**
 * @brief Handles interaction with a fader control.
 *
 * Updates fader state based on interaction:
 * - Calculates new value based on mouse position
 * - Clamps value to valid range
 * - Updates visual state
 * - Triggers repaint
 *
 * @param control The control to handle.
 * @param mousePos The current mouse position.
 * @param controlBounds The bounds of the control.
 */
void RackSlot::handleFaderInteraction(GearControl &control, const juce::Point<float> &mousePos, const juce::Rectangle<int> &controlBounds)
{
    // Map vertical position to value
    float normalizedY = 1.0f - (float)(mousePos.y - controlBounds.getY()) / controlBounds.getHeight();
    control.value = juce::jlimit(0.0f, 1.0f, normalizedY);
}

/**
 * @brief Checks if this slot is interested in a drag source.
 *
 * Determines if the slot can accept the dragged item based on:
 * - Whether the slot is empty
 * - Whether the dragged item is valid
 *
 * @param dragSourceDetails Details about the drag source.
 * @return true if this slot can accept the drag source.
 */
bool RackSlot::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // We're not using drag-and-drop for reordering anymore, but keeping the code for GearLibrary drags

    // Accept drag sources from the GearLibrary list box (legacy)
    if (dragSourceDetails.description.isInt())
    {
        auto *sourceComp = dragSourceDetails.sourceComponent.get();
        if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" ||
                           sourceComp->getComponentID() == "GearListBox"))
        {
            return true;
        }
    }

    // Accept drag sources from TreeView (new hierarchical view)
    if (dragSourceDetails.description.isString())
    {
        juce::String desc = dragSourceDetails.description.toString();
        if (desc.startsWith("GEAR:"))
        {
            auto *sourceComp = dragSourceDetails.sourceComponent.get();
            if (sourceComp && dynamic_cast<juce::TreeView *>(sourceComp) != nullptr)
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Called when a drag operation enters this slot.
 *
 * Updates visual state to indicate potential drop target:
 * - Highlights slot border
 * - Updates cursor
 *
 * @param dragSourceDetails Details about the drag source.
 */
void RackSlot::itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    setHighlighted(true);
}

/**
 * @brief Called when a drag operation moves within this slot.
 *
 * Updates visual feedback during drag:
 * - Maintains highlight state
 * - Updates cursor
 *
 * @param dragSourceDetails Details about the drag source.
 */
void RackSlot::itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Nothing needed here
}

/**
 * @brief Called when a drag operation exits this slot.
 *
 * Resets visual state:
 * - Removes highlight
 * - Restores default cursor
 *
 * @param dragSourceDetails Details about the drag source.
 */
void RackSlot::itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    setHighlighted(false);
}

/**
 * @brief Called when a drag operation is dropped on this slot.
 *
 * Handles the dropped item by:
 * - Creating a new gear item from the source
 * - Setting up the item in this slot
 * - Updating visual state
 *
 * @param dragSourceDetails Details about the drag source.
 */
void RackSlot::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
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
                positionInParent = parentRack->getLocalPoint(this, dragSourceDetails.localPosition);

                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    dragSourceDetails.description,
                    dragSourceDetails.sourceComponent.get(),
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
                auto posInContainer = container->getLocalPoint(this, dragSourceDetails.localPosition);
                positionInParent = container->rack->getLocalPoint(container, posInContainer);

                // Create simplified source details with only the required data
                juce::DragAndDropTarget::SourceDetails parentDetails(
                    dragSourceDetails.description,
                    dragSourceDetails.sourceComponent.get(),
                    positionInParent);

                // Call the Rack's itemDropped
                container->rack->itemDropped(parentDetails);
            }
        }
    }
}

/**
 * @brief Sets a new gear item in this slot.
 *
 * Updates the slot's state with the new item:
 * - Stores the item pointer
 * - Updates button states
 * - Triggers repaint
 *
 * @param newGearItem The gear item to set.
 */
void RackSlot::setGearItem(GearItem *newGearItem)
{
    DBG("RackSlot::setGearItem for slot " + juce::String(index));

    // If we already have a gear item and it's an instance, preserve its state
    if (gearItem != nullptr && gearItem->isInstance)
    {
        // Store the current state
        juce::String instanceId = gearItem->instanceId;
        juce::String sourceUnitId = gearItem->sourceUnitId;
        juce::Array<GearControl> preservedControls;

        // Create new GearControl objects with preserved values
        for (const auto &control : gearItem->controls)
        {
            GearControl newControl = control; // Use copy constructor
            preservedControls.add(newControl);
        }

        // Set the new gear item
        gearItem = newGearItem;

        // If the new item is the same type as the source, restore the instance state
        if (newGearItem != nullptr && newGearItem->unitId == sourceUnitId)
        {
            // Create a new instance of the source item
            newGearItem->createInstance(sourceUnitId);

            // Restore the preserved controls
            newGearItem->controls = preservedControls;

            // Update initial values to match current values
            for (auto &control : newGearItem->controls)
            {
                control.initialValue = control.value;
            }
        }
    }
    else if (newGearItem != nullptr && !newGearItem->isInstance)
    {
        // For new non-instance items, ensure they're not marked as instances
        newGearItem->isInstance = false;
        newGearItem->instanceId = juce::String();
        newGearItem->sourceUnitId = juce::String();
        gearItem = newGearItem;
    }
    else
    {
        gearItem = newGearItem;
    }

    updateButtonStates(); // Update button states when gear changes
    repaint();            // Trigger repaint to show the gear item
}

/**
 * @brief Removes the current gear item from this slot.
 *
 * Cleans up the slot's state:
 * - Clears the item pointer
 * - Updates button states
 * - Triggers repaint
 */
void RackSlot::clearGearItem()
{
    DBG("RackSlot::clearGearItem for slot " + juce::String(index));
    gearItem = nullptr;
    updateButtonStates(); // Update button states when gear is removed
    repaint();            // Trigger repaint to update
}

/**
 * @brief Sets the highlighted state of this slot.
 *
 * Updates visual state to indicate:
 * - Drag target status
 * - Selection status
 *
 * @param shouldHighlight Whether the slot should be highlighted.
 */
void RackSlot::setHighlighted(bool shouldHighlight)
{
    highlighted = shouldHighlight;
    repaint();
}

/**
 * @brief Creates an instance of the current gear item.
 *
 * Creates a new instance by:
 * - Generating a unique instance ID
 * - Copying the source item's properties
 * - Setting up instance-specific state
 */
void RackSlot::createInstance()
{
    if (gearItem == nullptr || gearItem->isInstance)
        return;

    // Create a new instance of the current gear item
    gearItem->createInstance(gearItem->unitId);

    // Log the instance creation
    DBG("Created instance of " + gearItem->name + " with ID: " + gearItem->instanceId);

    // Repaint to show the instance state
    repaint();
}

/**
 * @brief Resets the current gear item to its source state.
 *
 * Restores the item to its original state by:
 * - Clearing instance-specific properties
 * - Resetting control values
 * - Updating visual state
 */
void RackSlot::resetToSource()
{
    if (gearItem == nullptr || !gearItem->isInstance)
        return;

    // Reset the instance to match its source
    gearItem->resetToSource();

    // Log the reset
    DBG("Reset instance " + gearItem->instanceId + " to source " + gearItem->sourceUnitId);

    // Repaint to show the updated state
    repaint();
}

/**
 * @brief Finds the parent Rack component.
 *
 * Traverses up the component hierarchy to find the parent Rack.
 * Used for coordinating operations between slots.
 *
 * @return Pointer to the parent Rack component, or nullptr if not found.
 */
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
