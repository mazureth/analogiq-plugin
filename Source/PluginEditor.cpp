#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor& p)
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

AnalogIQEditor::~AnalogIQEditor()
{
    // The unique_ptrs will clean up automatically
}

void AnalogIQEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AnalogIQEditor::resized()
{
    auto area = getLocalBounds();
    
    // Left side: Gear library (1/4 of the width)
    auto libraryArea = area.removeFromLeft(area.getWidth() / 4);
    gearLibrary->setBounds(libraryArea);
    
    // Remaining area: Tabs containing Rack and Notes
    mainTabs.setBounds(area);
} 