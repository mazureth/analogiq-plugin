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
                            "unitId": "test.compressor.1",
                            "name": "Test Gear",
                            "type": "Series500",
                            "manufacturer": "Test Co",
                            "category": "Compressor",
                            "categoryString": "compressor",
                            "version": "1.0",
                            "schemaPath": "units/test.compressor.1.json",
                            "thumbnailImage": "assets/test.compressor.1.png",
                            "tags": ["compressor", "test"]
                        }
                    ]
                })");

            GearLibrary library(mockFetcher);
            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");
        }

        beginTest("Adding Items");
        {
            GearLibrary library(mockFetcher);
            library.addItem("Test Gear 2", "EQ", "A test gear item", "Test Co 2");
            expectEquals(library.getItems().size(), 2, "Library should have exactly one item after adding");
            expectEquals(library.getItems()[0].name, juce::String("Test Gear"), "Default Item name should match");
            expectEquals(library.getItems()[0].manufacturer, juce::String("Test Co"), "Default Manufacturer should match");
            expectEquals(library.getItems()[0].categoryString, juce::String("Compressor"), "Default Category should match");
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
                            "unitId": "test.compressor.1",
                            "name": "Test Gear",
                            "type": "Series500",
                            "manufacturer": "Test Co",
                            "category": "Compressor",
                            "categoryString": "compressor",
                            "version": "1.0",
                            "schemaPath": "units/test.compressor.1.json",
                            "thumbnailImage": "assets/test.compressor.1.png",
                            "tags": ["compressor", "test"]
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

            // Wait for both async operations to complete (filters + gear items)
            juce::Thread::sleep(100);

            expect(library.getItems().isEmpty(), "Library should be empty after failed load");
            expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should attempt to request units/index.json");
        }
    }
};

static GearLibraryTests gearLibraryTests;