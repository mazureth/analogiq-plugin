/**
 * @file NotesPanel.h
 * @brief Header file for the NotesPanel class.
 *
 * This file defines the NotesPanel class, which provides a text editor interface
 * for users to enter and manage session notes, including patchbay connections and
 * other important details.
 */

#pragma once

#include <JuceHeader.h>

/**
 * @brief A panel component for managing session notes.
 *
 * The NotesPanel class provides a user interface for entering and managing
 * session notes. It includes a title label and a text editor for entering
 * detailed notes about patchbay connections, settings, and other important
 * session information.
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
    void paint(juce::Graphics &) override;

    /**
     * @brief Handles resizing of the NotesPanel component.
     */
    void resized() override;

    /**
     * @brief Sets the text content of the notes panel.
     *
     * @param text The text to set in the editor
     */
    void setText(const juce::String &text);

    /**
     * @brief Gets the current text content from the notes panel.
     *
     * @return The current text content as a string
     */
    juce::String getText() const;

private:
    juce::TextEditor textEditor; ///< Text editor component for entering notes
    juce::Label titleLabel;      ///< Label displaying the panel title

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesPanel)
};