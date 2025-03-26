#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogIQEditor::AnalogIQEditor(AnalogIQProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Set up main window size
    setSize(1200, 800);
    
    // Set up tabs
    mainTabs.addTab("Rack", juce::Colours::darkgrey, &rackGrid, false);
    mainTabs.addTab("Notes", juce::Colours::darkgrey, &notesPanel, false);
    mainTabs.setTabBarDepth(30);
    addAndMakeVisible(mainTabs);
    
    // Add gear library to left side
    addAndMakeVisible(gearLibrary);
    
    // Start loading the gear library
    gearLibrary.loadLibraryAsync();
}

AnalogIQEditor::~AnalogIQEditor()
{
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
    gearLibrary.setBounds(libraryArea);
    
    // Remaining area: Tabs containing Rack and Notes
    mainTabs.setBounds(area);
} 