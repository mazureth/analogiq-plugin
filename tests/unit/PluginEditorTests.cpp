#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class PluginEditorTests : public juce::UnitTest
{
public:
    PluginEditorTests() : UnitTest("PluginEditor Tests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            AnalogIQProcessor processor;
            AnalogIQEditor editor(processor);

            expect(editor.getAudioProcessor() == &processor);
        }

        beginTest("Component Hierarchy");
        {
            AnalogIQProcessor processor;
            AnalogIQEditor editor(processor);

            // Check that main components exist
            expect(editor.findChildWithID("rack") != nullptr);
            expect(editor.findChildWithID("library") != nullptr);
            expect(editor.findChildWithID("notes") != nullptr);
        }

        beginTest("Resize Handling");
        {
            AnalogIQProcessor processor;
            AnalogIQEditor editor(processor);

            // Test minimum size
            editor.setSize(800, 600);
            expect(editor.getWidth() >= 800);
            expect(editor.getHeight() >= 600);

            // Test resize
            editor.setSize(1024, 768);
            expect(editor.getWidth() == 1024);
            expect(editor.getHeight() == 768);
        }

        beginTest("Parameter Updates");
        {
            AnalogIQProcessor processor;
            AnalogIQEditor editor(processor);

            // Test parameter change notification
            bool parameterChanged = false;
            // Simulate parameter change
            if (auto *param = processor.getParameters().getFirst())
            {
                param->setValue(0.5f);
                expect(parameterChanged);
            }
        }

        beginTest("Component Visibility");
        {
            AnalogIQProcessor processor;
            AnalogIQEditor editor(processor);

            // Check that all main components are visible
            auto *rack = editor.findChildWithID("rack");
            auto *library = editor.findChildWithID("library");
            auto *notes = editor.findChildWithID("notes");

            expect(rack != nullptr && rack->isVisible());
            expect(library != nullptr && library->isVisible());
            expect(notes != nullptr && notes->isVisible());
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static PluginEditorTests pluginEditorTests;