#include <JuceHeader.h>
#include "GearLibrary.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "MockStateVerifier.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class GearLibraryTests : public juce::UnitTest
{
public:
    GearLibraryTests() : UnitTest("GearLibraryTests") {}

    /**
     * @brief Set up common mock data for LA-2A compressor tests.
     *
     * This helper method sets up all the mock responses needed for
     * testing the LA-2A compressor gear item.
     */
    void setUpLA2AMocks()
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();

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

        // Use static test image data to prevent JUCE leak detection
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

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
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Constructor");
        {
            // Reset mocks and set up test data
            MockStateVerifier::resetAndVerify("Constructor");
            setUpLA2AMocks();

            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();
            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");

            // Verify image can be loaded on demand
            if (library.getItems().size() > 0)
            {
                auto &item = const_cast<GearItem &>(library.getItems().getReference(0));

                // Explicitly load image for this test
                item.loadImage();

                expect(item.image.isValid(), "Gear item should have a valid image");
                expectEquals(item.image.getWidth(), 24, "Image width should be 24");
                expectEquals(item.image.getHeight(), 24, "Image height should be 24");
            }
        }

        beginTest("Adding Items");
        {
            // Reset mocks and set up test data
            MockStateVerifier::resetAndVerify("Adding Items");
            setUpLA2AMocks();

            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();
            library.addItem("test-gear-2", "Test Gear 2", "equalizer", "A test gear item", "Test Co 2", true);
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
            // Reset mocks and set up test data
            MockStateVerifier::resetAndVerify("Item Retrieval");
            setUpLA2AMocks();

            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();
            library.addItem("test-gear", "Test Gear", "preamp", "A test gear item", "Test Co", true);
            auto *item = library.getGearItem(1);
            expect(item != nullptr);
            expectEquals(item->categoryString, juce::String("preamp"), "Retrieved item name should match");
            expect(library.getGearItem(999) == nullptr);
        }

        beginTest("URL Construction");
        {
            // Reset mocks for clean state
            MockStateVerifier::resetAndVerify("URL Construction");

            expectEquals(GearLibrary::getFullUrl("http://example.com"), juce::String("http://example.com"), "Full URLs should remain unchanged");
            expectEquals(GearLibrary::getFullUrl("/absolute/path"), juce::String("/absolute/path"), "Absolute paths should remain unchanged");
            expect(GearLibrary::getFullUrl("assets/image.jpg").contains("assets/"), "Asset URLs should contain assets/");
            expect(GearLibrary::getFullUrl("units/schema.json").contains("units/"), "Unit URLs should contain units/");
        }

        beginTest("Loading Library");
        {
            // Reset mocks and set up test data
            MockStateVerifier::resetAndVerify("Loading Library");
            setUpLA2AMocks();

            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();

            expectEquals(library.getItems().size(), 1, "Library should have one item after loading");
            expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should request units/index.json");
        }

        beginTest("Loading Library Error");
        {
            // Reset mocks and set up error condition
            MockStateVerifier::resetAndVerify("Loading Library Error");
            mockFetcher.setError("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json");

            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();

            // Wait for async operation to complete
            juce::Thread::sleep(100);

            expect(library.getItems().isEmpty(), "Library should be empty after failed load");
            expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should attempt to request units/index.json");
        }

        // Clean up mock responses
        mockFetcher.reset();

        beginTest("Recently Used Functionality");
        {
            // Create a gear library
            auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();

            // Add some test items
            library.addItem("test-eq", "Test EQ", "equalizer", "Test description", "Test Manufacturer", true);
            library.addItem("test-compressor", "Test Compressor", "compressor", "Test description", "Test Manufacturer", true);

            // Get the items to access their unit IDs
            const auto &items = library.getItems();
            expectEquals(items.size(), 2, "Should have 2 items");

            // Add items to recently used
            cacheManager.addToRecentlyUsed(items[0].unitId);
            cacheManager.addToRecentlyUsed(items[1].unitId);

            // Refresh the tree view
            library.refreshTreeView();

            // Test clearing recently used
            library.clearRecentlyUsed();
            expectEquals(cacheManager.getRecentlyUsed().size(), 0, "Recently used should be cleared");
        }

        beginTest("Favorites Functionality");
        {
            // Create a gear library
            auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
            GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
            library.loadLibrary();

            // Add some test items
            library.addItem("test-eq-2", "Test EQ", "equalizer", "Test description", "Test Manufacturer", true);
            library.addItem("test-compressor-2", "Test Compressor", "compressor", "Test description", "Test Manufacturer", true);

            // Get the items to access their unit IDs
            const auto &items = library.getItems();
            expectEquals(items.size(), 2, "Should have 2 items");

            // Add items to favorites
            cacheManager.addToFavorites(items[0].unitId);
            cacheManager.addToFavorites(items[1].unitId);

            // Refresh the tree view
            library.refreshTreeView();

            // Test clearing favorites
            library.clearFavorites();
            expectEquals(cacheManager.getFavorites().size(), 0, "Favorites should be cleared");
        }
    }
};

static GearLibraryTests gearLibraryTests;
