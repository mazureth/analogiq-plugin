#include <JuceHeader.h>
#include "../../Source/GearLibrary.h"

class GearLibraryTests : public juce::UnitTest
{
public:
    GearLibraryTests() : UnitTest("GearLibraryTests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            GearLibrary library;
            expect(library.getItems().isEmpty());
        }

        beginTest("Adding Items");
        {
            GearLibrary library;
            library.addItem("Test Gear", "Compressor", "A test gear item", "Test Co");
            expect(library.getItems().size() == 1);
            expect(library.getItems()[0].name == "Test Gear");
            expect(library.getItems()[0].manufacturer == "Test Co");
            expect(library.getItems()[0].categoryString == "compressor");
        }

        beginTest("Item Retrieval");
        {
            GearLibrary library;
            library.addItem("Test Gear", "Compressor", "A test gear item", "Test Co");
            auto *item = library.getGearItem(0);
            expect(item != nullptr);
            expect(item->name == "Test Gear");
            expect(library.getGearItem(999) == nullptr);
        }

        beginTest("URL Construction");
        {
            expect(GearLibrary::getFullUrl("http://example.com") == "http://example.com");
            expect(GearLibrary::getFullUrl("/absolute/path") == "/absolute/path");
            expect(GearLibrary::getFullUrl("assets/image.jpg").contains("assets/"));
            expect(GearLibrary::getFullUrl("units/schema.json").contains("units/"));
        }
    }
};

static GearLibraryTests gearLibraryTests;