/**
 * @file NotesPanel.h
 * @brief Header file for the NotesPanel class, which provides a text editor for user notes.
 *
 * The NotesPanel class is responsible for:
 * - Displaying and editing user notes
 * - Managing text persistence
 * - Providing a simple text editing interface
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>

/**
 * @class NotesPanel
 * @brief A component that provides a text editor for user notes.
 *
 * NotesPanel provides a simple interface for users to:
 * - View and edit notes about their gear setup
 * - Save and load notes
 * - Format text as needed
 */
class NotesPanel : public juce::Component
{
public:
    /**
     * @brief Constructs a new NotesPanel.
     *
     * Initializes the panel with:
     * - A title label
     * - A text editor for note input
     */
    NotesPanel();

    /**
     * @brief Destructor for NotesPanel.
     */
    ~NotesPanel() override;

    /**
     * @brief Paints the panel background and components.
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &) override;

    /**
     * @brief Called when the component is resized.
     * Updates the layout of the text editor and title label.
     */
    void resized() override;

    /**
     * @brief Sets the text content of the notes panel.
     * @param text The text to display in the editor
     */
    void setText(const juce::String &text);

    /**
     * @brief Gets the current text content of the notes panel.
     * @return The current text in the editor
     */
    juce::String getText() const;

private:
    juce::TextEditor textEditor; ///< The text editor component for note input
    juce::Label titleLabel;      ///< The title label for the panel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesPanel)
};