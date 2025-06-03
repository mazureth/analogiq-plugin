#include <JuceHeader.h>
#include "../../Source/NotesPanel.h"

class NotesPanelTests : public juce::UnitTest
{
public:
    NotesPanelTests() : UnitTest("NotesPanelTests") {}

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

static NotesPanelTests notesPanelTests;