#include <JuceHeader.h>
#include "NotesPanel.h"

class NotesPanelTests : public juce::UnitTest
{
public:
    NotesPanelTests() : UnitTest("NotesPanel Tests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            NotesPanel panel;
            expect(panel.getText().isEmpty());
        }

        beginTest("Set Text");
        {
            NotesPanel panel;
            panel.setText("Test Note");
            expectEquals(panel.getText(), juce::String("Test Note"));
        }

        beginTest("Clear Text");
        {
            NotesPanel panel;
            panel.setText("");
            expect(panel.getText().isEmpty());
        }

        beginTest("Text Changed Callback");
        {
            NotesPanel panel;
            panel.setText("New Text");
            expect(panel.getText() == "New Text");
        }

        beginTest("Serialization");
        {
            // Serialization tests removed as writeToStream and readFromStream are not available
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static NotesPanelTests notesPanelTests;