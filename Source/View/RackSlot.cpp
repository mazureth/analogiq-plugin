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
    // Temporarily disable component ID to isolate JUCE assertion issues
    // setComponentID("RackSlot_" + juce::String(index));

    // Set up navigation buttons
    upButton.setButtonText("↑");
    downButton.setButtonText("↓");

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
    g.drawText("Slot " + juce::String(index), area.removeFromLeft(60), juce::Justification::centred);

    // Draw gear item info if present
    if (gearItem != nullptr)
    {
        g.setColour(juce::Colours::black);
        g.setFont(12.0f);
        g.drawText(gearItem->name, area.removeFromLeft(120), juce::Justification::centredLeft);
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
    // For now, accept all drag sources - this can be refined later
    return true;
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

    // For now, just log the drop - full implementation will come later
    std::cout << "[RackSlot " << index << "] Item dropped" << std::endl;
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
