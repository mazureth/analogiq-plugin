#include <JuceHeader.h>
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class PluginEditorTests : public juce::UnitTest
{
public:
    PluginEditorTests() : UnitTest("PluginEditorTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        mockFetcher.reset();

        // Set up mock response for the units index
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json",
            R"({
                "units": [
                    {
                        "unitId": "test.eq.1",
                        "name": "Test EQ",
                        "type": "Series500",
                        "manufacturer": "Test Co",
                        "category": "EQ",
                        "categoryString": "equalizer",
                        "version": "1.0",
                        "schemaPath": "units/test.eq.1.json",
                        "thumbnailImage": "assets/test.eq.1.png",
                        "tags": ["eq", "test"]
                    }
                ]
            })");

        beginTest("Construction");
        {
            AnalogIQProcessor processor(mockFetcher);
            AnalogIQEditor editor(processor);
            expect(editor.getAudioProcessor() == &processor);
        }

        beginTest("Component Hierarchy");
        {
            AnalogIQProcessor processor(mockFetcher);
            AnalogIQEditor editor(processor);

            // Check for GearLibrary as direct child
            auto *gearLibrary = editor.findChildWithID("GearLibrary");
            expect(gearLibrary != nullptr);

            // Check for MainTabs
            auto *mainTabs = editor.findChildWithID("MainTabs");
            expect(mainTabs != nullptr);

            // TODO: Check for RackTab and NotesTab
        }

        beginTest("Resize Handling");
        {
            AnalogIQProcessor processor(mockFetcher);
            AnalogIQEditor editor(processor);
            editor.setSize(800, 600);
            expect(editor.getWidth() >= 800);
            expect(editor.getHeight() >= 600);
        }

        beginTest("Parameter Updates");
        {
            AnalogIQProcessor processor(mockFetcher);
            AnalogIQEditor editor(processor);
            bool parameterChanged = false;
            if (auto *param = processor.getParameters().getFirst())
            {
                param->setValue(0.5f);
                expect(parameterChanged);
            }
        }

        beginTest("Component Visibility");
        {
            AnalogIQProcessor processor(mockFetcher);
            AnalogIQEditor editor(processor);

            // Check GearLibrary visibility
            auto *library = editor.findChildWithID("GearLibrary");
            expect(library != nullptr && library->isVisible());

            // Check MainTabs and its contents
            auto *mainTabs = editor.findChildWithID("MainTabs");
            expect(mainTabs != nullptr && mainTabs->isVisible());

            // TODO: Check for RackTab and NotesTab
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static PluginEditorTests pluginEditorTests;