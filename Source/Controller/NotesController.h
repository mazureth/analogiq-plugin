/**
 * @file NotesController.h
 * @brief Header file for the NotesController class.
 *
 * This file defines the notes controller that coordinates between
 * the NotesPanel view and the application state, handling notes
 * management and session information.
 */

#pragma once

#include <JuceHeader.h>
#include "../View/NotesPanel.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"

// Forward declarations
class PresetManager;

/**
 * @brief Controller for notes and session information management.
 *
 * The NotesController is responsible for:
 * - Coordinating between NotesPanel (View) and application state
 * - Managing notes content and persistence
 * - Handling notes search and filtering
 * - Managing session information
 * - Coordinating with preset management
 * - Handling notes export and import
 */
class NotesController
{
public:
    /**
     * @brief Constructs a new NotesController.
     *
     * @param notesPanel Reference to the notes panel view
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     */
    NotesController(NotesPanel &notesPanel,
                    IFileSystem &fileSystem,
                    ICacheManager &cacheManager,
                    PresetManager &presetManager);

    /**
     * @brief Destructor for NotesController.
     */
    ~NotesController() = default;

    // Notes Content Management
    /**
     * @brief Gets the current notes content.
     *
     * @return The current notes text
     */
    juce::String getNotesContent() const;

    /**
     * @brief Sets the notes content.
     *
     * @param content The new notes content
     * @return true if successful, false otherwise
     */
    bool setNotesContent(const juce::String &content);

    /**
     * @brief Appends text to the current notes.
     *
     * @param text The text to append
     * @return true if successful, false otherwise
     */
    bool appendToNotes(const juce::String &text);

    /**
     * @brief Clears all notes content.
     *
     * @return true if successful, false otherwise
     */
    bool clearNotes();

    /**
     * @brief Gets the notes character count.
     *
     * @return Number of characters in the notes
     */
    int getNotesCharacterCount() const;

    /**
     * @brief Gets the notes word count.
     *
     * @return Number of words in the notes
     */
    int getNotesWordCount() const;

    /**
     * @brief Gets the notes line count.
     *
     * @return Number of lines in the notes
     */
    int getNotesLineCount() const;

    // Notes Persistence
    /**
     * @brief Saves the current notes to a file.
     *
     * @param filePath The file path to save to
     * @return true if successful, false otherwise
     */
    bool saveNotesToFile(const juce::String &filePath);

    /**
     * @brief Loads notes from a file.
     *
     * @param filePath The file path to load from
     * @return true if successful, false otherwise
     */
    bool loadNotesFromFile(const juce::String &filePath);

    /**
     * @brief Auto-saves the current notes.
     *
     * @return true if successful, false otherwise
     */
    bool autoSaveNotes();

    /**
     * @brief Loads the last auto-saved notes.
     *
     * @return true if successful, false otherwise
     */
    bool loadAutoSavedNotes();

    // Notes Search and Filtering
    /**
     * @brief Searches for text within the notes.
     *
     * @param searchTerm The text to search for
     * @param caseSensitive Whether the search should be case sensitive
     * @return Array of search result positions
     */
    juce::Array<int> searchInNotes(const juce::String &searchTerm, bool caseSensitive = false);

    /**
     * @brief Replaces text within the notes.
     *
     * @param searchTerm The text to search for
     * @param replaceTerm The text to replace with
     * @param caseSensitive Whether the search should be case sensitive
     * @return Number of replacements made
     */
    int replaceInNotes(const juce::String &searchTerm, const juce::String &replaceTerm, bool caseSensitive = false);

    /**
     * @brief Finds the next occurrence of text.
     *
     * @param searchTerm The text to search for
     * @param startPosition The position to start searching from
     * @param caseSensitive Whether the search should be case sensitive
     * @return Position of the next occurrence, or -1 if not found
     */
    int findNext(const juce::String &searchTerm, int startPosition = 0, bool caseSensitive = false);

    /**
     * @brief Finds the previous occurrence of text.
     *
     * @param searchTerm The text to search for
     * @param startPosition The position to start searching from
     * @param caseSensitive Whether the search should be case sensitive
     * @return Position of the previous occurrence, or -1 if not found
     */
    int findPrevious(const juce::String &searchTerm, int startPosition = 0, bool caseSensitive = false);

    // Session Information
    /**
     * @brief Gets the current session name.
     *
     * @return The current session name
     */
    juce::String getSessionName() const;

    /**
     * @brief Sets the current session name.
     *
     * @param sessionName The new session name
     * @return true if successful, false otherwise
     */
    bool setSessionName(const juce::String &sessionName);

    /**
     * @brief Gets the session start time.
     *
     * @return The session start time
     */
    juce::Time getSessionStartTime() const;

    /**
     * @brief Gets the session duration.
     *
     * @return The session duration in milliseconds
     */
    juce::int64 getSessionDuration() const;

    /**
     * @brief Gets the session notes count.
     *
     * @return Number of notes created in this session
     */
    int getSessionNotesCount() const;

    // Preset Integration
    /**
     * @brief Saves notes to a preset.
     *
     * @param presetName The name of the preset
     * @return true if successful, false otherwise
     */
    bool saveNotesToPreset(const juce::String &presetName);

    /**
     * @brief Loads notes from a preset.
     *
     * @param presetName The name of the preset
     * @return true if successful, false otherwise
     */
    bool loadNotesFromPreset(const juce::String &presetName);

    /**
     * @brief Checks if notes have been modified since last save.
     *
     * @return true if modified, false otherwise
     */
    bool isNotesModified() const;

    /**
     * @brief Marks notes as modified.
     */
    void markNotesAsModified();

    /**
     * @brief Clears the modified state.
     */
    void clearModifiedState();

    // Notes Export and Import
    /**
     * @brief Exports notes to plain text format.
     *
     * @param filePath The file path to export to
     * @return true if successful, false otherwise
     */
    bool exportToPlainText(const juce::String &filePath);

    /**
     * @brief Exports notes to HTML format.
     *
     * @param filePath The file path to export to
     * @return true if successful, false otherwise
     */
    bool exportToHTML(const juce::String &filePath);

    /**
     * @brief Exports notes to Markdown format.
     *
     * @param filePath The file path to export to
     * @return true if successful, false otherwise
     */
    bool exportToMarkdown(const juce::String &filePath);

    /**
     * @brief Imports notes from plain text format.
     *
     * @param filePath The file path to import from
     * @return true if successful, false otherwise
     */
    bool importFromPlainText(const juce::String &filePath);

    /**
     * @brief Imports notes from HTML format.
     *
     * @param filePath The file path to import from
     * @return true if successful, false otherwise
     */
    bool importFromHTML(const juce::String &filePath);

    /**
     * @brief Imports notes from Markdown format.
     *
     * @param filePath The file path to import from
     * @return true if successful, false otherwise
     */
    bool importFromMarkdown(const juce::String &filePath);

    // Notes Statistics
    /**
     * @brief Gets notes creation statistics.
     *
     * @return Map of creation dates to note counts
     */
    std::map<juce::String, int> getNotesCreationStats() const;

    /**
     * @brief Gets notes modification statistics.
     *
     * @return Map of modification dates to note counts
     */
    std::map<juce::String, int> getNotesModificationStats() const;

    /**
     * @brief Gets the most frequently used words in notes.
     *
     * @param maxWords Maximum number of words to return
     * @return Array of word-frequency pairs
     */
    juce::Array<std::pair<juce::String, int>> getMostFrequentWords(int maxWords = 10) const;

private:
    // Core dependencies
    NotesPanel &notesPanel;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    PresetManager &presetManager;

    // State tracking
    juce::String currentNotesContent;
    juce::String sessionName;
    juce::Time sessionStartTime;
    bool isModified{false};
    int sessionNotesCount{0};

    // Private helper methods
    /**
     * @brief Updates the notes panel with current content.
     */
    void updateNotesPanel();

    /**
     * @brief Validates a file path.
     *
     * @param filePath The file path to validate
     * @return true if valid, false otherwise
     */
    bool isValidFilePath(const juce::String &filePath) const;

    /**
     * @brief Gets the auto-save file path.
     *
     * @return The auto-save file path
     */
    juce::String getAutoSaveFilePath() const;

    /**
     * @brief Parses HTML content to extract plain text.
     *
     * @param htmlContent The HTML content to parse
     * @return The extracted plain text
     */
    juce::String parseHTMLContent(const juce::String &htmlContent) const;

    /**
     * @brief Parses Markdown content to extract plain text.
     *
     * @param markdownContent The Markdown content to parse
     * @return The extracted plain text
     */
    juce::String parseMarkdownContent(const juce::String &markdownContent) const;

    /**
     * @brief Generates HTML content from plain text.
     *
     * @param plainText The plain text to convert
     * @return The generated HTML content
     */
    juce::String generateHTMLContent(const juce::String &plainText) const;

    /**
     * @brief Generates Markdown content from plain text.
     *
     * @param plainText The plain text to convert
     * @return The generated Markdown content
     */
    juce::String generateMarkdownContent(const juce::String &plainText) const;

    /**
     * @brief Counts words in a text string.
     *
     * @param text The text to count words in
     * @return Number of words
     */
    int countWords(const juce::String &text) const;

    /**
     * @brief Counts lines in a text string.
     *
     * @param text The text to count lines in
     * @return Number of lines
     */
    int countLines(const juce::String &text) const;
};
