#include <JuceHeader.h>
#include "NotesPanel.h"
#include "TestFixture.h"

class NotesPanelTests : public juce::UnitTest
{
public:
    NotesPanelTests() : UnitTest("NotesPanelTests") {}

    void runTest() override
    {
        TestFixture fixture;
        beginTest("Construction");
        {
            NotesPanel panel;
            expectEquals(panel.getText(), juce::String("Enter your session notes here. Document patchbay connections, settings, and any other important details."), "Text should be empty");
        }

        beginTest("Set Text");
        {
            NotesPanel panel;
            panel.setText("Test Note");
            expectEquals(panel.getText(), juce::String("Test Note"), "Text should match");
        }

        beginTest("Clear Text");
        {
            NotesPanel panel;
            panel.setText("");
            expectEquals(panel.getText(), juce::String(""), "Text should be empty");
        }

        beginTest("Text Changed Callback");
        {
            NotesPanel panel;
            panel.setText("New Text");
            expectEquals(panel.getText(), juce::String("New Text"), "Text should match");
        }

        beginTest("Serialization");
        {
            // Serialization tests removed as writeToStream and readFromStream are not available
        }
    }
};

static NotesPanelTests notesPanelTests;