/**
 * @file NotesPanel.cpp
 * @brief Implementation of the NotesPanel class, providing a text editor for user notes.
 *
 * This file implements the notes panel that:
 * - Displays a title and text editor
 * - Manages text input and display
 * - Handles component layout and styling
 * - Provides text persistence functionality
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#include "NotesPanel.h"

/**
 * @brief Constructs a new NotesPanel.
 *
 * Initializes the panel with:
 * - A centered title label with bold font
 * - A multi-line text editor with:
 *   - Scrollbars
 *   - Caret visibility
 *   - Popup menu
 *   - White background
 *   - Black text
 *   - Grey outline
 * - Default placeholder text
 */
NotesPanel::NotesPanel()
{
    // Set up title
    titleLabel.setText("Session Notes", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Set up text editor
    textEditor.setMultiLine(true);
    textEditor.setReturnKeyStartsNewLine(true);
    textEditor.setReadOnly(false);
    textEditor.setScrollbarsShown(true);
    textEditor.setCaretVisible(true);
    textEditor.setPopupMenuEnabled(true);
    textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    textEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
    textEditor.setText("Enter your session notes here. Document patchbay connections, settings, and any other important details.");
    addAndMakeVisible(textEditor);
}

/**
 * @brief Destructor for NotesPanel.
 */
NotesPanel::~NotesPanel()
{
}

/**
 * @brief Paints the panel background.
 *
 * Fills the background with a dark grey color.
 *
 * @param g The graphics context to paint with
 */
void NotesPanel::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
}

/**
 * @brief Called when the component is resized.
 *
 * Updates the layout of child components:
 * - Title label at the top with 40px height
 * - Text editor filling the remaining space
 * - 20px padding around all components
 */
void NotesPanel::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));

    // Text editor fills the rest
    textEditor.setBounds(bounds);
}

/**
 * @brief Sets the text content of the notes panel.
 *
 * Updates the text editor with the provided text.
 *
 * @param text The text to display in the editor
 */
void NotesPanel::setText(const juce::String &text)
{
    textEditor.setText(text);
}

/**
 * @brief Gets the current text content of the notes panel.
 *
 * Retrieves the current text from the editor.
 *
 * @return The current text in the editor
 */
juce::String NotesPanel::getText() const
{
    return textEditor.getText();
}