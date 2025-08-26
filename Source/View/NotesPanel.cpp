/**
 * @file NotesPanel.cpp
 * @brief Implementation of the NotesPanel class.
 *
 * This file implements a simple notes panel component for the AnalogIQ plugin,
 * providing a text area for session notes and documentation.
 */

#include "NotesPanel.h"

/**
 * @brief Constructs a new NotesPanel.
 *
 * Initializes the notes editor with default settings and placeholder text.
 */
NotesPanel::NotesPanel()
{
    // Set component ID for debugging
    setComponentID("NotesPanel");

    // Configure the notes editor
    notesEditor.setMultiLine(true);
    notesEditor.setReturnKeyStartsNewLine(true);
    notesEditor.setScrollbarsShown(true);
    notesEditor.setCaretVisible(true);
    notesEditor.setPopupMenuEnabled(true);
    notesEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    notesEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    notesEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::lightgrey);
    notesEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::blue);

    // Set placeholder text
    notesEditor.setTextToShowWhenEmpty("Enter your session notes here...", juce::Colours::darkgrey);

    // Add the editor to this component
    addAndMakeVisible(notesEditor);
}

/**
 * @brief Destructor for NotesPanel.
 */
NotesPanel::~NotesPanel()
{
    // The notesEditor will be cleaned up automatically
}

/**
 * @brief Paints the NotesPanel component.
 *
 * Fills the background with a light color and draws a border.
 *
 * @param g The graphics context to paint with
 */
void NotesPanel::paint(juce::Graphics &g)
{
    // Fill background
    g.fillAll(juce::Colours::lightgrey);

    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

/**
 * @brief Handles resizing of the NotesPanel component.
 *
 * Resizes the notes editor to fill the available space with a small margin.
 */
void NotesPanel::resized()
{
    auto area = getLocalBounds();

    // Add a small margin around the editor
    area.reduce(5, 5);

    notesEditor.setBounds(area);
}

/**
 * @brief Gets the current notes text.
 *
 * @return The current notes text as a string
 */
juce::String NotesPanel::getNotes() const
{
    return notesEditor.getText();
}

/**
 * @brief Sets the notes text.
 *
 * @param notes The new notes text to set
 */
void NotesPanel::setNotes(const juce::String &notes)
{
    notesEditor.setText(notes);
}

/**
 * @brief Clears all notes.
 */
void NotesPanel::clearNotes()
{
    notesEditor.clear();
}
