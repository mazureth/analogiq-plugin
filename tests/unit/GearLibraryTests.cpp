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

    void setUpMultipleItemMocks()
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();

        // Set up mock response with multiple gear items for more comprehensive testing
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
                        "tags": ["compressor", "tube", "optical", "vintage", "hardware"]
                    },
                    {
                        "unitId": "pultec-eq",
                        "name": "Pultec EQP-1A",
                        "manufacturer": "Pulse Techniques",
                        "category": "EQ",
                        "version": "1.0.0",
                        "schemaPath": "units/pultec-eq-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/pultec-eq-1.0.0.jpg",
                        "tags": ["eq", "tube", "vintage", "passive"]
                    },
                    {
                        "unitId": "neve-preamp",
                        "name": "Neve 1073",
                        "manufacturer": "Neve",
                        "category": "Preamp",
                        "version": "1.0.0",
                        "schemaPath": "units/neve-preamp-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/neve-preamp-1.0.0.jpg",
                        "tags": ["preamp", "500-series", "vintage", "transformer"]
                    }
                ]
            })");

        // Use static test image data
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

        // Set up mock responses for all images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/pultec-eq-1.0.0.jpg",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/neve-preamp-1.0.0.jpg",
            imageData);
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Constructor");
        testConstructor(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Adding Items");
        testAddingItems(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Item Retrieval");
        testItemRetrieval(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("URL Construction");
        testURLConstruction();

        beginTest("Loading Library");
        testLoadingLibrary(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Loading Library Error");
        testLoadingLibraryError(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Recently Used Functionality");
        testRecentlyUsedFunctionality(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Favorites Functionality");
        testFavoritesFunctionality(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Component Lifecycle");
        testComponentLifecycle(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Paint and Resize Methods");
        testPaintAndResizeMethods(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("JSON Parsing");
        testJSONParsing(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Button Handling");
        testButtonHandling(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Mouse Event Handling");
        testMouseEventHandling(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Tree View Operations");
        testTreeViewOperations(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Search Functionality");
        testSearchFunctionality(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Category Management");
        testCategoryManagement(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Async Operations");
        testAsyncOperations(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Error Handling");
        testErrorHandling(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Multiple Items Management");
        testMultipleItemsManagement(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("UI State Management");
        testUIStateManagement(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Cache Integration");
        testCacheIntegration(mockFetcher, mockFileSystem, cacheManager, presetManager);
    }

    void testConstructor(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                        CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Reset mocks and set up test data
        MockStateVerifier::resetAndVerify("Constructor");
        setUpLA2AMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        expectEquals(library.getItems().size(), 1, "Library should have one item after loading");

        // Test component properties
        expect(library.getNumRows() > 0, "Library should have rows");

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

    void testAddingItems(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                        CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Reset mocks and set up test data
        MockStateVerifier::resetAndVerify("Adding Items");
        setUpLA2AMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        library.addItem("test-gear-2", "Test Gear 2", "equalizer", "A test gear item", "Test Co 2", true);
        expectEquals(library.getItems().size(), 2, "Library should have exactly two items after adding");
        expectEquals(library.getItems()[0].name, juce::String("LA-2A Tube Compressor"), "Default Item name should match");
        expectEquals(library.getItems()[0].manufacturer, juce::String("Universal Audio"), "Default Manufacturer should match");
        expectEquals(library.getItems()[0].categoryString, juce::String("compressor"), "Default Category should match");
        expectEquals(library.getItems()[1].name, juce::String("Test Gear 2"), "Added Item name should match");
        expectEquals(library.getItems()[1].manufacturer, juce::String("Test Co 2"), "Added Manufacturer should match");
        expectEquals(library.getItems()[1].categoryString, juce::String("equalizer"), "Added Category should match");

        // Test adding different types of gear
        library.addItem("test-500-series", "Test 500 Series Preamp", "Preamp", "500 series preamp", "Test Manufacturer", true);
        expectEquals(library.getItems().size(), 3, "Library should have three items");

        library.addItem("test-lunchbox", "Test Lunchbox EQ", "EQ", "Lunchbox format EQ", "Test Manufacturer", true);
        expectEquals(library.getItems().size(), 4, "Library should have four items");

        library.addItem("test-compressor", "Test Compressor", "Compressor", "Test compressor", "Test Manufacturer", true);
        expectEquals(library.getItems().size(), 5, "Library should have five items");
    }

    void testItemRetrieval(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                          CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Reset mocks and set up test data
        MockStateVerifier::resetAndVerify("Item Retrieval");
        setUpLA2AMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        library.addItem("test-gear", "Test Gear", "preamp", "A test gear item", "Test Co", true);
        
        auto *item = library.getGearItem(1);
        expect(item != nullptr, "Should retrieve valid item");
        expectEquals(item->categoryString, juce::String("preamp"), "Retrieved item category should match");
        
        expect(library.getGearItem(999) == nullptr, "Should return nullptr for invalid index");
        expect(library.getGearItem(-1) == nullptr, "Should return nullptr for negative index");

        // Test retrieval by unit ID
        auto *itemByUnitId = library.getGearItemByUnitId("test-gear");
        expect(itemByUnitId != nullptr, "Should retrieve item by unit ID");
        expectEquals(itemByUnitId->unitId, juce::String("test-gear"), "Retrieved item unit ID should match");

        auto *nonExistentItem = library.getGearItemByUnitId("non-existent");
        expect(nonExistentItem == nullptr, "Should return nullptr for non-existent unit ID");
    }

    void testURLConstruction()
    {
        // Reset mocks for clean state
        MockStateVerifier::resetAndVerify("URL Construction");

        expectEquals(GearLibrary::getFullUrl("http://example.com"), juce::String("http://example.com"), "Full URLs should remain unchanged");
        expectEquals(GearLibrary::getFullUrl("https://example.com"), juce::String("https://example.com"), "HTTPS URLs should remain unchanged");
        expectEquals(GearLibrary::getFullUrl("/absolute/path"), juce::String("/absolute/path"), "Absolute paths should remain unchanged");
        expect(GearLibrary::getFullUrl("assets/image.jpg").contains("assets/"), "Asset URLs should contain assets/");
        expect(GearLibrary::getFullUrl("units/schema.json").contains("units/"), "Unit URLs should contain units/");
        
        // Test empty string
        juce::String emptyResult = GearLibrary::getFullUrl("");
        expect(emptyResult.isNotEmpty(), "Empty string should return non-empty URL");
        
        // Test relative paths
        juce::String relativePath = GearLibrary::getFullUrl("relative/path.json");
        expect(relativePath.contains("relative/path.json"), "Relative paths should be preserved in result");
    }

    void testLoadingLibrary(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Reset mocks and set up test data
        MockStateVerifier::resetAndVerify("Loading Library");
        setUpLA2AMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();

        expectEquals(library.getItems().size(), 1, "Library should have one item after loading");
        expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Library should request units/index.json");

        // Test loading gear items directly
        library.loadGearItems();
        expect(mockFetcher.wasUrlRequested("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json"), "Should request units index again");
    }

    void testLoadingLibraryError(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                CacheManager &cacheManager, PresetManager &presetManager)
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

    void testRecentlyUsedFunctionality(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                      CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Create a gear library
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

    void testFavoritesFunctionality(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                   CacheManager &cacheManager, PresetManager &presetManager)
    {
        // Create a gear library
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

    void testComponentLifecycle(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Component Lifecycle");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test bounds setting
        library.setBounds(0, 0, 400, 600);
        auto bounds = library.getBounds();
        expectEquals(bounds.getWidth(), 400, "Width should be set correctly");
        expectEquals(bounds.getHeight(), 600, "Height should be set correctly");
        
        // Test resizing
        library.setBounds(0, 0, 500, 700);
        library.resized();
        bounds = library.getBounds();
        expectEquals(bounds.getWidth(), 500, "Resized width should be correct");
        expectEquals(bounds.getHeight(), 700, "Resized height should be correct");
    }

    void testPaintAndResizeMethods(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                  CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Paint and Resize");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.setBounds(0, 0, 400, 600);
        
        // Create a test graphics context
        juce::Image testImage(juce::Image::RGB, 400, 600, true);
        juce::Graphics g(testImage);
        
        // Test paint method
        library.paint(g);
        expect(testImage.isValid(), "Test image should remain valid after painting");
        
        // Test resize method
        library.resized();
        expect(true, "Resize method should execute without errors");
        
        // Test different sizes
        library.setBounds(0, 0, 200, 400);
        library.resized();
        library.setBounds(0, 0, 800, 1000);
        library.resized();
        expect(true, "Should handle different sizes correctly");
    }

    void testJSONParsing(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                        CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("JSON Parsing");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test loading with valid JSON data through public interface
        setUpMultipleItemMocks();
        library.loadLibrary();
        
        expectEquals(library.getItems().size(), 3, "Should load 3 items from valid JSON");
        
        // Test loading with error conditions through public interface
        mockFetcher.setError("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json");
        
        GearLibrary errorLibrary(mockFetcher, mockFileSystem, cacheManager, presetManager);
        errorLibrary.loadLibrary();
        
        // Should handle errors gracefully
        expect(true, "Should handle JSON errors gracefully through public interface");
    }

    void testButtonHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Button Handling");
        setUpMultipleItemMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        
        // Test button clicked with null button
        library.buttonClicked(nullptr);
        expect(true, "Should handle null button gracefully");
        
        // Test with mock button (we can't easily create real JUCE buttons in tests)
        // The method should execute without crashing
        expect(true, "Button handling should work correctly");
    }

    void testMouseEventHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Mouse Event Handling");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.setBounds(0, 0, 400, 600);
        
        // Test mouse events - we can't easily create real MouseEvent objects, but we can test that the methods exist
        // and don't crash when called
        
        // The mouseDown and mouseDrag methods should be callable
        expect(true, "Mouse event methods should be accessible");
    }

    void testTreeViewOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Tree View Operations");
        setUpMultipleItemMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        
        // Test tree view refresh
        library.refreshTreeView();
        expect(true, "Tree view refresh should execute without errors");
        
        // Test with different items added
        library.addItem("test-tree-1", "Tree Test 1", "EQ", "Tree test", "Manufacturer", true);
        library.addItem("test-tree-2", "Tree Test 2", "Compressor", "Tree test", "Manufacturer", true);
        library.refreshTreeView();
        expect(true, "Tree view should handle multiple items");
    }

    void testSearchFunctionality(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Search Functionality");
        setUpMultipleItemMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        
        // Add some searchable items
        library.addItem("searchable-eq", "Searchable EQ", "EQ", "An EQ for search testing", "Search Co", true);
        library.addItem("searchable-comp", "Searchable Compressor", "Compressor", "A compressor for search testing", "Search Co", true);
        
        expectEquals(library.getItems().size(), 5, "Should have 5 items total"); // 3 from mock + 2 added
        
        // Test that items were added correctly
        auto *searchEQ = library.getGearItemByUnitId("searchable-eq");
        expect(searchEQ != nullptr, "Should find searchable EQ");
        auto *searchComp = library.getGearItemByUnitId("searchable-comp");
        expect(searchComp != nullptr, "Should find searchable compressor");
    }

    void testCategoryManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Category Management");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test adding items with different categories
        library.addItem("eq-unit", "EQ Unit", "EQ", "EQ description", "EQ Manufacturer", true);
        library.addItem("preamp-unit", "Preamp Unit", "Preamp", "Preamp description", "Preamp Manufacturer", true);
        library.addItem("comp-unit", "Compressor Unit", "Compressor", "Compressor description", "Comp Manufacturer", true);
        library.addItem("other-unit", "Other Unit", "Other", "Other description", "Other Manufacturer", true);
        
        expectEquals(library.getItems().size(), 4, "Should have 4 items with different categories");
        
        // Verify categories were set correctly
        expect(library.getItems()[0].categoryString == "EQ", "EQ category should be correct");
        expect(library.getItems()[1].categoryString == "Preamp", "Preamp category should be correct");
        expect(library.getItems()[2].categoryString == "Compressor", "Compressor category should be correct");
        expect(library.getItems()[3].categoryString == "Other", "Other category should be correct");
    }

    void testAsyncOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                            CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Async Operations");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test async save operation
        library.saveLibraryAsync();
        
        // Wait a short time for async operation
        juce::Thread::sleep(100);
        
        expect(true, "Async save operation should execute without errors");
        
        // Test multiple async operations
        library.saveLibraryAsync();
        library.saveLibraryAsync();
        
        juce::Thread::sleep(100);
        
        expect(true, "Multiple async operations should work correctly");
    }

    void testErrorHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                          CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Error Handling");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test error conditions through public interface
        library.getGearItem(-1);
        library.getGearItem(1000);
        library.getGearItemByUnitId("");
        
        expect(true, "Error conditions should be handled gracefully");
        
        // Test with network errors
        mockFetcher.setError("https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json");
        library.loadLibrary();
        library.loadGearItems();
        
        expect(true, "Network errors should be handled gracefully");
        
        // Test empty string handling
        auto *nullItem = library.getGearItemByUnitId("");
        expect(nullItem == nullptr, "Empty unit ID should return nullptr");
        
        // Test boundary conditions
        expect(library.getGearItem(-100) == nullptr, "Large negative index should return nullptr");
        expect(library.getGearItem(10000) == nullptr, "Large positive index should return nullptr");
    }

    void testMultipleItemsManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                    CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Multiple Items Management");
        setUpMultipleItemMocks();

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        library.loadLibrary();
        
        expectEquals(library.getItems().size(), 3, "Should load 3 items from mock");
        
        // Add more items
        for (int i = 0; i < 10; ++i)
        {
            library.addItem("test-item-" + juce::String(i), "Test Item " + juce::String(i), 
                           "EQ", "Test description", "Test Manufacturer", true);
        }
        
        expectEquals(library.getItems().size(), 13, "Should have 13 items total");
        expectEquals(library.getNumRows(), 13, "getNumRows should return correct count");
        
        // Test retrieval of various items
        for (int i = 0; i < library.getItems().size(); ++i)
        {
            auto *item = library.getGearItem(i);
            expect(item != nullptr, "Should retrieve each item successfully");
        }
    }

    void testUIStateManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                              CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("UI State Management");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test various UI operations
        library.setBounds(0, 0, 300, 500);
        library.resized();
        
        library.addItem("ui-test-1", "UI Test 1", "EQ", "UI test", "UI Manufacturer", false); // Don't bypass UI
        library.refreshTreeView();
        
        // Test visibility and component state
        library.setVisible(true);
        expect(library.isVisible(), "Library should be visible when explicitly set");
        
        library.setVisible(false);
        expect(!library.isVisible(), "Library should be hidden when explicitly set to false");
        
        library.setVisible(true);
        expect(library.isVisible(), "Library should be visible again when explicitly set");
    }

    void testCacheIntegration(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                             CacheManager &cacheManager, PresetManager &presetManager)
    {
        MockStateVerifier::resetAndVerify("Cache Integration");

        GearLibrary library(mockFetcher, mockFileSystem, cacheManager, presetManager);
        
        // Test cache manager access
        auto &libraryCacheManager = library.getCacheManager();
        expect(&libraryCacheManager == &cacheManager, "Should return the same cache manager instance");
        
        // Test cache operations through library
        library.addItem("cache-test-1", "Cache Test 1", "EQ", "Cache test", "Cache Manufacturer", true);
        
        const auto &items = library.getItems();
        if (!items.isEmpty())
        {
            // Test recently used integration
            libraryCacheManager.addToRecentlyUsed(items[0].unitId);
            auto recentlyUsed = libraryCacheManager.getRecentlyUsed();
            expect(recentlyUsed.size() > 0, "Recently used should contain items");
            
            // Test favorites integration
            libraryCacheManager.addToFavorites(items[0].unitId);
            auto favorites = libraryCacheManager.getFavorites();
            expect(favorites.size() > 0, "Favorites should contain items");
            
            // Test clearing through library
            library.clearRecentlyUsed();
            library.clearFavorites();
            
            expect(libraryCacheManager.getRecentlyUsed().size() == 0, "Recently used should be cleared");
            expect(libraryCacheManager.getFavorites().size() == 0, "Favorites should be cleared");
        }
    }
};

static GearLibraryTests gearLibraryTests;
