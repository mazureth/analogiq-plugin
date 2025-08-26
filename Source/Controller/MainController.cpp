/**
 * @file MainController.cpp
 * @brief Implementation of the MainController class.
 *
 * This file implements the main application controller that coordinates between
 * the Model and View layers, handling preset operations, state synchronization,
 * and user interactions.
 */

#include "MainController.h"
#include "../Model/GearLibrary.h"

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
MainController::MainController(AnalogIQProcessor &processor,
                               IFileSystem &fileSystem,
                               ICacheManager &cacheManager,
                               INetworkFetcher &networkFetcher,
                               PresetManager &presetManager,
                               GearLibrary &gearLibrary)
    : processor(processor),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      networkFetcher(networkFetcher),
      presetManager(presetManager),
      gearLibrary(gearLibrary)
{
    // Set component ID for debugging
    setComponentID("MainController");

    // Create the main editor
    editor = std::make_unique<AnalogIQEditor>(processor, fileSystem, cacheManager, presetManager, gearLibrary);

    // Add the editor to this component
    addAndMakeVisible(editor.get());
}

/**
 * @brief Destructor for MainController.
 */
MainController::~MainController()
{
    // Shutdown before destruction
    shutdown();
}

// Preset Operation Coordination

bool MainController::savePreset(const juce::String &presetName)
{
    // Validate the preset name
    juce::String errorMessage;
    if (!validatePresetName(presetName, errorMessage))
    {
        showError("Invalid Preset Name", errorMessage);
        return false;
    }

    // Check for name conflicts
    if (checkPresetNameConflict(presetName, errorMessage))
    {
        showError("Preset Name Conflict", errorMessage);
        return false;
    }

    // Perform the save operation
    if (performPresetSave(presetName))
    {
        currentPresetName = presetName;
        clearModifiedState();
        showInfo("Preset Saved", "Preset '" + presetName + "' has been saved successfully.");
        return true;
    }
    else
    {
        showError("Save Failed", "Failed to save preset '" + presetName + "'.");
        return false;
    }
}

bool MainController::loadPreset(const juce::String &presetName)
{
    // Check for unsaved changes
    if (hasUnsavedChanges())
    {
        if (!showConfirmation("Unsaved Changes",
                              "You have unsaved changes. Do you want to continue without saving?"))
        {
            return false;
        }
    }

    // Perform the load operation
    if (performPresetLoad(presetName))
    {
        currentPresetName = presetName;
        clearModifiedState();
        showInfo("Preset Loaded", "Preset '" + presetName + "' has been loaded successfully.");
        return true;
    }
    else
    {
        showError("Load Failed", "Failed to load preset '" + presetName + "'.");
        return false;
    }
}

bool MainController::deletePreset(const juce::String &presetName)
{
    // Confirm deletion
    if (!showConfirmation("Delete Preset",
                          "Are you sure you want to delete the preset '" + presetName + "'?"))
    {
        return false;
    }

    // Perform the delete operation
    if (performPresetDelete(presetName))
    {
        // If this was the current preset, clear the current preset name
        if (currentPresetName == presetName)
        {
            currentPresetName = "";
        }
        showInfo("Preset Deleted", "Preset '" + presetName + "' has been deleted successfully.");
        return true;
    }
    else
    {
        showError("Delete Failed", "Failed to delete preset '" + presetName + "'.");
        return false;
    }
}

juce::StringArray MainController::getPresetNames() const
{
    return presetManager.getPresetNames();
}

juce::String MainController::getCurrentPresetName() const
{
    return currentPresetName;
}

bool MainController::hasUnsavedChanges() const
{
    return isModified;
}

void MainController::markAsModified()
{
    isModified = true;
}

void MainController::clearModifiedState()
{
    isModified = false;
}

// State Synchronization

void MainController::synchronizeState()
{
    // Synchronize state between all components
    if (editor)
    {
        // Update editor state based on current application state
        // This will be implemented as the integration progresses
    }
}

void MainController::saveApplicationState()
{
    // Save the current application state
    if (editor)
    {
        // Get the rack state from the editor
        if (auto *rack = editor->getRack())
        {
            processor.saveInstanceStateFromRack(rack);
        }
    }
}

void MainController::loadApplicationState()
{
    // Load the application state
    if (editor)
    {
        // Load the rack state into the editor
        if (auto *rack = editor->getRack())
        {
            processor.loadInstanceState(rack);
        }
    }
}

// Error Handling

void MainController::showError(const juce::String &title, const juce::String &message)
{
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                           title,
                                           message);
}

void MainController::showInfo(const juce::String &title, const juce::String &message)
{
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                           title,
                                           message);
}

bool MainController::showConfirmation(const juce::String &title, const juce::String &message)
{
    // For now, use a simple confirmation dialog
    // This could be enhanced with more sophisticated confirmation handling
    auto *dialog = new juce::AlertWindow(title, message, juce::AlertWindow::QuestionIcon);

    dialog->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    bool result = false;
    dialog->enterModalState(true, juce::ModalCallbackFunction::create([&result, dialog](int buttonResult)
                                                                      {
        result = (buttonResult == 1);
        delete dialog; }),
                            true);

    return result;
}

// Component Lifecycle

void MainController::initialize()
{
    if (isInitialized)
        return;

    // Initialize the editor
    if (editor)
    {
        // Set up the editor
        editor->setSize(1200, 800);
    }

    // Load any saved application state
    loadApplicationState();

    isInitialized = true;
}

void MainController::shutdown()
{
    if (!isInitialized)
        return;

    // Save the current application state
    saveApplicationState();

    // Clean up resources
    isInitialized = false;
}

// JUCE Component overrides

void MainController::paint(juce::Graphics &g)
{
    // The main controller doesn't need to paint anything
    // It just contains the editor component
}

void MainController::resized()
{
    // Make the editor fill the entire controller area
    if (editor)
    {
        editor->setBounds(getLocalBounds());
    }
}

// Private helper methods

bool MainController::validatePresetName(const juce::String &presetName, juce::String &errorMessage)
{
    // Basic validation
    if (presetName.trim().isEmpty())
    {
        errorMessage = "Preset name cannot be empty.";
        return false;
    }

    if (presetName.length() > 100)
    {
        errorMessage = "Preset name cannot exceed 100 characters.";
        return false;
    }

    // Check for invalid characters
    if (presetName.containsAnyOf("\\/:*?\"<>|"))
    {
        errorMessage = "Preset name contains invalid characters.";
        return false;
    }

    return true;
}

bool MainController::checkPresetNameConflict(const juce::String &presetName, juce::String &errorMessage)
{
    auto existingPresets = presetManager.getPresetNames();

    for (const auto &existingName : existingPresets)
    {
        if (existingName.equalsIgnoreCase(presetName))
        {
            errorMessage = "A preset with this name already exists.";
            return true;
        }
    }

    return false;
}

bool MainController::performPresetSave(const juce::String &presetName)
{
    try
    {
        // Save the preset using the preset manager
        // For now, just mark as saved - full implementation will come later
        // when we have the complete preset saving logic
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool MainController::performPresetLoad(const juce::String &presetName)
{
    try
    {
        // Load the preset using the preset manager
        // For now, just mark as loaded - full implementation will come later
        // when we have the complete preset loading logic
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool MainController::performPresetDelete(const juce::String &presetName)
{
    try
    {
        // Delete the preset using the preset manager
        // For now, just mark as deleted - full implementation will come later
        // when we have the complete preset deletion logic
        return true;
    }
    catch (...)
    {
        return false;
    }
}
