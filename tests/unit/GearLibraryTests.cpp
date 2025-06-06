#include <JuceHeader.h>
#include "GearLibrary.h"
#include "TestFixture.h"

class GearLibraryTests : public juce::UnitTest
{
public:
    GearLibraryTests() : UnitTest("GearLibraryTests") {}

    void runTest() override
    {
        TestFixture fixture;
        beginTest("Construction");
        {
            GearLibrary library;
            expect(library.getItems().isEmpty());
        }

        beginTest("Adding Items");
        {
            GearLibrary library;
            library.addItem("Test Gear", "Compressor", "A test gear item", "Test Co");
            expectEquals(library.getItems().size(), 1, "Library should have exactly one item after adding");
            expectEquals(library.getItems()[0].name, juce::String("Test Gear"), "Item name should match what was added");
            expectEquals(library.getItems()[0].manufacturer, juce::String("Test Co"), "Manufacturer should match what was added");
            expectEquals(library.getItems()[0].categoryString, juce::String("compressor"), "Category should match what was added");
        }

        beginTest("Item Retrieval");
        {
            GearLibrary library;
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
    }
};

static GearLibraryTests gearLibraryTests;