/**
 * @file AnalogIQEditor.cpp
 * @brief Implementation of the AnalogIQEditor class.
 *
 * This file implements the main editor interface for the AnalogIQ plugin,
 * providing a user interface for managing audio gear, racks, and session notes.
 * It includes a gear library, rack interface, and notes panel organized in a
 * tabbed layout.
 */

#include "AnalogIQEditor.h"
#include "../Model/AnalogIQProcessor.h"
#include "GearLibraryTree.h"
#include "../Shared/CrashLogger.h"

/**
 * @brief Constructs a new AnalogIQEditor.
 *
 * Initializes the editor with a gear library, rack, and notes panel.
 * Sets up the tabbed interface and configures drag and drop functionality.
 *
 * @param processor Reference to the associated AudioProcessor
 * @param fileSystem Reference to the file system
 * @param cacheManager Reference to the cache manager
 * @param presetManager Reference to the preset manager
 * @param gearLibrary Reference to the gear library
 */
AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor &processor,
                               IFileSystem *fileSystem,
                               ICacheManager *cacheManager,
                               PresetManager *presetManager,
                               GearLibrary *gearLibrary)
    : AudioProcessorEditor(&processor),
      processor(processor),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(gearLibrary),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Log editor creation
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - START - Creating new editor instance");
    CrashLogger::getInstance().log("CONSTRUCTOR", "AnalogIQEditor constructor started");

    setComponentID("AnalogIQEditor");

    // Create GearLibraryTree component (to the left of the rack)
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - Creating GearLibraryTree");
    CrashLogger::getInstance().log("CONSTRUCTOR", "Creating GearLibraryTree");
    gearLibraryTree = std::make_unique<GearLibraryTree>(*gearLibrary, *cacheManager, *presetManager);
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - GearLibraryTree created successfully");
    CrashLogger::getInstance().log("CONSTRUCTOR", "GearLibraryTree created successfully");

    // Create Rack component
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - Creating Rack");
    CrashLogger::getInstance().log("CONSTRUCTOR", "Creating Rack - RackModel instance ID: " + juce::String(processor.getRackModel()->getInstanceId()));
    rack = std::make_unique<Rack>(*processor.getRackModel());
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - Rack created successfully");
    CrashLogger::getInstance().log("CONSTRUCTOR", "Rack created successfully - connected to RackModel instance ID: " + juce::String(processor.getRackModel()->getInstanceId()));

    // Create NotesPanel component
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - Creating NotesPanel");
    notesPanel = std::make_unique<NotesPanel>();
    juce::Logger::writeToLog("AnalogIQEditor::AnalogIQEditor - NotesPanel created successfully");

    // Set component IDs for Rack and Notes tabs
    rack->setComponentID("RackTab");
    notesPanel->setComponentID("NotesTab");

    // Set up main window size
    setSize(1200, 800);

    // Set up tabs
    mainTabs.setComponentID("MainTabs");

    // Add Rack and Notes tabs
    mainTabs.addTab("Rack", juce::Colours::darkgrey, rack.get(), false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, notesPanel.get(), false);

    mainTabs.setTabBarDepth(30);
    mainTabs.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(mainTabs);

    // Add GearLibraryTree to visible components
    addAndMakeVisible(gearLibraryTree.get());

    // CRITICAL FIX: Set initial bounds immediately after adding to visible components
    // This ensures the component is visible even if resized() is never called
    int initialTreeWidth = getWidth() > 0 ? getWidth() / 3 : 200;          // Default width if getWidth() returns 0
    gearLibraryTree->setBounds(0, 30, initialTreeWidth, getHeight() - 30); // Position below menu bar

    // Register GearLibraryTree as a listener for rack state changes
    // This allows the tree to refresh when gear is added/removed from the rack
    processor.getRackModel()->addRackStateListener(static_cast<RackStateListener *>(gearLibraryTree.get()));

    // CRITICAL FIX: Call resized() to ensure proper layout initialization
    resized();

    // Set up menu bar components
    menuBarContainer.setComponentID("MenuBarContainer");
    presetsMenuButton.setComponentID("PresetsMenuButton");

    // Configure preset menu button with onClick lambda
    presetsMenuButton.setButtonText("Presets");
    presetsMenuButton.onClick = [this]()
    { showPresetMenu(); };

    // Apply custom look and feel for no background/border
    presetsMenuButton.setLookAndFeel(&flatMenuLookAndFeel);

    // Style the text color
    presetsMenuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    presetsMenuButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    // Add menu bar components to the editor
    addAndMakeVisible(menuBarContainer);
    addAndMakeVisible(presetsMenuButton);

    // Set up menu bar styling
    menuBarContainer.setOpaque(true);

#ifdef JUCE_DEBUG
    // Configure debug buttons
    debugSaveButton.setButtonText("Debug: Save State");
    debugSaveButton.onClick = [this]()
    {
        // Simulate the full getStateInformation process
        juce::MemoryBlock destData;
        this->processor.getStateInformation(destData);

        // Log the result
    };
    addAndMakeVisible(debugSaveButton);

    debugLoadButton.setButtonText("Debug: Load State");
    debugLoadButton.onClick = [this]()
    {
        if (auto *rack = getRack())
        {
            this->processor.loadInstanceState(rack);
        }
    };
    addAndMakeVisible(debugLoadButton);

    debugClearCacheButton.setButtonText("Clear All Cache");
    debugClearCacheButton.onClick = [this]()
    { clearAllCache(); };
    addAndMakeVisible(debugClearCacheButton);

    debugClearPresetsButton.setButtonText("Clear Presets");
    debugClearPresetsButton.onClick = [this]()
    { clearPresetCache(); };
    addAndMakeVisible(debugClearPresetsButton);

    debugFreshInstallButton.setButtonText("Fresh Install");
    debugFreshInstallButton.onClick = [this]()
    { simulateFreshInstall(); };
    addAndMakeVisible(debugFreshInstallButton);
#endif

    // Configure drag and drop
    // This is critical - make sure this component is configured as the DragAndDropContainer
    setInterceptsMouseClicks(false, true);

    // Note: GearLibrary loading is handled by the Model layer, not the UI
}

/**
 * @brief Constructs a new AnalogIQEditor for testing.
 *
 * Initializes the editor with a gear library, rack, and notes panel.
 * Sets up the tabbed interface and configures drag and drop functionality.
 *
 * @param processor Reference to the associated AudioProcessor
 * @param cacheManager Reference to the cache manager
 * @param presetManager Reference to the preset manager
 * @param disableAutoLoad Whether to disable auto-loading of the gear library (for testing)
 */
AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor &processor, ICacheManager *cacheManager, PresetManager *presetManager, bool disableAutoLoad)
    : AudioProcessorEditor(&processor),
      processor(processor),
      fileSystem(processor.getFileSystem()),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(processor.getGearLibrary()),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    setComponentID("AnalogIQEditor");

    // Create GearLibraryTree component (to the left of the rack)
    gearLibraryTree = std::make_unique<GearLibraryTree>(*gearLibrary, *cacheManager, *presetManager);

    // Add GearLibraryTree to visible components
    addAndMakeVisible(gearLibraryTree.get());

    // CRITICAL FIX: Set initial bounds immediately after adding to visible components
    // This ensures the component is visible even if resized() is never called
    int initialTreeWidth = getWidth() > 0 ? getWidth() / 3 : 200;          // Default width if getWidth() returns 0
    gearLibraryTree->setBounds(0, 30, initialTreeWidth, getHeight() - 30); // Position below menu bar

    // Debug: Log the component hierarchy

    // CRITICAL FIX: Call resized() to ensure proper layout initialization
    resized();

    // Create Rack component
    rack = std::make_unique<Rack>(*processor.getRackModel());

    // Create NotesPanel component
    notesPanel = std::make_unique<NotesPanel>();

    // Set component IDs for Rack and Notes tabs
    rack->setComponentID("RackTab");
    notesPanel->setComponentID("NotesTab");

    // Set up main window size
    setSize(1200, 800);

    // Set up tabs
    mainTabs.setComponentID("MainTabs");

    // Add Rack and Notes tabs
    mainTabs.addTab("Rack", juce::Colours::darkgrey, rack.get(), false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, notesPanel.get(), false);

    mainTabs.setTabBarDepth(30);
    mainTabs.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(mainTabs);

    // GearLibraryTree is now properly added to the UI with addAndMakeVisible()

    // Set up menu bar components
    menuBarContainer.setComponentID("MenuBarContainer");
    presetsMenuButton.setComponentID("PresetsMenuButton");

    // Configure preset menu button with onClick lambda
    presetsMenuButton.setButtonText("Presets");
    presetsMenuButton.onClick = [this]()
    { showPresetMenu(); };

    // Apply custom look and feel for no background/border
    presetsMenuButton.setLookAndFeel(&flatMenuLookAndFeel);

    // Style the text color
    presetsMenuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    presetsMenuButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    // Add menu bar components to the editor
    addAndMakeVisible(menuBarContainer);
    addAndMakeVisible(presetsMenuButton);

    // Set up menu bar styling
    menuBarContainer.setOpaque(true);

    // Configure drag and drop
    // This is critical - make sure this component is configured as the DragAndDropContainer
    setInterceptsMouseClicks(false, true);

    // Note: GearLibrary loading is handled by the Model layer, not the UI
}

/**
 * @brief Destructor for AnalogIQEditor.
 *
 * Cleans up all components through unique_ptr automatic destruction.
 */
AnalogIQEditor::~AnalogIQEditor()
{
    juce::Logger::writeToLog("AnalogIQEditor::~AnalogIQEditor - START - Destroying editor instance");
    CrashLogger::getInstance().log("DESTRUCTOR", "AnalogIQEditor destructor started");

    // Notify the processor that we're being destroyed so it can clear stored references
    // TODO: Implement when processor has clearRackReference method
    // processor.clearRackReference();

    // CRITICAL: Clear LookAndFeel reference before destruction to avoid JUCE assertion
    // This prevents "LookAndFeel object being destroyed while something is still using it"
    juce::Logger::writeToLog("AnalogIQEditor::~AnalogIQEditor - Clearing LookAndFeel reference");
    presetsMenuButton.setLookAndFeel(nullptr);

    // The unique_ptrs will clean up automatically
    juce::Logger::writeToLog("AnalogIQEditor::~AnalogIQEditor - END - Editor destruction complete");
    CrashLogger::getInstance().log("DESTRUCTOR", "AnalogIQEditor destructor completed");
}

/**
 * @brief Paints the AnalogIQEditor component.
 *
 * Fills the background with the default window background color.
 *
 * @param g The graphics context to paint with
 */
void AnalogIQEditor::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

/**
 * @brief Handles resizing of the AnalogIQEditor component.
 *
 * Arranges the menu bar at the top and the tabbed interface (rack and notes) in the remaining space.
 */
void AnalogIQEditor::resized()
{
    juce::Logger::writeToLog("AnalogIQEditor::resized - START - Resizing editor components");
    CrashLogger::getInstance().log("RESIZE", "AnalogIQEditor resized() called");
    auto area = getLocalBounds();
    juce::Logger::writeToLog("AnalogIQEditor::resized - Local bounds: " + area.toString());

    // Top area: Menu bar (full width)
    auto menuBarArea = area.removeFromTop(30);
    menuBarContainer.setBounds(menuBarArea);

    // Position preset menu button on the left side of the menu bar
    presetsMenuButton.setBounds(menuBarArea.removeFromLeft(80));

#ifdef JUCE_DEBUG
    // Position debug buttons on the right side of the menu bar
    // Each button gets 100px width, positioned from right to left
    debugFreshInstallButton.setBounds(menuBarArea.removeFromRight(100));
    debugClearPresetsButton.setBounds(menuBarArea.removeFromRight(100));
    debugClearCacheButton.setBounds(menuBarArea.removeFromRight(100));
    debugLoadButton.setBounds(menuBarArea.removeFromRight(100));
    debugSaveButton.setBounds(menuBarArea.removeFromRight(100));
#endif

    // Remaining area: Split between Gear Library Tree (left) and Tabs (right)
    // Gear Library Tree takes 1/3 of the width, Tabs take 2/3
    int treeWidth = area.getWidth() / 3;

    // Left side: Gear Library Tree
    auto treeArea = area.removeFromLeft(treeWidth);
    gearLibraryTree->setBounds(treeArea);

    // Debug: Log the bounds being set

    // Right side: Tabs containing Rack and Notes
    mainTabs.setBounds(area);

    juce::Logger::writeToLog("AnalogIQEditor::resized - END - All components resized successfully");
}

void AnalogIQEditor::showPresetMenu()
{
    juce::PopupMenu menu;

    // Add "Save Preset..." option with direct callback
    menu.addItem("Save Preset...", [this]()
                 { showSavePresetDialog(); });

    // Add separator
    menu.addSeparator();

    // Add "Load Preset..." option with direct callback
    menu.addItem("Load Preset...", [this]()
                 { showLoadPresetDialog(); });

    // Add preset list if any exist
    auto presetNames = presetManager->getPresetNames();

    if (presetNames.size() > 0)
    {
        // Add separator
        menu.addSeparator();

        // Add "Delete Preset..." option with direct callback
        menu.addItem("Delete Preset...", [this]()
                     { showDeletePresetDialog(); });

        // Add separator
        menu.addSeparator();

        // Add individual presets for quick loading with direct callbacks
        for (int i = 0; i < presetNames.size(); ++i)
        {
            juce::String presetName = presetNames[i];
            menu.addItem(presetName, [this, presetName]()
                         { handleLoadPreset(presetName); });
        }
    }
    else
    {
        menu.addSeparator();
        menu.addItem(999, "No presets available", false, false);
    }

    // Show the menu with proper positioning
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetsMenuButton));
}

void AnalogIQEditor::showSavePresetDialog()
{
    auto *dialog = new juce::AlertWindow("Save Preset",
                                         "Enter a name for the new preset:",
                                         juce::AlertWindow::NoIcon);

    dialog->addTextEditor("presetName", "", "Preset Name:");
    dialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    // Get the text editor for validation
    juce::TextEditor *nameEditor = dialog->getTextEditor("presetName");
    if (nameEditor != nullptr)
    {
        // Add validation on text change
        nameEditor->onTextChange = [this, dialog, nameEditor]()
        {
            juce::String presetName = nameEditor->getText().trim();

            // Basic validation - just check if name is not empty
            if (presetName.isEmpty())
            {
                dialog->setMessage("Enter a name for the new preset:\n\nPreset name cannot be empty.");
                dialog->getButton(1)->setEnabled(false); // Disable Save button
            }
            else
            {
                dialog->setMessage("Enter a name for the new preset:");
                dialog->getButton(1)->setEnabled(true); // Enable Save button
            }
        };
    }

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog](int result)
                                                                      {
        if (result == 1)
        {
            juce::String presetName = dialog->getTextEditorContents("presetName").trim();
            if (presetName.isNotEmpty())
            {
                handleSavePreset(presetName);
            }
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::showLoadPresetDialog()
{
    auto presetNames = presetManager->getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    showPresetSelectionDialog("Load Preset", "Select a preset to load:", "Load", presetNames, [this](const juce::String &presetName)
                              {
        if (!presetName.isEmpty())
        {
            handleLoadPreset(presetName);
        } });
}

void AnalogIQEditor::showDeletePresetDialog()
{
    auto presetNames = presetManager->getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    showPresetSelectionDialog("Delete Preset", "Select a preset to delete:", "Delete", presetNames, [this](const juce::String &presetName)
                              {
        if (!presetName.isEmpty())
        {
            // Show confirmation dialog before deleting
            auto *confirmDialog = new juce::AlertWindow("Confirm Delete",
                                                       "Are you sure you want to delete the preset '" + presetName + "'?",
                                                       juce::AlertWindow::QuestionIcon);
            
            confirmDialog->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
            confirmDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
            
            confirmDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, presetName, confirmDialog](int result)
            {
                if (result == 1) // Delete
                {
                    handleDeletePreset(presetName);
                }
                delete confirmDialog;
            }), true);
        } });
}

void AnalogIQEditor::handleSavePreset(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return;

    // Get the rack component
    if (!rack)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Save Preset Failed",
                                               "Rack component is not available.");
        return;
    }

    // Serialize the rack state to JSON
    juce::String rackStateJSON = processor.getRackModel()->serializeToJSON();

    if (rackStateJSON.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Save Preset Failed",
                                               "Failed to serialize rack state.");
        return;
    }

    // Save the preset using the preset manager
    if (presetManager && presetManager->savePreset(presetName, rackStateJSON))
    {
        currentPresetName = presetName;
        clearModifiedState();

        // Notify the rack that a preset was saved
        processor.getRackModel()->notifyPresetSaved(presetName);

        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Preset Saved",
                                               "Preset '" + presetName + "' has been saved successfully.");
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Save Preset Failed",
                                               "Failed to save preset '" + presetName + "'.");
    }
}

void AnalogIQEditor::handleLoadPreset(const juce::String &presetName)
{
    // Check for unsaved changes
    if (hasUnsavedChanges())
    {
        // Simple confirmation dialog
        auto *dialog = new juce::AlertWindow("Unsaved Changes",
                                             "You have unsaved changes. Do you want to continue without saving?",
                                             juce::AlertWindow::QuestionIcon);

        dialog->addButton("Continue", 1, juce::KeyPress(juce::KeyPress::returnKey));
        dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog, presetName](int result)
                                                                          {
            if (result == 1) // Continue
            {
                performLoadPreset(presetName);
            }
            delete dialog; }),
                                true);
    }
    else
    {
        performLoadPreset(presetName);
    }
}

void AnalogIQEditor::performLoadPreset(const juce::String &presetName)
{
    if (presetName.isEmpty())
        return;

    // Get the rack component
    if (!rack)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Load Preset Failed",
                                               "Rack component is not available.");
        return;
    }

    // Load the preset using the preset manager
    juce::String rackStateJSON;
    if (presetManager && presetManager->loadPreset(presetName, rackStateJSON))
    {
        // Deserialize the rack state from JSON
        if (processor.getRackModel()->deserializeFromJSON(rackStateJSON))
        {
            currentPresetName = presetName;
            clearModifiedState();

            // Notify the rack that a preset was loaded
            processor.getRackModel()->notifyPresetLoaded(presetName);

            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                                   "Preset Loaded",
                                                   "Preset '" + presetName + "' has been loaded successfully.");
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                   "Load Preset Failed",
                                                   "Failed to deserialize rack state from preset '" + presetName + "'.");
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Load Preset Failed",
                                               "Failed to load preset '" + presetName + "'.");
    }
}

void AnalogIQEditor::handleDeletePreset(const juce::String &presetName)
{
    // Delete the preset using the preset manager
    if (presetManager && presetManager->deletePreset(presetName))
    {
        // Clear current preset if it was the one deleted
        if (currentPresetName == presetName)
        {
            currentPresetName = "";
            clearModifiedState();
        }

        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Preset Deleted",
                                               "Preset '" + presetName + "' has been deleted successfully.");
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Delete Preset Failed",
                                               "Failed to delete preset '" + presetName + "'.");
    }
}

void AnalogIQEditor::refreshPresetMenu()
{
    // The preset menu is refreshed dynamically when shown
    // No need to store any state
}

bool AnalogIQEditor::hasUnsavedChanges() const
{
    return isModified;
}

void AnalogIQEditor::markAsModified()
{
    isModified = true;
}

void AnalogIQEditor::clearModifiedState()
{
    isModified = false;
}

void AnalogIQEditor::showPresetSelectionDialog(const juce::String &title,
                                               const juce::String &message,
                                               const juce::String &actionButtonText,
                                               const juce::StringArray &presetNames,
                                               std::function<void(const juce::String &)> callback)
{
    // Create an AlertWindow with custom content
    auto *dialog = new juce::AlertWindow(title, message, juce::AlertWindow::NoIcon);

    auto *content = new PresetSelectionComponent(presetNames, [callback, dialog](const juce::String &selectedPreset)
                                                 {
        callback(selectedPreset);
        dialog->exitModalState(0); }, actionButtonText);

    // Set a proper size for the component
    content->setSize(400, 300);

    // Add the custom component to the AlertWindow
    dialog->addCustomComponent(content);

    // Add the action and cancel buttons
    dialog->addButton(actionButtonText, 1);
    dialog->addButton("Cancel", 0);

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog, actionButtonText, callback](int result)
                                                                      {
        if (result == 1) // Action button clicked
        {
            // Get the selected preset from the component
            auto *content = dynamic_cast<PresetSelectionComponent*>(dialog->getCustomComponent(0));
            if (content)
            {
                int selectedRow = content->getSelectedRow();
                if (selectedRow >= 0)
                {
                    auto presetNames = content->getPresetNames();
                    if (selectedRow < presetNames.size())
                    {
                        callback(presetNames[selectedRow]);
                    }
                }
            }
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::notifyDestruction()
{
    // TODO: Implement when processor has clearRackReference method
    // processor.clearRackReference();
}

#ifdef JUCE_DEBUG
void AnalogIQEditor::clearAllCache()
{
    if (cacheManager)
    {
        cacheManager->clearCache();

        // Show confirmation dialog
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Cache Cleared",
                                               "All cache data has been cleared.\n\nThis simulates a fresh install.",
                                               "OK");
    }
}

void AnalogIQEditor::clearPresetCache()
{
    if (!fileSystem)
    {
        return;
    }

    // Clear preset metadata files
    juce::String cacheRoot = fileSystem->getCacheRootDirectory();
    juce::String presetsDir = fileSystem->joinPath(cacheRoot, "Presets");

    if (fileSystem->directoryExists(presetsDir))
    {
        // Remove preset metadata files
        juce::StringArray presetFiles = fileSystem->getFiles(presetsDir);
        int deletedCount = 0;

        for (auto &file : presetFiles)
        {
            juce::String filePath = fileSystem->joinPath(presetsDir, file);
            if (fileSystem->deleteFile(filePath))
            {
                deletedCount++;
            }
            else
            {
            }
        }
    }
    else
    {
    }

    // Reset UI state
    currentPresetName = "";
    clearModifiedState();

    // Show confirmation dialog
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                           "Presets Cleared",
                                           "All preset files have been cleared from disk.\n\n"
                                           "The plugin will now behave as if no presets were ever saved.",
                                           "OK");
}

void AnalogIQEditor::simulateFreshInstall()
{
    if (!fileSystem)
    {
        return;
    }

    // 1. Clear all asset cache
    if (cacheManager)
    {
        cacheManager->clearCache();
    }

    // 2. Clear user preference files
    juce::String cacheRoot = fileSystem->getCacheRootDirectory();

    // Remove favorites.json
    juce::String favoritesPath = fileSystem->joinPath(cacheRoot, "favorites.json");
    if (fileSystem->fileExists(favoritesPath))
    {
        fileSystem->deleteFile(favoritesPath);
    }

    // Remove recently_used.json
    juce::String recentlyUsedPath = fileSystem->joinPath(cacheRoot, "recently_used.json");
    if (fileSystem->fileExists(recentlyUsedPath))
    {
        fileSystem->deleteFile(recentlyUsedPath);
    }

    // 3. Clear gear library metadata files
    juce::String gearLibraryDir = fileSystem->joinPath(cacheRoot, "GearLibrary");
    if (fileSystem->directoryExists(gearLibraryDir))
    {
        // Remove gear library metadata files
        juce::StringArray gearFiles = fileSystem->getFiles(gearLibraryDir);
        for (auto &file : gearFiles)
        {
            juce::String filePath = fileSystem->joinPath(gearLibraryDir, file);
            fileSystem->deleteFile(filePath);
        }
    }

    // 4. Clear preset metadata files
    juce::String presetsDir = fileSystem->joinPath(cacheRoot, "Presets");
    if (fileSystem->directoryExists(presetsDir))
    {
        // Remove preset metadata files
        juce::StringArray presetFiles = fileSystem->getFiles(presetsDir);
        for (auto &file : presetFiles)
        {
            juce::String filePath = fileSystem->joinPath(presetsDir, file);
            fileSystem->deleteFile(filePath);
        }
    }

    // 5. Clear gear library in memory and reset initialization flag
    if (gearLibrary)
    {
        gearLibrary->clearAllGearItems();
        // Reset the initialized flag to prevent automatic re-population
        gearLibrary->resetInitialization();
    }

    // 6. Reset UI state
    currentPresetName = "";
    clearModifiedState();

    // 7. Refresh the gear library tree
    if (gearLibraryTree)
    {
        gearLibraryTree->repaint();
    }

    // Show confirmation dialog
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                           "Fresh Install Simulated",
                                           "Plugin has been reset to fresh install state.\n\n"
                                           "All cache, user preferences, gear items, and presets have been cleared.\n\n"
                                           "The plugin will now behave as if it was just installed for the first time.",
                                           "OK");
}
#endif

// PresetSelectionComponent Implementation
PresetSelectionComponent::PresetSelectionComponent(const juce::StringArray &names, PresetSelectedCallback cb, const juce::String &actionText)
    : presetNames(names), callback(cb), actionButtonText(actionText)
{
    // Set up the list box
    listBox.setModel(this);
    listBox.setRowHeight(25);
    listBox.setMultipleSelectionEnabled(false);
    addAndMakeVisible(listBox);

    // Buttons are handled by AlertWindow, so we don't need internal buttons

    // Selection changes are handled in listBoxItemClicked
}

void PresetSelectionComponent::paint(juce::Graphics &g)
{
    // Draw background
    g.fillAll(juce::Colours::white);

    // Draw border
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 1);
}

void PresetSelectionComponent::resized()
{
    auto bounds = getLocalBounds();

    // List box takes the full area since AlertWindow handles buttons
    bounds.reduce(5, 5);
    listBox.setBounds(bounds);
}

int PresetSelectionComponent::getNumRows()
{
    return presetNames.size();
}

void PresetSelectionComponent::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= 0 && rowNumber < presetNames.size())
    {
        if (rowIsSelected)
        {
            g.fillAll(juce::Colours::lightblue);
        }
        else
        {
            g.fillAll(juce::Colours::white);
        }

        g.setColour(juce::Colours::black);
        g.setFont(14.0f);
        g.drawText(presetNames[rowNumber], 10, 0, width - 20, height, juce::Justification::left);
    }
}

void PresetSelectionComponent::listBoxItemClicked(int row, const juce::MouseEvent &e)
{
    // Selection is handled by the ListBox itself
    // AlertWindow will handle button enabling/disabling
}

void PresetSelectionComponent::listBoxItemDoubleClicked(int row, const juce::MouseEvent &e)
{
    if (row >= 0 && row < presetNames.size())
    {
        callback(presetNames[row]);
    }
}

int PresetSelectionComponent::getSelectedRow() const
{
    return listBox.getSelectedRow();
}

const juce::StringArray &PresetSelectionComponent::getPresetNames() const
{
    return presetNames;
}
