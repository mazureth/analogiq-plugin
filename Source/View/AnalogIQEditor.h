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
#include "../Model/GearLibrary.h"
#include "../Model/PresetManager.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "NotesPanel.h"
#include "Rack.h"

// Forward declarations
class AnalogIQProcessor;
class GearLibraryTree;

/**
 * @brief Custom component for preset selection with scrollable list.
 */
class PresetSelectionComponent : public juce::Component,
                                 public juce::ListBoxModel
{
public:
    /**
     * @brief Callback function type for preset selection.
     */
    using PresetSelectedCallback = std::function<void(const juce::String &)>;

    /**
     * @brief Constructs a new PresetSelectionComponent.
     *
     * @param presetNames Array of preset names to display
     * @param callback Callback function called when a preset is selected
     * @param actionButtonText Text for the action button (e.g., "Load", "Delete")
     */
    PresetSelectionComponent(const juce::StringArray &presetNames, PresetSelectedCallback callback, const juce::String &actionButtonText = "Select");

    /**
     * @brief Destructor.
     */
    ~PresetSelectionComponent() override = default;

    // juce::Component overrides
    void paint(juce::Graphics &g) override;
    void resized() override;

    // juce::ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent &e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent &e) override;

    // Helper methods for AlertWindow integration
    int getSelectedRow() const;
    const juce::StringArray &getPresetNames() const;

private:
    juce::StringArray presetNames;
    PresetSelectedCallback callback;
    juce::ListBox listBox;
    juce::String actionButtonText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelectionComponent)
};

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
     * @param fileSystem Pointer to the file system
     * @param cacheManager Pointer to the cache manager
     * @param presetManager Pointer to the preset manager
     * @param gearLibrary Pointer to the gear library
     */
    AnalogIQEditor(AnalogIQProcessor &processor,
                   IFileSystem *fileSystem,
                   ICacheManager *cacheManager,
                   PresetManager *presetManager,
                   GearLibrary *gearLibrary);

    /**
     * @brief Constructs a new AnalogIQEditor for testing.
     *
     * @param processor Reference to the associated AudioProcessor
     * @param cacheManager Pointer to the cache manager
     * @param presetManager Pointer to the preset manager
     * @param disableAutoLoad Whether to disable auto-loading of the gear library (for testing)
     */
    AnalogIQEditor(AnalogIQProcessor &processor, ICacheManager *cacheManager, PresetManager *presetManager, bool disableAutoLoad);

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
    GearLibrary *getGearLibrary() const { return gearLibrary; }

    /**
     * @brief Gets a pointer to the preset manager.
     *
     * @return Pointer to the PresetManager instance
     */
    PresetManager *getPresetManager() const { return presetManager; }

    /**
     * @brief Gets a pointer to the notes panel component.
     *
     * @return Pointer to the NotesPanel component
     */
    NotesPanel *getNotesPanel() const { return notesPanel.get(); }

    /**
     * @brief Notifies the processor that this editor is being destroyed.
     * This allows the processor to clear any stored references to prevent dangling pointers.
     */
    void notifyDestruction();

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
     * @brief Shows a preset selection dialog with scrollable list.
     *
     * @param title Dialog title
     * @param message Dialog message
     * @param actionButtonText Text for the action button
     * @param presetNames Array of preset names to display
     * @param callback Callback function called when a preset is selected
     */
    void showPresetSelectionDialog(const juce::String &title,
                                   const juce::String &message,
                                   const juce::String &actionButtonText,
                                   const juce::StringArray &presetNames,
                                   std::function<void(const juce::String &)> callback);

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

#ifdef JUCE_DEBUG
    /**
     * @brief Clears all cache data (simulates fresh install).
     */
    void clearAllCache();

    /**
     * @brief Clears only gear library cache.
     */
    void clearGearLibraryCache();

    /**
     * @brief Clears only preset cache.
     */
    void clearPresetCache();

    /**
     * @brief Simulates a complete fresh install by clearing all data.
     */
    void simulateFreshInstall();
#endif

private:
    AnalogIQProcessor &processor;
    IFileSystem *fileSystem;
    ICacheManager *cacheManager;
    PresetManager *presetManager;
    GearLibrary *gearLibrary;

    // UI Components
    juce::TabbedComponent mainTabs{juce::TabbedButtonBar::TabsAtTop}; ///< Main tabbed interface
    std::unique_ptr<GearLibraryTree> gearLibraryTree;                 ///< Gear library tree component
    std::unique_ptr<Rack> rack;                                       ///< Rack component
    std::unique_ptr<NotesPanel> notesPanel;                           ///< Notes panel component

    // Menu Bar Components
    juce::TextButton presetsMenuButton{"PresetsMenuButton"}; ///< Menu button for preset operations

#ifdef JUCE_DEBUG
    // Debug Components
    juce::TextButton debugSaveButton{"DebugSaveButton"};                 ///< Debug button for manual state save
    juce::TextButton debugLoadButton{"DebugLoadButton"};                 ///< Debug button for manual state load
    juce::TextButton debugClearCacheButton{"DebugClearCacheButton"};     ///< Debug button for clearing all cache
    juce::TextButton debugClearGearButton{"DebugClearGearButton"};       ///< Debug button for clearing gear library cache
    juce::TextButton debugClearPresetsButton{"DebugClearPresetsButton"}; ///< Debug button for clearing presets
    juce::TextButton debugFreshInstallButton{"DebugFreshInstallButton"}; ///< Debug button for simulating fresh install
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
