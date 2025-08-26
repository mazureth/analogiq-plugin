/**
 * @file MainController.h
 * @brief Header file for the MainController class.
 *
 * This file defines the main application controller that coordinates between
 * the Model and View layers, handling preset operations, state synchronization,
 * and user interactions.
 */

#pragma once

#include <JuceHeader.h>
#include "../Model/AnalogIQProcessor.h"
#include "../Model/PresetManager.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/INetworkFetcher.h"
#include "../View/AnalogIQEditor.h"

// Forward declarations
class GearLibrary;

/**
 * @brief Main application controller that coordinates between Model and View layers.
 *
 * The MainController is responsible for:
 * - Coordinating preset operations (save/load/delete)
 * - Managing application state and synchronization
 * - Handling user interactions from the main editor
 * - Coordinating with external services
 * - Managing error handling and user feedback
 * - Handling plugin lifecycle events
 */
class MainController : public juce::Component
{
public:
    /**
     * @brief Constructs a new MainController.
     *
     * @param processor Reference to the main audio processor
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     * @param networkFetcher Reference to the network fetcher
     * @param presetManager Reference to the preset manager
     * @param gearLibrary Reference to the gear library
     */
    MainController(AnalogIQProcessor &processor,
                   IFileSystem &fileSystem,
                   ICacheManager &cacheManager,
                   INetworkFetcher &networkFetcher,
                   PresetManager &presetManager,
                   GearLibrary &gearLibrary);

    /**
     * @brief Destructor for MainController.
     */
    ~MainController() override;

    /**
     * @brief Gets the main editor component.
     *
     * @return Pointer to the AnalogIQEditor component
     */
    AnalogIQEditor *getEditor() const { return editor.get(); }

    // Preset Operation Coordination
    /**
     * @brief Coordinates preset save operations.
     *
     * @param presetName The name for the new preset
     * @return true if the preset was saved successfully, false otherwise
     */
    bool savePreset(const juce::String &presetName);

    /**
     * @brief Coordinates preset load operations.
     *
     * @param presetName The name of the preset to load
     * @return true if the preset was loaded successfully, false otherwise
     */
    bool loadPreset(const juce::String &presetName);

    /**
     * @brief Coordinates preset delete operations.
     *
     * @param presetName The name of the preset to delete
     * @return true if the preset was deleted successfully, false otherwise
     */
    bool deletePreset(const juce::String &presetName);

    /**
     * @brief Gets the list of available preset names.
     *
     * @return Array of preset names
     */
    juce::StringArray getPresetNames() const;

    /**
     * @brief Gets the current preset name.
     *
     * @return The current preset name, or empty string if none
     */
    juce::String getCurrentPresetName() const;

    /**
     * @brief Checks if there are unsaved changes.
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

    // State Synchronization
    /**
     * @brief Synchronizes state between all components.
     */
    void synchronizeState();

    /**
     * @brief Saves the current application state.
     */
    void saveApplicationState();

    /**
     * @brief Loads the application state.
     */
    void loadApplicationState();

    // Error Handling
    /**
     * @brief Displays an error message to the user.
     *
     * @param title The error dialog title
     * @param message The error message
     */
    void showError(const juce::String &title, const juce::String &message);

    /**
     * @brief Displays an information message to the user.
     *
     * @param title The info dialog title
     * @param message The info message
     */
    void showInfo(const juce::String &title, const juce::String &message);

    /**
     * @brief Shows a confirmation dialog.
     *
     * @param title The dialog title
     * @param message The confirmation message
     * @return true if the user confirmed, false otherwise
     */
    bool showConfirmation(const juce::String &title, const juce::String &message);

    // Component Lifecycle
    /**
     * @brief Initializes the controller and all components.
     */
    void initialize();

    /**
     * @brief Shuts down the controller and cleans up resources.
     */
    void shutdown();

    // JUCE Component overrides
    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    // Core dependencies
    AnalogIQProcessor &processor;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    INetworkFetcher &networkFetcher;
    PresetManager &presetManager;
    GearLibrary &gearLibrary;

    // UI Components
    std::unique_ptr<AnalogIQEditor> editor;

    // State tracking
    bool isModified{false};
    juce::String currentPresetName;
    bool isInitialized{false};

    // Private helper methods
    /**
     * @brief Validates a preset name.
     *
     * @param presetName The preset name to validate
     * @param errorMessage Output parameter for error message
     * @return true if the name is valid, false otherwise
     */
    bool validatePresetName(const juce::String &presetName, juce::String &errorMessage);

    /**
     * @brief Checks for preset name conflicts.
     *
     * @param presetName The preset name to check
     * @param errorMessage Output parameter for error message
     * @return true if there's a conflict, false otherwise
     */
    bool checkPresetNameConflict(const juce::String &presetName, juce::String &errorMessage);

    /**
     * @brief Performs the actual preset save operation.
     *
     * @param presetName The preset name
     * @return true if successful, false otherwise
     */
    bool performPresetSave(const juce::String &presetName);

    /**
     * @brief Performs the actual preset load operation.
     *
     * @param presetName The preset name
     * @return true if successful, false otherwise
     */
    bool performPresetLoad(const juce::String &presetName);

    /**
     * @brief Performs the actual preset delete operation.
     *
     * @param presetName The preset name
     * @return true if successful, false otherwise
     */
    bool performPresetDelete(const juce::String &presetName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainController)
};
