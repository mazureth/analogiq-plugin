/**
 * @file NotesPanel.cpp
 * @brief Implementation of the NotesPanel class for managing session notes.
 *
 * This file implements the NotesPanel class, which provides a text editor interface
 * for users to enter and manage session notes, including patchbay connections and
 * other important details.
 */

#include "NotesPanel.h"

/**
 * @brief Constructs a new NotesPanel.
 *
 * Initializes the panel with a title label and a text editor for entering notes.
 * Sets up the text editor with appropriate styling and default text.
 */
NotesPanel::NotesPanel()
{
    // Set up title
    titleLabel.setText("Session Notes", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Create viewport and container
    notesViewport = std::make_unique<juce::Viewport>();
    notesContainer = std::make_unique<NotesContainer>();
    notesViewport->setViewedComponent(notesContainer.get(), false); // false = don't delete when viewport is deleted
    addAndMakeVisible(notesViewport.get());

    // Set up the container
    notesContainer->panel = this;

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
    notesContainer->addAndMakeVisible(textEditor);
}

/**
 * @brief Destructor for NotesPanel.
 */
NotesPanel::~NotesPanel()
{
    // unique_ptr members will be automatically deleted
}

/**
 * @brief Paints the NotesPanel component.
 *
 * @param g The graphics context to paint with
 */
void NotesPanel::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
}

/**
 * @brief Handles resizing of the NotesPanel component.
 *
 * Arranges the title label and text editor within the panel's bounds.
 */
void NotesPanel::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));

    // Viewport fills the rest
    notesViewport->setBounds(bounds);

    // Size the container to match the viewport width
    const int containerWidth = notesViewport->getWidth();
    const int containerHeight = juce::jmax(notesViewport->getHeight(), 400); // Minimum height of 400
    notesContainer->setSize(containerWidth, containerHeight);

    // Text editor fills the container
    textEditor.setBounds(notesContainer->getLocalBounds().reduced(10));
}

/**
 * @brief Sets the text content of the notes panel.
 *
 * @param text The text to set in the editor
 */
void NotesPanel::setText(const juce::String &text)
{
    textEditor.setText(text);
}

/**
 * @brief Gets the current text content from the notes panel.
 *
 * @return The current text content as a string
 */
juce::String NotesPanel::getText() const
{
    return textEditor.getText();
}