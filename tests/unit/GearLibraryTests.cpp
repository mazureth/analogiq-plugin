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
            GearLibrary library(mockFetcher);
            expect(library.getItems().isEmpty());
        }

        beginTest("Adding Items");
        {
            GearLibrary library(mockFetcher);
            library.addItem("Test Gear", "Compressor", "A test gear item", "Test Co");
            expectEquals(library.getItems().size(), 1, "Library should have exactly one item after adding");
            expectEquals(library.getItems()[0].name, juce::String("Test Gear"), "Item name should match what was added");
            expectEquals(library.getItems()[0].manufacturer, juce::String("Test Co"), "Manufacturer should match what was added");
            expectEquals(library.getItems()[0].categoryString, juce::String("compressor"), "Category should match what was added");
        }

        beginTest("Item Retrieval");
        {
            GearLibrary library(mockFetcher);
            library.addItem("Test Gear", "Effects", "A test gear item", "Test Co");
            auto *item = library.getGearItem(0);
            expect(item != nullptr);
            expectEquals(item->name, juce::String("Test Gear"), "Retrieved item name should match");
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
            mockFetcher.setResponse("http://example.com/gear.json",
                                    R"({"items": [{"name": "Test Gear", "category": "Compressor", "description": "Test", "manufacturer": "Test Co"}]})");

            GearLibrary library(mockFetcher);
            library.loadLibraryAsync();

            // Wait for async load to complete
            juce::Thread::sleep(100);

            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");
            expect(mockFetcher.wasUrlRequested("http://example.com/gear.json"), "Library should request gear.json");
        }

        beginTest("Loading Library Error");
        {
            mockFetcher.setError("http://example.com/gear.json");

            GearLibrary library(mockFetcher);
            library.loadLibraryAsync();

            // Wait for async load to complete
            juce::Thread::sleep(100);

            expect(library.getItems().isEmpty(), "Library should be empty after failed load");
            expect(mockFetcher.wasUrlRequested("http://example.com/gear.json"), "Library should attempt to request gear.json");
        }
    }
};

static GearLibraryTests gearLibraryTests;