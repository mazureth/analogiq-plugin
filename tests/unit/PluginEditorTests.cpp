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
                        "unitId": "la2a-compressor",
                        "name": "LA-2A Tube Compressor",
                        "manufacturer": "Universal Audio",
                        "category": "compressor",
                        "version": "1.0.0",
                        "schemaPath": "units/la2a-compressor-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                        "tags": [
                            "compressor",
                            "tube",
                            "optical",
                            "vintage",
                            "hardware"
                        ]
                    }
                ]
            })");

        // Set up mock response for the compressor image
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
            "mock_image_data"); // The actual content doesn't matter for the test

        // Set up mock response for the compressor image
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            "mock_image_data"); // The actual content doesn't matter for the test

        // Set up mock response for the compressor image
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
            "mock_image_data"); // The actual content doesn't matter for the test

        // Set up mock response for the compressor schema
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/la2a-compressor-1.0.0.json",
            R"({
                    "unitId": "la2a-compressor",
                    "name": "LA-2A Tube Compressor",
                    "manufacturer": "Universal Audio",
                    "tags": [
                        "compressor",
                        "tube",
                        "optical",
                        "vintage",
                        "hardware"
                    ],
                    "version": "1.0.0",
                    "category": "compressor",
                    "formFactor": "19-inch-rack",
                    "faceplateImage": "assets/faceplates/la2a-compressor-1.0.0.jpg",
                    "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                    "width": 1900,
                    "height": 525,
                    "controls": [
                        {
                            "id": "peak-reduction",
                            "label": "Peak Reduction",
                            "type": "knob",
                            "position": {
                            "x": 0.68,
                            "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        },
                        {
                            "id": "gain",
                            "label": "Gain",
                            "type": "knob",
                            "position": {
                                "x": 0.257,
                                "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
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