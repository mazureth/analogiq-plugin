#include "NotesPanel.h"

NotesPanel::NotesPanel()
{
    // Set up title
    titleLabel.setText("Session Notes", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Set up text editor
    textEditor.setMultiLine(true);
    textEditor.setReturnKeyStartsNewLine(true);
    textEditor.setReadOnly(false);
    textEditor.setScrollbarsShown(true);
    textEditor.setCaretVisible(true);
    textEditor.setPopupMenuEnabled(true);
    textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    textEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
    textEditor.setText("Enter your session notes here. Document patchbay connections, settings, and any other important details.");
    addAndMakeVisible(textEditor);
}

NotesPanel::~NotesPanel()
{
}

void NotesPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
}

void NotesPanel::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));
    
    // Text editor fills the rest
    textEditor.setBounds(bounds);
}

void NotesPanel::setText(const juce::String& text)
{
    textEditor.setText(text);
}

juce::String NotesPanel::getText() const
{
    return textEditor.getText();
} 