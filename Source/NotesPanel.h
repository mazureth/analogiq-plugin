#pragma once

#include <JuceHeader.h>

class NotesPanel : public juce::Component
{
public:
    NotesPanel();
    ~NotesPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void setText(const juce::String& text);
    juce::String getText() const;

private:
    juce::TextEditor textEditor;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesPanel)
}; 