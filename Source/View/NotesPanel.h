/**
 * @file NotesPanel.h
 * @brief Header file for the NotesPanel class.
 *
 * This file defines a simple notes panel component for the AnalogIQ plugin,
 * providing a text area for session notes and documentation.
 */

#pragma once

#include <JuceHeader.h>

/**
 * @brief A simple notes panel component for session notes and documentation.
 *
 * The NotesPanel class provides a text area where users can write and edit
 * session notes, gear documentation, and other text-based information.
 */
class NotesPanel : public juce::Component
{
public:
    /**
     * @brief Constructs a new NotesPanel.
     */
    NotesPanel();

    /**
     * @brief Destructor for NotesPanel.
     */
    ~NotesPanel() override;

    /**
     * @brief Paints the NotesPanel component.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handles resizing of the NotesPanel component.
     */
    void resized() override;

    /**
     * @brief Gets the current notes text.
     *
     * @return The current notes text as a string
     */
    juce::String getNotes() const;

    /**
     * @brief Sets the notes text.
     *
     * @param notes The new notes text to set
     */
    void setNotes(const juce::String &notes);

    /**
     * @brief Clears all notes.
     */
    void clearNotes();

private:
    juce::TextEditor notesEditor; ///< Text editor for notes input

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesPanel)
};
