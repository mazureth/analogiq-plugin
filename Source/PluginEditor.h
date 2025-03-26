#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GearLibrary.h"
#include "Rack.h"
#include "NotesPanel.h"

class AnalogIQEditor : public juce::AudioProcessorEditor,
                       public juce::DragAndDropContainer
{
public:
    AnalogIQEditor(AnalogIQProcessor&);
    ~AnalogIQEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AnalogIQProcessor& processor;

    // UI Components
    juce::TabbedComponent mainTabs;
    GearLibrary gearLibrary;
    Rack rackGrid;
    NotesPanel notesPanel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQEditor)
}; 