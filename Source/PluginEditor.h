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
    AnalogIQEditor(AnalogIQProcessor &);
    ~AnalogIQEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    // Add getter for rack
    Rack *getRack() const { return rack.get(); }

private:
    // Store processor reference - keep it even though it's currently unused
    // as we'll need it later for state management
    AnalogIQProcessor &audioProcessor;

    // UI Components
    juce::TabbedComponent mainTabs{juce::TabbedButtonBar::TabsAtTop};
    std::unique_ptr<GearLibrary> gearLibrary;
    std::unique_ptr<Rack> rack;
    std::unique_ptr<NotesPanel> notesPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQEditor)
};