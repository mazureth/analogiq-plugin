/**
 * @file AnalogIQEditor.h
 * @brief Header file for the AnalogIQEditor class.
 *
 * This file defines the main editor interface for the AnalogIQ plugin,
 * providing a user interface for managing audio gear, racks, and session notes.
 * It includes a gear library, rack interface, and notes panel organized in a
 * tabbed layout.
 */

#pragma once

#include <JuceHeader.h>
#include "GearLibrary.h"
#include "Rack.h"
#include "NotesPanel.h"
#include "PresetManager.h"
#include "FileSystem.h"

// Forward declaration
class AnalogIQProcessor;

/**
 * @brief Main editor interface for the AnalogIQ plugin.
 *
 * The AnalogIQEditor class provides the user interface for the plugin,
 * managing the gear library, rack, and notes panel. It supports drag and drop
 * operations for gear items and organizes the interface using a tabbed layout.
 */
class AnalogIQEditor : public juce::AudioProcessorEditor,
                       public juce::DragAndDropContainer
{
public:
    /**
     * @brief Constructs a new AnalogIQEditor.
     *
     * @param processor Reference to the associated AudioProcessor
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     */
    AnalogIQEditor(AnalogIQProcessor &processor,
                   IFileSystem &fileSystem,
                   CacheManager &cacheManager,
                   PresetManager &presetManager,
                   GearLibrary &gearLibrary);

    /**
     * @brief Constructs a new AnalogIQEditor for testing.
     *
     * @param processor Reference to the associated AudioProcessor
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     * @param disableAutoLoad Whether to disable auto-loading of the gear library (for testing)
     */
    AnalogIQEditor(AnalogIQProcessor &processor, CacheManager &cacheManager, PresetManager &presetManager, bool disableAutoLoad);

    /**
     * @brief Destructor for AnalogIQEditor.
     */
    ~AnalogIQEditor() override;

    /**
     * @brief Paints the AnalogIQEditor component.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &) override;

    /**
     * @brief Handles resizing of the AnalogIQEditor component.
     */
    void resized() override;

    /**
     * @brief Gets a pointer to the rack component.
     *
     * @return Pointer to the Rack component
     */
    Rack *getRack() const { return rack.get(); }

    /**
     * @brief Gets a pointer to the gear library component.
     *
     * @return Pointer to the GearLibrary component
     */
    GearLibrary *getGearLibrary() const { return &gearLibrary; }

    /**
     * @brief Gets a reference to the preset manager.
     *
     * @return Reference to the PresetManager instance
     */
    PresetManager &getPresetManager() const { return presetManager; }

    /**
     * @brief Gets a pointer to the notes panel component.
     *
     * @return Pointer to the NotesPanel component
     */
    NotesPanel *getNotesPanel() const { return notesPanel.get(); }

private:
    /**
     * @brief Shows the presets popup menu.
     */
    void showPresetMenu();

    /**
     * @brief Shows a dialog to save a new preset.
     */
    void showSavePresetDialog();

    /**
     * @brief Shows a dialog to load a preset.
     */
    void showLoadPresetDialog();

    /**
     * @brief Shows a dialog to delete a preset.
     */
    void showDeletePresetDialog();

    /**
     * @brief Handles saving a preset with the given name.
     *
     * @param presetName The name of the preset to save
     */
    void handleSavePreset(const juce::String &presetName);

    /**
     * @brief Handles loading a preset with the given name.
     *
     * @param presetName The name of the preset to load
     */
    void handleLoadPreset(const juce::String &presetName);

    /**
     * @brief Performs the actual loading of a preset (called after confirmation if needed).
     *
     * @param presetName The name of the preset to load
     */
    void performLoadPreset(const juce::String &presetName);

    /**
     * @brief Handles deleting a preset with the given name.
     *
     * @param presetName The name of the preset to delete
     */
    void handleDeletePreset(const juce::String &presetName);

    /**
     * @brief Refreshes the preset menu with current preset list.
     */
    void refreshPresetMenu();

    /**
     * @brief Checks if the current rack state has unsaved changes.
     *
     * @return true if there are unsaved changes, false otherwise
     */
    bool hasUnsavedChanges() const;

    /**
     * @brief Marks the current state as having unsaved changes.
     */
    void markAsModified();

    /**
     * @brief Clears the modified state.
     */
    void clearModifiedState();

private:
    AnalogIQProcessor &processor;
    IFileSystem &fileSystem;
    CacheManager &cacheManager;
    PresetManager &presetManager;
    GearLibrary &gearLibrary;

    // UI Components
    juce::TabbedComponent mainTabs{juce::TabbedButtonBar::TabsAtTop}; ///< Main tabbed interface
    std::unique_ptr<Rack> rack;                                       ///< Rack component
    std::unique_ptr<NotesPanel> notesPanel;                           ///< Notes panel component

    // Menu Bar Components
    juce::TextButton presetsMenuButton{"PresetsMenuButton"}; ///< Menu button for preset operations

#ifdef JUCE_DEBUG
    // Debug Components
    juce::TextButton debugSaveButton{"DebugSaveButton"}; ///< Debug button for manual state save
    juce::TextButton debugLoadButton{"DebugLoadButton"}; ///< Debug button for manual state load
#endif

    // State tracking
    bool isModified{false};         ///< Tracks if the current state has unsaved changes
    juce::String currentPresetName; ///< Name of the currently loaded preset, if any

    /**
     * @brief Custom component for the menu bar container with styling.
     */
    class MenuBarContainer : public juce::Component
    {
    public:
        MenuBarContainer() = default;
        ~MenuBarContainer() override = default;

        void paint(juce::Graphics &g) override
        {
            // Draw menu bar background
            g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(0.1f));
            g.fillAll();

            // Draw bottom border
            g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(0.2f));
            g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
        }
    };

    MenuBarContainer menuBarContainer; ///< Container for the menu bar

    /**
     * @brief Custom LookAndFeel for menu buttons with no background or border.
     */
    class FlatMenuButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics &g, juce::Button &b,
                                  const juce::Colour &, bool, bool) override
        {
            // Do nothing: no background or border
        }
    };

    FlatMenuButtonLookAndFeel flatMenuLookAndFeel; ///< Custom look and feel for menu buttons

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQEditor)
};