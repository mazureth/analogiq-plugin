/**
 * @file AnalogIQEditor.cpp
 * @brief Implementation of the AnalogIQEditor class.
 *
 * This file implements the main editor interface for the AnalogIQ plugin,
 * providing a user interface for managing audio gear, racks, and session notes.
 * It includes a gear library, rack interface, and notes panel organized in a
 * tabbed layout.
 */

#include "AnalogIQProcessor.h"
#include "AnalogIQEditor.h"
#include "PresetManager.h"

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
                               IFileSystem &fileSystem,
                               CacheManager &cacheManager,
                               PresetManager &presetManager,
                               GearLibrary &gearLibrary)
    : AudioProcessorEditor(&processor),
      processor(processor),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(gearLibrary),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Set component IDs for debugging
    setComponentID("AnalogIQEditor");

    // Create our components
    rack = std::make_unique<Rack>(processor.getNetworkFetcher(), fileSystem, cacheManager, presetManager, &gearLibrary);
    notesPanel = std::make_unique<NotesPanel>();

    // Set component IDs
    gearLibrary.setComponentID("GearLibrary");
    rack->setComponentID("RackTab");
    notesPanel->setComponentID("NotesTab");

    // Set up main window size
    setSize(1200, 800);

    // Set up tabs
    mainTabs.setComponentID("MainTabs");

    // Add tabs with components
    mainTabs.addTab("Rack", juce::Colours::darkgrey, rack.get(), false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, notesPanel.get(), false);

    mainTabs.setTabBarDepth(30);
    mainTabs.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(mainTabs);

    // Add gear library to left side
    addAndMakeVisible(&gearLibrary);

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

#ifdef JUCE_DEBUG
    // Add debug buttons for testing state saving/loading
    debugSaveButton.setButtonText("Debug: Save State");
    debugSaveButton.onClick = [this, &processor]()
    {
        // Simulate the full getStateInformation process
        juce::MemoryBlock destData;
        processor.getStateInformation(destData); // This will call saveInstanceState internally

        // Log the result
        std::cout << "[Debug] State saved, data size: " << destData.getSize() << " bytes" << std::endl;
    };
    addAndMakeVisible(debugSaveButton);

    debugLoadButton.setButtonText("Debug: Load State");
    debugLoadButton.onClick = [this, &processor]()
    {
        if (auto *rack = getRack())
        {
            processor.loadInstanceState(rack);
        }
    };
    addAndMakeVisible(debugLoadButton);
#endif

    // Set up menu bar styling
    menuBarContainer.setOpaque(true);

    // Configure drag and drop
    // This is critical - make sure this component is configured as the DragAndDropContainer
    setInterceptsMouseClicks(false, true);

    // Load the gear library data now that the plugin is ready
    gearLibrary.loadLibrary();
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
AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor &processor, CacheManager &cacheManager, PresetManager &presetManager, bool disableAutoLoad)
    : AudioProcessorEditor(&processor),
      processor(processor),
      fileSystem(processor.getFileSystem()),
      cacheManager(cacheManager),
      presetManager(presetManager),
      gearLibrary(processor.getGearLibrary()),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Set component IDs for debugging
    setComponentID("AnalogIQEditor");

    // Create our components
    rack = std::make_unique<Rack>(processor.getNetworkFetcher(), fileSystem, cacheManager, presetManager, &gearLibrary);
    notesPanel = std::make_unique<NotesPanel>();

    // Set component IDs
    gearLibrary.setComponentID("GearLibrary");
    rack->setComponentID("RackTab");
    notesPanel->setComponentID("NotesTab");

    // Set up main window size
    setSize(1200, 800);

    // Set up tabs
    mainTabs.setComponentID("MainTabs");

    // Add tabs with components
    mainTabs.addTab("Rack", juce::Colours::darkgrey, rack.get(), false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, notesPanel.get(), false);

    mainTabs.setTabBarDepth(30);
    mainTabs.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(mainTabs);

    // Add gear library to left side
    addAndMakeVisible(&gearLibrary);

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
}

/**
 * @brief Destructor for AnalogIQEditor.
 *
 * Cleans up all components through unique_ptr automatic destruction.
 */
AnalogIQEditor::~AnalogIQEditor()
{
    // CRITICAL: Clear LookAndFeel reference before destruction to avoid JUCE assertion
    // This prevents "LookAndFeel object being destroyed while something is still using it"
    presetsMenuButton.setLookAndFeel(nullptr);

    // The unique_ptrs will clean up automatically
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
 * Arranges the menu bar at the top, gear library on the left side (1/4 width),
 * and the tabbed interface (rack and notes) in the remaining space.
 */
void AnalogIQEditor::resized()
{
    auto area = getLocalBounds();

    // Top area: Menu bar (full width)
    auto menuBarArea = area.removeFromTop(30);
    menuBarContainer.setBounds(menuBarArea);

    // Position preset menu button on the left side of the menu bar
    presetsMenuButton.setBounds(menuBarArea.removeFromLeft(80));

#ifdef JUCE_DEBUG
    // Position debug buttons on the right side of the menu bar
    debugSaveButton.setBounds(menuBarArea.removeFromRight(120));
    debugLoadButton.setBounds(menuBarArea.removeFromRight(120));
#endif

    // Left side: Gear library (1/4 of the remaining width)
    auto libraryArea = area.removeFromLeft(area.getWidth() / 4);
    gearLibrary.setBounds(libraryArea);

    // Remaining area: Tabs containing Rack and Notes
    mainTabs.setBounds(area);
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
    auto presetNames = presetManager.getPresetNames();

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
            juce::String displayName = presetManager.getPresetDisplayNameNoTimestamp(presetNames[i]);
            menu.addItem(displayName, [this, i, presetNames]()
                         { 
                juce::String presetName = presetNames[i];
                handleLoadPreset(presetName); });
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

            // Validate the name
            juce::String validationError;
            if (!presetManager.validatePresetName(presetName, validationError))
            {
                // Show validation error in dialog
                dialog->setMessage("Enter a name for the new preset:\n\n" + validationError);
                dialog->getButton(1)->setEnabled(false); // Disable Save button
            }
            else
            {
                // Check for name conflicts
                juce::String conflictError;
                if (presetManager.checkPresetNameConflict(presetName, conflictError))
                {
                    dialog->setMessage("Enter a name for the new preset:\n\n" + conflictError);
                    dialog->getButton(1)->setEnabled(false); // Disable Save button
                }
                else
                {
                    dialog->setMessage("Enter a name for the new preset:");
                    dialog->getButton(1)->setEnabled(true); // Enable Save button
                }
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
                // Final validation before saving
                juce::String validationError;
                if (!presetManager.validatePresetName(presetName, validationError))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                           "Invalid Preset Name",
                                                           validationError);
                }
                else
                {
                    juce::String conflictError;
                    if (presetManager.checkPresetNameConflict(presetName, conflictError))
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                               "Preset Name Conflict",
                                                               conflictError);
                    }
                    else
                    {
                        handleSavePreset(presetName);
                    }
                }
            }
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::showLoadPresetDialog()
{
    auto presetNames = presetManager.getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    auto *dialog = new juce::AlertWindow("Load Preset",
                                         "Select a preset to load:",
                                         juce::AlertWindow::NoIcon);

    // Create StringArray with preset display names
    juce::StringArray presetDisplayNames;
    for (int i = 0; i < presetNames.size(); ++i)
    {
        juce::String displayName = presetManager.getPresetDisplayName(presetNames[i]);
        presetDisplayNames.add(displayName);
    }

    // Add dropdown for preset selection with pre-populated list
    dialog->addComboBox("presetSelect", presetDisplayNames);
    juce::ComboBox *presetCombo = dialog->getComboBoxComponent("presetSelect");

    if (presetCombo != nullptr)
    {
        presetCombo->setSelectedItemIndex(0, juce::dontSendNotification);
    }

    dialog->addButton("Load", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog](int result)
                                                                      {
        if (result == 1)
        {
            juce::ComboBox *presetCombo = dialog->getComboBoxComponent("presetSelect");
            if (presetCombo != nullptr)
            {
                int selectedIndex = presetCombo->getSelectedItemIndex();
                if (selectedIndex >= 0)
                {
                    auto presetNames = presetManager.getPresetNames();
                    juce::String presetName = presetNames[selectedIndex];
                    
                    handleLoadPreset(presetName);
                }
            }
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::showDeletePresetDialog()
{
    auto presetNames = presetManager.getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    auto *dialog = new juce::AlertWindow("Delete Preset",
                                         "Select a preset to delete:",
                                         juce::AlertWindow::NoIcon);

    // Create StringArray with preset display names
    juce::StringArray presetDisplayNames;
    for (int i = 0; i < presetNames.size(); ++i)
    {
        juce::String displayName = presetManager.getPresetDisplayName(presetNames[i]);
        presetDisplayNames.add(displayName);
    }

    // Add dropdown for preset selection with pre-populated list
    dialog->addComboBox("presetSelect", presetDisplayNames);
    juce::ComboBox *presetCombo = dialog->getComboBoxComponent("presetSelect");

    if (presetCombo != nullptr)
    {
        presetCombo->setSelectedItemIndex(0, juce::dontSendNotification);
    }

    dialog->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog](int result)
                                                                      {
        if (result == 1)
        {
            juce::ComboBox *presetCombo = dialog->getComboBoxComponent("presetSelect");
            if (presetCombo != nullptr)
            {
                int selectedIndex = presetCombo->getSelectedItemIndex();
                if (selectedIndex >= 0)
                {
                    auto presetNames = presetManager.getPresetNames();
                    juce::String presetName = presetNames[selectedIndex];
                    
                    // Show confirmation dialog
                    auto* confirmDialog = new juce::AlertWindow("Confirm Delete",
                                                               "Are you sure you want to delete the preset:\n\"" + presetName + "\"?",
                                                               juce::AlertWindow::WarningIcon);
                    
                    confirmDialog->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
                    confirmDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                    
                    confirmDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, presetName, confirmDialog](int confirmResult)
                                                                                                {
                        if (confirmResult == 1)
                        {
                            handleDeletePreset(presetName);
                        }
                        delete confirmDialog;
                    }), true);
                }
            }
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::handleSavePreset(const juce::String &presetName)
{
    if (presetManager.savePreset(presetName, rack.get()))
    {
        currentPresetName = presetName;
        clearModifiedState();

        // Show success message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Preset Saved",
                                               "Preset '" + presetName + "' saved successfully.");
    }
    else
    {
        juce::String errorMessage = presetManager.getLastErrorMessage();

        // Show detailed error message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Preset Save Error",
                                               "Failed to save preset '" + presetName + "'.\n\nError: " + errorMessage);
    }
}

void AnalogIQEditor::handleLoadPreset(const juce::String &presetName)
{
    // Check if the rack has any gear items
    bool hasGearItems = false;
    for (int i = 0; i < rack->getNumSlots(); ++i)
    {
        if (auto *slot = rack->getSlot(i))
        {
            if (slot->getGearItem() != nullptr)
            {
                hasGearItems = true;
                break;
            }
        }
    }

    // If rack has gear items, show confirmation dialog
    if (hasGearItems)
    {
        auto *confirmDialog = new juce::AlertWindow("Confirm Load Preset",
                                                    "Loading the preset '" + presetName + "' will replace all current gear items in the rack.\n\nDo you want to continue?",
                                                    juce::AlertWindow::WarningIcon);

        confirmDialog->addButton("Load Preset", 1, juce::KeyPress(juce::KeyPress::returnKey));
        confirmDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        confirmDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, presetName, confirmDialog](int result)
                                                                                 {
            if (result == 1)
            {
                // User confirmed, proceed with loading
                performLoadPreset(presetName);
            }
            delete confirmDialog; }),
                                       true);
    }
    else
    {
        // Rack is empty, load directly
        performLoadPreset(presetName);
    }
}

void AnalogIQEditor::handleDeletePreset(const juce::String &presetName)
{
    if (presetManager.deletePreset(presetName))
    {
        // If this was the currently loaded preset, clear the current preset name
        if (currentPresetName == presetName)
        {
            currentPresetName = "";
        }

        // Show success message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Preset Deleted",
                                               "Preset '" + presetName + "' deleted successfully.");
    }
    else
    {
        juce::String errorMessage = presetManager.getLastErrorMessage();

        // Show detailed error message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Preset Delete Error",
                                               "Failed to delete preset '" + presetName + "'.\n\nError: " + errorMessage);
    }
}

void AnalogIQEditor::refreshPresetMenu()
{
    // The menu is rebuilt each time showPresetMenu() is called,
    // so this method can be used to trigger a menu refresh if needed
    // For now, we'll just log that it was called
}

bool AnalogIQEditor::hasUnsavedChanges() const
{
    return isModified;
}

void AnalogIQEditor::markAsModified()
{
    if (!isModified)
    {
        isModified = true;
    }
}

void AnalogIQEditor::clearModifiedState()
{
    if (isModified)
    {
        isModified = false;
    }
}

void AnalogIQEditor::performLoadPreset(const juce::String &presetName)
{
    if (presetManager.loadPreset(presetName, rack.get(), &gearLibrary))
    {
        currentPresetName = presetName;
        clearModifiedState();

        // Show success message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Preset Loaded",
                                               "Preset '" + presetName + "' loaded successfully.");
    }
    else
    {
        juce::String errorMessage = presetManager.getLastErrorMessage();

        // Show detailed error message
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Preset Load Error",
                                               "Failed to load preset '" + presetName + "'.\n\nError: " + errorMessage);
    }
}