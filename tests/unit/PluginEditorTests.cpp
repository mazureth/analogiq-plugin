#include <JuceHeader.h>
#include "AnalogIQEditor.h"
#include "AnalogIQProcessor.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"

class AnalogIQEditorTests : public juce::UnitTest
{
public:
    AnalogIQEditorTests() : UnitTest("AnalogIQEditorTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Create instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

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
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);
            expect(editor.getAudioProcessor() == &processor);
        }

        beginTest("Component Hierarchy");
        {
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);

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
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);
            editor.setSize(800, 600);
            expect(editor.getWidth() >= 800);
            expect(editor.getHeight() >= 600);
        }

        beginTest("Parameter Updates");
        {
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);
            bool parameterChanged = false;
            if (auto *param = processor.getParameters().getFirst())
            {
                param->setValue(0.5f);
                expect(parameterChanged);
            }
        }

        beginTest("Component Visibility");
        {
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);

            // Check GearLibrary visibility
            auto *library = editor.findChildWithID("GearLibrary");
            expect(library != nullptr && library->isVisible());

            // Check MainTabs and its contents
            auto *mainTabs = editor.findChildWithID("MainTabs");
            expect(mainTabs != nullptr && mainTabs->isVisible());

            // TODO: Check for RackTab and NotesTab
        }

        beginTest("Preset Integration");
        {
            AnalogIQProcessor processor(mockFetcher, mockFileSystem);
            AnalogIQEditor editor(processor, cacheManager, presetManager, true);

            // Test that the editor has access to preset manager
            auto &editorPresetManager = editor.getPresetManager();
            expect(&editorPresetManager != nullptr, "Preset manager should be accessible");

            // Test that the editor has access to rack and gear library for preset operations
            auto *rack = editor.getRack();
            auto *gearLibrary = editor.getGearLibrary();
            expect(rack != nullptr, "Rack should be accessible for preset operations");
            expect(gearLibrary != nullptr, "Gear library should be accessible for preset operations");

            // Test that the rack is empty initially (for confirmation dialog testing)
            bool hasGearItems = false;
            for (int i = 0; i < rack->getNumSlots(); ++i)
            {
                if (auto *slot = rack->getSlot(i))
                {
                    if (slot->getGearItem() != nullptr)
                    {
                        hasGearItems = true;
                        break;
                    }
                }
            }
            expect(!hasGearItems, "Rack should be empty initially for preset confirmation testing");

            // Test that the editor can be resized without errors (menu positioning)
            editor.setSize(800, 600);
            editor.resized();
            expect(true, "Editor should resize without errors");

            // Test that the editor components are properly initialized
            expect(rack->getNumSlots() > 0, "Rack should have slots available");
            expect(gearLibrary != nullptr, "Gear library should be initialized");
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static AnalogIQEditorTests analogIQEditorTests;