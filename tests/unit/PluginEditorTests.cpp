#include <JuceHeader.h>
#include "../../Source/PluginEditor.h"
#include "PluginProcessor.h"

class PluginEditorTests : public juce::UnitTest
{
public:
    PluginEditorTests() : UnitTest("PluginEditorTests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            // TODO: Create a concrete test processor class
            // AnalogIQProcessor processor;
            // AnalogIQEditor editor(processor);
            // expect(editor.getAudioProcessor() == &processor);
        }

        beginTest("Component Hierarchy");
        {
            // TODO: Create a concrete test processor class
            // AnalogIQProcessor processor;
            // AnalogIQEditor editor(processor);
            // expect(editor.findChildWithID("rack") != nullptr);
            // expect(editor.findChildWithID("library") != nullptr);
            // expect(editor.findChildWithID("notes") != nullptr);
        }

        beginTest("Resize Handling");
        {
            // TODO: Create a concrete test processor class
            // AnalogIQProcessor processor;
            // AnalogIQEditor editor(processor);
            // editor.setSize(800, 600);
            // expect(editor.getWidth() >= 800);
            // expect(editor.getHeight() >= 600);
        }

        beginTest("Parameter Updates");
        {
            // TODO: Create a concrete test processor class
            // AnalogIQProcessor processor;
            // AnalogIQEditor editor(processor);
            // bool parameterChanged = false;
            // if (auto *param = processor.getParameters().getFirst())
            // {
            //     param->setValue(0.5f);
            //     expect(parameterChanged);
            // }
        }

        beginTest("Component Visibility");
        {
            // TODO: Create a concrete test processor class
            // AnalogIQProcessor processor;
            // AnalogIQEditor editor(processor);
            // auto *rack = editor.findChildWithID("rack");
            // auto *library = editor.findChildWithID("library");
            // auto *notes = editor.findChildWithID("notes");
            // expect(rack != nullptr && rack->isVisible());
            // expect(library != nullptr && library->isVisible());
            // expect(notes != nullptr && notes->isVisible());
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static PluginEditorTests pluginEditorTests;