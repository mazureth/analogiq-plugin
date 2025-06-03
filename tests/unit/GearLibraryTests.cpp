#include <JuceHeader.h>
#include "../../Source/GearLibrary.h"

class GearLibraryTests : public juce::UnitTest
{
public:
    GearLibraryTests() : UnitTest("GearLibrary Tests") {}

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

            // Test adding a gear item with all required parameters
            library.addItem("Test Gear", "Effects", "A test gear item", "Test Co");

            // Verify the item was added
            expect(library.getItems().size() == 1);
            expect(library.getItems()[0].name == "Test Gear");
            expect(library.getItems()[0].manufacturer == "Test Co");
            expect(library.getItems()[0].categoryString == "Effects");
        }

        beginTest("Item Retrieval");
        {
            GearLibrary library;

            // Add a test item
            library.addItem("Test Gear", "Effects", "A test gear item", "Test Co");

            // Test getGearItem
            auto *item = library.getGearItem(0);
            expect(item != nullptr);
            expect(item->name == "Test Gear");

            // Test invalid index
            expect(library.getGearItem(999) == nullptr);
        }

        beginTest("URL Construction");
        {
            // Test various URL construction scenarios
            expect(GearLibrary::getFullUrl("http://example.com") == "http://example.com");
            expect(GearLibrary::getFullUrl("/absolute/path") == "/absolute/path");
            expect(GearLibrary::getFullUrl("assets/image.jpg").contains("assets/"));
            expect(GearLibrary::getFullUrl("units/schema.json").contains("units/"));
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static GearLibraryTests gearLibraryTests;