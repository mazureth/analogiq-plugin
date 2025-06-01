/**
 * @file DraggableListBox.h
 * @brief A custom ListBox implementation that supports drag and drop operations.
 *
 * This class extends JUCE's ListBox to add drag and drop functionality,
 * allowing list items to be dragged to other components that implement
 * DragAndDropTarget. It includes detailed debug logging to help track
 * drag and drop operations.
 */

#pragma once

#include <JuceHeader.h>

/**
 * @brief A ListBox that supports drag and drop operations.
 *
 * This class extends JUCE's ListBox to add drag and drop functionality.
 * It tracks mouse events to initiate drag operations and provides
 * visual feedback during dragging. Includes comprehensive debug logging
 * to help diagnose drag and drop issues.
 */
class DraggableListBox : public juce::ListBox
{
public:
    /**
     * @brief Constructs a new DraggableListBox.
     *
     * @param name The name of the component
     * @param model The ListBoxModel to use for data
     */
    DraggableListBox(const juce::String &name, juce::ListBoxModel *model)
        : juce::ListBox(name, model)
    {
        // Set a descriptive ID for debugging
        setComponentID("DraggableListBox");

        // CRITICAL: Ensure listbox captures mouse events correctly
        setWantsKeyboardFocus(true);

        // Debugging info - log when object is created
        DBG("DraggableListBox constructed - ID: " + getComponentID());

        // Add a mouse listener to log clicks for debugging
        addMouseListener(this, true);
    }

    /**
     * @brief Handles mouse down events.
     *
     * Initiates the drag operation by tracking the clicked row and
     * initial position. Includes detailed debug logging.
     *
     * @param e The mouse event details
     */
    void mouseDown(const juce::MouseEvent &e) override
    {
        // First, find out which row was clicked - CRITICAL FIX
        int row = getRowContainingPosition(e.x, e.y);
        draggedRow = row;

        // Track the initial position for the drag
        dragStartPosition = e.getPosition();

        // VERY CLEAR debug output - CRITICAL FOR IDENTIFYING ISSUES
        juce::String msg = "\n\n****** MOUSE DOWN DETECTED ON DRAGGABLE LISTBOX ******\n";
        msg += "Row clicked: " + juce::String(draggedRow) + "\n";
        msg += "Position: " + dragStartPosition.toString() + "\n";
        msg += "Number of rows: " + juce::String(getModel() ? getModel()->getNumRows() : 0) + "\n";
        msg += "Row height: " + juce::String(getRowHeight()) + "\n";
        msg += "Selected row: " + juce::String(getSelectedRow()) + "\n";
        msg += "***************************************************\n\n";
        DBG(msg);

        // Let the base class handle selection
        ListBox::mouseDown(e);

        // IMPORTANT: Store the selected row after the base class has handled it
        draggedRow = getSelectedRow();
        DBG("After ListBox::mouseDown, selected row is now: " + juce::String(draggedRow));
    }

    /**
     * @brief Handles mouse drag events.
     *
     * Manages the drag operation, including:
     * - Checking if drag should start
     * - Creating drag image
     * - Initiating drag operation
     * - Debug logging
     *
     * @param e The mouse event details
     */
    void mouseDrag(const juce::MouseEvent &e) override
    {
        // Only initiate drag if:
        // 1. We have a valid row
        // 2. The mouse has moved a reasonable distance (to avoid accidental drags)
        // 3. We haven't already started dragging

        // CLEAR debug output every time mouseDrag is called
        DBG("DraggableListBox: mouseDrag called, draggedRow=" + juce::String(draggedRow) +
            ", selectedRow=" + juce::String(getSelectedRow()) +
            ", isDragging=" + juce::String(isDragging) +
            ", distance=" + juce::String(e.getDistanceFromDragStart()));

        // CRITICAL FIX: Use the selected row if the dragged row is invalid
        int rowToDrag = (draggedRow >= 0) ? draggedRow : getSelectedRow();

        if (rowToDrag >= 0 && !isDragging && e.getDistanceFromDragStart() > 5)
        {
            isDragging = true;

            // Find the top-level DragAndDropContainer component
            auto *dndContainer = juce::DragAndDropContainer::findParentDragContainerFor(this);

            if (dndContainer)
            {
                DBG("DraggableListBox: Starting drag operation for row " + juce::String(rowToDrag));

                // Create drag data (simple integer with row index) - CRITICAL FIX: use rowToDrag
                juce::var dragData(rowToDrag);

                // Create simple drag image that clearly shows which row is being dragged
                juce::Image dragImage(juce::Image::ARGB, 200, 40, true);
                juce::Graphics g(dragImage);
                g.fillAll(juce::Colours::lime.withAlpha(0.8f)); // Brighter color to see the drag image
                g.setColour(juce::Colours::black);

                // Show more info in the drag image for debugging
                if (rowToDrag < getModel()->getNumRows())
                {
                    juce::String itemText = "Row " + juce::String(rowToDrag);
                    g.drawText(itemText,
                               dragImage.getBounds().reduced(2),
                               juce::Justification::centred,
                               true);
                }

                // Start the actual drag operation with a bright visible image
                dndContainer->startDragging(dragData, this, juce::ScaledImage(dragImage), false);

                DBG("DraggableListBox: Drag operation started - container ID: " +
                    dndContainer->getComponentID() + ", row: " + juce::String(rowToDrag));
            }
            else
            {
                DBG("DraggableListBox: ERROR - No DragAndDropContainer found!");

                // Show component hierarchy for debugging
                juce::Component *parent = getParentComponent();
                while (parent != nullptr)
                {
                    DBG("Parent component: ID=" +
                        parent->getComponentID() + ", class=" +
                        typeid(*parent).name());
                    parent = parent->getParentComponent();
                }
            }
        }

        // Always let the base class handle the event too
        ListBox::mouseDrag(e);
    }

    /**
     * @brief Handles mouse up events.
     *
     * Resets drag state and cleans up after drag operation.
     *
     * @param e The mouse event details
     */
    void mouseUp(const juce::MouseEvent &e) override
    {
        // Reset drag state
        isDragging = false;
        draggedRow = -1;

        // Let the base class handle the event
        ListBox::mouseUp(e);
    }

private:
    bool isDragging = false;            ///< Whether a drag operation is in progress
    int draggedRow = -1;                ///< The index of the row being dragged
    juce::Point<int> dragStartPosition; ///< The starting position of the drag operation
};