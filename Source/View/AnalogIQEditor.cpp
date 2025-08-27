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
    juce::Logger::writeToLog("AnalogIQEditor: Constructor 2 starting");
    setComponentID("AnalogIQEditor");

    // Create GearLibraryTree component (to the left of the rack)
    juce::Logger::writeToLog("AnalogIQEditor: Creating GearLibraryTree component");
    gearLibraryTree = std::make_unique<GearLibraryTree>(*gearLibrary, *cacheManager, *presetManager);
    juce::Logger::writeToLog("AnalogIQEditor: GearLibraryTree component created successfully");

    // Create Rack component
    juce::Logger::writeToLog("AnalogIQEditor: Creating Rack component");
    rack = std::make_unique<Rack>(*processor.getNetworkFetcher(), *fileSystem, *cacheManager, *presetManager, *gearLibrary);
    juce::Logger::writeToLog("AnalogIQEditor: Rack component created successfully");

    // Create NotesPanel component
    juce::Logger::writeToLog("AnalogIQEditor: Creating NotesPanel component");
    notesPanel = std::make_unique<NotesPanel>();
    juce::Logger::writeToLog("AnalogIQEditor: NotesPanel component created successfully");

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

    // Note: GearLibrary is not a JUCE Component, so it can't be added to the UI
    // It will be managed separately through the Model layer

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
    juce::Logger::writeToLog("AnalogIQEditor: Constructor 1 starting");
    setComponentID("AnalogIQEditor");

    // Create GearLibraryTree component (to the left of the rack)
    juce::Logger::writeToLog("AnalogIQEditor: Creating GearLibraryTree component");
    gearLibraryTree = std::make_unique<GearLibraryTree>(*gearLibrary, *cacheManager, *presetManager);
    juce::Logger::writeToLog("AnalogIQEditor: GearLibraryTree component created successfully");

    // Create Rack component
    juce::Logger::writeToLog("AnalogIQEditor: Creating Rack component");
    rack = std::make_unique<Rack>(*processor.getNetworkFetcher(), *fileSystem, *cacheManager, *presetManager, *gearLibrary);
    juce::Logger::writeToLog("AnalogIQEditor: Rack component created successfully");

    // Create NotesPanel component
    juce::Logger::writeToLog("AnalogIQEditor: Creating NotesPanel component");
    notesPanel = std::make_unique<NotesPanel>();
    juce::Logger::writeToLog("AnalogIQEditor: NotesPanel component created successfully");

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

    // Note: GearLibrary is not a JUCE Component, so it can't be added to the UI
    // It will be managed separately through the Model layer

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
    // Notify the processor that we're being destroyed so it can clear stored references
    // TODO: Implement when processor has clearRackReference method
    // processor.clearRackReference();

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
 * Arranges the menu bar at the top and the tabbed interface (rack and notes) in the remaining space.
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

    // Remaining area: Split between Gear Library Tree (left) and Tabs (right)
    // Gear Library Tree takes 1/3 of the width, Tabs take 2/3
    int treeWidth = area.getWidth() / 3;

    // Left side: Gear Library Tree
    auto treeArea = area.removeFromLeft(treeWidth);
    gearLibraryTree->setBounds(treeArea);

    // Right side: Tabs containing Rack and Notes
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

    auto *dialog = new juce::AlertWindow("Load Preset",
                                         "Select a preset to load:",
                                         juce::AlertWindow::NoIcon);

    // Add preset list
    for (int i = 0; i < presetNames.size(); ++i)
    {
        juce::String presetName = presetNames[i];
        dialog->addButton(presetName, i + 1);
    }

    dialog->addButton("Cancel", 0);

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog, presetNames](int result)
                                                                      {
        if (result > 0 && result <= presetNames.size())
        {
            juce::String presetName = presetNames[result - 1];
            handleLoadPreset(presetName);
        }
        delete dialog; }),
                            true);
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

    auto *dialog = new juce::AlertWindow("Delete Preset",
                                         "Select a preset to delete:",
                                         juce::AlertWindow::NoIcon);

    // Add preset list
    for (int i = 0; i < presetNames.size(); ++i)
    {
        juce::String presetName = presetNames[i];
        dialog->addButton(presetName, i + 1);
    }

    dialog->addButton("Cancel", 0);

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog, presetNames](int result)
                                                                      {
        if (result > 0 && result <= presetNames.size())
        {
            juce::String presetName = presetNames[result - 1];
            handleDeletePreset(presetName);
        }
        delete dialog; }),
                            true);
}

void AnalogIQEditor::handleSavePreset(const juce::String &presetName)
{
    // TODO: Implement when Rack component is available and we have access to AudioProcessorValueTreeState
    // For now, just mark as modified
    currentPresetName = presetName;
    clearModifiedState();
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
    // TODO: Implement when Rack component is available and we have access to AudioProcessorValueTreeState
    // For now, just update the current preset name
    currentPresetName = presetName;
    clearModifiedState();
}

void AnalogIQEditor::handleDeletePreset(const juce::String &presetName)
{
    auto *dialog = new juce::AlertWindow("Delete Preset",
                                         "Are you sure you want to delete the preset '" + presetName + "'?",
                                         juce::AlertWindow::QuestionIcon);

    dialog->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog, presetName](int result)
                                                                      {
        if (result == 1) // Delete
        {
            // TODO: Implement when we have access to AudioProcessorValueTreeState
            // presetManager.deletePreset(presetName);
            
            // Clear current preset if it was the one deleted
            if (currentPresetName == presetName)
            {
                currentPresetName = "";
                clearModifiedState();
            }
        }
        delete dialog; }),
                            true);
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

void AnalogIQEditor::notifyDestruction()
{
    // TODO: Implement when processor has clearRackReference method
    // processor.clearRackReference();
}
