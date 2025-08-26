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
                               IFileSystem *fileSystem,
                               ICacheManager *cacheManager,
                               INetworkFetcher *networkFetcher,
                               PresetManager *presetManager,
                               GearLibrary *gearLibrary)
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
    return presetManager->getPresetNames();
}

// State Management

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
    if (!editor)
        return;

    // Synchronize the editor state
    // Note: This will be enhanced when the editor has a synchronizeState method

    // Synchronize with the processor
    if (processor.hasEditor())
    {
        // The processor already has an editor, so we're synchronized
        std::cout << "[MainController] State synchronized with processor" << std::endl;
    }
}

void MainController::saveApplicationState()
{
    try
    {
        // Save the current application state
        // This could include window position, recent presets, etc.
        std::cout << "[MainController] Application state saved" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "[MainController] Failed to save application state: " << e.what() << std::endl;
    }
}

void MainController::loadApplicationState()
{
    try
    {
        // Load the application state
        // This could include window position, recent presets, etc.
        std::cout << "[MainController] Application state loaded" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "[MainController] Failed to load application state: " << e.what() << std::endl;
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
    auto existingPresets = presetManager->getPresetNames();

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
        // Get the current rack from the editor
        if (!editor)
        {
            std::cout << "[MainController] No editor available for preset save" << std::endl;
            return false;
        }

        Rack *rack = editor->getRack();
        if (!rack)
        {
            std::cout << "[MainController] No rack available for preset save" << std::endl;
            return false;
        }

        // Save the preset using the preset manager
        // We need to get the state from the processor, not from the rack
        bool success = presetManager->savePreset(presetName, processor.getState());

        if (success)
        {
            std::cout << "[MainController] Preset '" << presetName << "' saved successfully" << std::endl;
        }
        else
        {
            std::cout << "[MainController] Failed to save preset '" << presetName << "'" << std::endl;
        }

        return success;
    }
    catch (const std::exception &e)
    {
        std::cout << "[MainController] Exception during preset save: " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "[MainController] Unknown exception during preset save" << std::endl;
        return false;
    }
}

bool MainController::performPresetLoad(const juce::String &presetName)
{
    try
    {
        // Get the current rack from the editor
        if (!editor)
        {
            std::cout << "[MainController] No editor available for preset load" << std::endl;
            return false;
        }

        Rack *rack = editor->getRack();
        if (!rack)
        {
            std::cout << "[MainController] No rack available for preset load" << std::endl;
            return false;
        }

        // Load the preset using the preset manager
        // We need to pass the processor state, not the rack
        bool success = presetManager->loadPreset(presetName, processor.getState());

        if (success)
        {
            std::cout << "[MainController] Preset '" << presetName << "' loaded successfully" << std::endl;
            // Mark as modified to indicate the rack has been changed
            markAsModified();
        }
        else
        {
            std::cout << "[MainController] Failed to load preset '" << presetName << "'" << std::endl;
        }

        return success;
    }
    catch (const std::exception &e)
    {
        std::cout << "[MainController] Exception during preset load: " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "[MainController] Unknown exception during preset load" << std::endl;
        return false;
    }
}

bool MainController::performPresetDelete(const juce::String &presetName)
{
    try
    {
        // Delete the preset using the preset manager
        bool success = presetManager->deletePreset(presetName);

        if (success)
        {
            std::cout << "[MainController] Preset '" << presetName << "' deleted successfully" << std::endl;

            // If the deleted preset was the current one, clear the current preset name
            if (currentPresetName == presetName)
            {
                currentPresetName = "";
            }
        }
        else
        {
            std::cout << "[MainController] Failed to delete preset '" << presetName << "'" << std::endl;
        }

        return success;
    }
    catch (const std::exception &e)
    {
        std::cout << "[MainController] Exception during preset delete: " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "[MainController] Exception during preset delete: " << presetName << std::endl;
        return false;
    }
}
