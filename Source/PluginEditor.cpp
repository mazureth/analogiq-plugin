/**
 * @file PluginEditor.cpp
 * @brief Implementation of the AnalogIQEditor class, providing the main user interface for the AnalogIQ plugin.
 *
 * This file implements the main editor component that:
 * - Initializes and manages the plugin's UI components
 * - Sets up the tabbed interface for rack and notes
 * - Configures the gear library and rack components
 * - Handles layout and painting of the main window
 * - Manages drag and drop functionality
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

/**
 * @brief Constructs a new AnalogIQEditor instance.
 *
 * Initializes the editor with:
 * - Component IDs for debugging
 * - Main UI components (gear library, rack, notes panel)
 * - Tabbed interface configuration
 * - Window size and layout
 * - Drag and drop setup
 *
 * @param p Reference to the audio processor
 */
AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Set component IDs for debugging
    setComponentID("AnalogIQEditor");

    // Create our components
    gearLibrary = std::make_unique<GearLibrary>();
    rack = std::make_unique<Rack>();
    notesPanel = std::make_unique<NotesPanel>();

    // Set component IDs
    gearLibrary->setComponentID("GearLibrary");
    rack->setComponentID("Rack");
    notesPanel->setComponentID("NotesPanel");

    // Connect the Rack to the GearLibrary for drag and drop
    rack->setGearLibrary(gearLibrary.get());

    // Set up main window size
    setSize(1200, 800);

    // Set up tabs
    mainTabs.setComponentID("MainTabs");
    mainTabs.addTab("Rack", juce::Colours::darkgrey, rack.get(), false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, notesPanel.get(), false);
    mainTabs.setTabBarDepth(30);
    mainTabs.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(mainTabs);

    // Add gear library to left side
    addAndMakeVisible(*gearLibrary);

    // Start loading the gear library
    gearLibrary->loadLibraryAsync();

    // Configure drag and drop
    // This is critical - make sure this component is configured as the DragAndDropContainer
    setInterceptsMouseClicks(false, true);

    // Debug info
    DBG("AnalogIQEditor constructed as DragAndDropContainer. Components set up.");
}

/**
 * @brief Destructor for AnalogIQEditor.
 *
 * The unique_ptrs will automatically clean up the components.
 */
AnalogIQEditor::~AnalogIQEditor()
{
    // The unique_ptrs will clean up automatically
}

/**
 * @brief Paints the editor background.
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
 * @brief Called when the component is resized.
 *
 * Updates the layout of all child components:
 * - Gear library takes up 1/4 of the width on the left
 * - Remaining space is used for the tabbed interface containing rack and notes
 *
 * @param area The new bounds of the component
 */
void AnalogIQEditor::resized()
{
    auto area = getLocalBounds();

    // Left side: Gear library (1/4 of the width)
    auto libraryArea = area.removeFromLeft(area.getWidth() / 4);
    gearLibrary->setBounds(libraryArea);

    // Remaining area: Tabs containing Rack and Notes
    mainTabs.setBounds(area);
}