/**
 * @file PluginEditor.cpp
 * @brief Implementation of the AnalogIQEditor class.
 *
 * This file implements the main editor interface for the AnalogIQ plugin,
 * providing a user interface for managing audio gear, racks, and session notes.
 * It includes a gear library, rack interface, and notes panel organized in a
 * tabbed layout.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetManager.h"

/**
 * @brief Constructs a new AnalogIQEditor.
 *
 * Initializes the editor with a gear library, rack, and notes panel.
 * Sets up the tabbed interface and configures drag and drop functionality.
 *
 * @param p Reference to the associated AudioProcessor
 */
AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Set component IDs for debugging
    setComponentID("AnalogIQEditor");

    // Create our components
    gearLibrary = std::make_unique<GearLibrary>(audioProcessor.getNetworkFetcher());
    rack = std::make_unique<Rack>(audioProcessor.getNetworkFetcher());
    notesPanel = std::make_unique<NotesPanel>();

    // Set component IDs
    gearLibrary->setComponentID("GearLibrary");
    rack->setComponentID("RackTab");
    notesPanel->setComponentID("NotesTab");

    // Connect the Rack to the GearLibrary for drag and drop
    rack->setGearLibrary(gearLibrary.get());

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
    addAndMakeVisible(*gearLibrary);

    // Start loading the gear library
    gearLibrary->loadLibraryAsync();

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

    // Left side: Gear library (1/4 of the remaining width)
    auto libraryArea = area.removeFromLeft(area.getWidth() / 4);
    gearLibrary->setBounds(libraryArea);

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
    auto &presetManager = PresetManager::getInstance();
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
            juce::String displayName = presetManager.getPresetDisplayName(presetNames[i]);
            menu.addItem(displayName, [this, i, presetNames]()
                         { 
                juce::String presetName = presetNames[i];
                auto &presetManager = PresetManager::getInstance();
                if (presetManager.loadPreset(presetName, rack.get(), gearLibrary.get()))
                {
                    juce::Logger::writeToLog("Preset loaded: " + presetName);
                }
                else
                {
                    juce::Logger::writeToLog("Failed to load preset: " + presetName);
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                          "Preset Load Error",
                                                          "Failed to load preset: " + presetName);
                } });
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
    juce::AlertWindow dialog("Save Preset",
                             "Enter a name for the new preset:",
                             juce::AlertWindow::QuestionIcon);

    dialog.addTextEditor("presetName", "", "Preset Name:");
    dialog.addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog.enterModalState(true, juce::ModalCallbackFunction::create([this, &dialog](int result)
                                                                     {
        if (result == 1)
        {
            juce::String presetName = dialog.getTextEditorContents("presetName").trim();
            if (presetName.isNotEmpty())
            {
                auto &presetManager = PresetManager::getInstance();
                if (presetManager.savePreset(presetName, rack.get()))
                {
                    juce::Logger::writeToLog("Preset saved: " + presetName);
                }
                else
                {
                    juce::Logger::writeToLog("Failed to save preset: " + presetName);
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                          "Preset Save Error",
                                                          "Failed to save preset: " + presetName);
                }
            }
        } }),
                           true);
}

void AnalogIQEditor::showLoadPresetDialog()
{
    auto &presetManager = PresetManager::getInstance();
    auto presetNames = presetManager.getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    juce::AlertWindow dialog("Load Preset",
                             "Select a preset to load:",
                             juce::AlertWindow::QuestionIcon);

    // Add dropdown for preset selection
    dialog.addComboBox("presetSelect", juce::String("Preset:"));
    juce::ComboBox *presetCombo = dialog.getComboBoxComponent("presetSelect");

    if (presetCombo != nullptr)
    {
        for (int i = 0; i < presetNames.size(); ++i)
        {
            juce::String displayName = presetManager.getPresetDisplayName(presetNames[i]);
            presetCombo->addItem(displayName, i + 1);
        }
        presetCombo->setSelectedId(1, juce::dontSendNotification);
    }

    dialog.addButton("Load", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog.enterModalState(true, juce::ModalCallbackFunction::create([this, &dialog](int result)
                                                                     {
        if (result == 1)
        {
            juce::ComboBox *presetCombo = dialog.getComboBoxComponent("presetSelect");
            if (presetCombo != nullptr)
            {
                int selectedId = presetCombo->getSelectedId();
                if (selectedId > 0)
                {
                    auto &presetManager = PresetManager::getInstance();
                    auto presetNames = presetManager.getPresetNames();
                    juce::String presetName = presetNames[selectedId - 1];
                    
                    if (presetManager.loadPreset(presetName, rack.get(), gearLibrary.get()))
                    {
                        juce::Logger::writeToLog("Preset loaded: " + presetName);
                    }
                    else
                    {
                        juce::Logger::writeToLog("Failed to load preset: " + presetName);
                        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                              "Preset Load Error",
                                                              "Failed to load preset: " + presetName);
                    }
                }
            }
        } }),
                           true);
}

void AnalogIQEditor::showDeletePresetDialog()
{
    auto &presetManager = PresetManager::getInstance();
    auto presetNames = presetManager.getPresetNames();

    if (presetNames.size() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "No Presets Available",
                                               "No presets have been saved yet.");
        return;
    }

    juce::AlertWindow dialog("Delete Preset",
                             "Select a preset to delete:",
                             juce::AlertWindow::WarningIcon);

    // Add dropdown for preset selection
    dialog.addComboBox("presetSelect", juce::String("Preset:"));
    juce::ComboBox *presetCombo = dialog.getComboBoxComponent("presetSelect");

    if (presetCombo != nullptr)
    {
        for (int i = 0; i < presetNames.size(); ++i)
        {
            juce::String displayName = presetManager.getPresetDisplayName(presetNames[i]);
            presetCombo->addItem(displayName, i + 1);
        }
        presetCombo->setSelectedId(1, juce::dontSendNotification);
    }

    dialog.addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog.enterModalState(true, juce::ModalCallbackFunction::create([this, &dialog](int result)
                                                                     {
        if (result == 1)
        {
            juce::ComboBox *presetCombo = dialog.getComboBoxComponent("presetSelect");
            if (presetCombo != nullptr)
            {
                int selectedId = presetCombo->getSelectedId();
                if (selectedId > 0)
                {
                    auto &presetManager = PresetManager::getInstance();
                    auto presetNames = presetManager.getPresetNames();
                    juce::String presetName = presetNames[selectedId - 1];
                    
                    // Show confirmation dialog
                    juce::AlertWindow confirmDialog("Confirm Delete",
                                                   "Are you sure you want to delete the preset:\n\"" + presetName + "\"?",
                                                   juce::AlertWindow::WarningIcon);
                    
                    confirmDialog.addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
                    confirmDialog.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                    
                    confirmDialog.enterModalState(true, juce::ModalCallbackFunction::create([presetName](int confirmResult)
                                                                                            {
                        if (confirmResult == 1)
                        {
                            auto &presetManager = PresetManager::getInstance();
                            if (presetManager.deletePreset(presetName))
                            {
                                juce::Logger::writeToLog("Preset deleted: " + presetName);
                            }
                            else
                            {
                                juce::Logger::writeToLog("Failed to delete preset: " + presetName);
                                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                                      "Preset Delete Error",
                                                                      "Failed to delete preset: " + presetName);
                            }
                        } }),
                                                true);
                }
            }
        } }),
                           true);
}