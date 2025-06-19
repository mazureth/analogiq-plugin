#include <JuceHeader.h>
#include "GearLibrary.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class GearLibraryTests : public juce::UnitTest
{
public:
    GearLibraryTests() : UnitTest("GearLibraryTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        mockFetcher.reset();

        beginTest("Construction");
        {
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

            // Create a simple JPEG image for testing
            juce::Image testImage(juce::Image::RGB, 24, 24, true);
            {
                juce::Graphics g(testImage);
                g.fillAll(juce::Colours::darkgrey);
                g.setColour(juce::Colours::white);
                g.drawText("Test", testImage.getBounds(), juce::Justification::centred, true);
            }

            // Convert to JPEG format
            juce::MemoryOutputStream stream;
            juce::JPEGImageFormat jpegFormat;
            jpegFormat.setQuality(0.8f);
            jpegFormat.writeImageToStream(testImage, stream);
            juce::MemoryBlock imageData(stream.getData(), stream.getDataSize());

            // Set up mock response for the compressor image
            mockFetcher.setBinaryResponse(
                "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
                imageData);

            // Set up mock response for the compressor thumbnail
            mockFetcher.setBinaryResponse(
                "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
                imageData);

            // Set up mock response for the compressor knob
            mockFetcher.setBinaryResponse(
                "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
                imageData);

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

            GearLibrary library(mockFetcher);
            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");

            // Verify image was loaded
            if (library.getItems().size() > 0)
            {
                const auto &item = library.getItems().getReference(0);
                expect(item.image.isValid(), "Gear item should have a valid image");
                expectEquals(item.image.getWidth(), 24, "Image width should be 24");
                expectEquals(item.image.getHeight(), 24, "Image height should be 24");
            }
        }

        beginTest("Adding Items");
        {
            GearLibrary library(mockFetcher);
            library.addItem("Test Gear 2", "EQ", "A test gear item", "Test Co 2");
            expectEquals(library.getItems().size(), 2, "Library should have exactly one item after adding");
            expectEquals(library.getItems()[0].name, juce::String("LA-2A Tube Compressor"), "Default Item name should match");
            expectEquals(library.getItems()[0].manufacturer, juce::String("Universal Audio"), "Default Manufacturer should match");
            expectEquals(library.getItems()[0].categoryString, juce::String("compressor"), "Default Category should match");
            expectEquals(library.getItems()[1].name, juce::String("Test Gear 2"), "Added Item name should match");
            expectEquals(library.getItems()[1].manufacturer, juce::String("Test Co 2"), "Added Manufacturer should match");
            expectEquals(library.getItems()[1].categoryString, juce::String("equalizer"), "Added Category should match");
        }

        beginTest("Item Retrieval");
        {
            GearLibrary library(mockFetcher);
            library.addItem("Test Gear", "Preamp", "A test gear item", "Test Co");
            auto *item = library.getGearItem(1);
            expect(item != nullptr);
            expectEquals(item->categoryString, juce::String("preamp"), "Retrieved item name should match");
            expect(library.getGearItem(999) == nullptr);
        }

        beginTest("URL Construction");
        {
            expectEquals(GearLibrary::getFullUrl("http://example.com"), juce::String("http://example.com"), "Full URLs should remain unchanged");
            expectEquals(GearLibrary::getFullUrl("/absolute/path"), juce::String("/absolute/path"), "Absolute paths should remain unchanged");
            expect(GearLibrary::getFullUrl("assets/image.jpg").contains("assets/"), "Asset URLs should contain assets/");
            expect(GearLibrary::getFullUrl("units/schema.json").contains("units/"), "Unit URLs should contain units/");
        }

        beginTest("Loading Library");
        {
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

            GearLibrary library(mockFetcher);
            library.loadLibraryAsync();

            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");
            expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should request units/index.json");
        }

        beginTest("Loading Library Error");
        {
            mockFetcher.setError("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json");

            GearLibrary library(mockFetcher);
            library.loadLibraryAsync();

            // Wait for async operation to complete
            juce::Thread::sleep(100);

            expect(library.getItems().isEmpty(), "Library should be empty after failed load");
            expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should attempt to request units/index.json");
        }

        // Clean up mock responses
        mockFetcher.reset();
    }
};

static GearLibraryTests gearLibraryTests;