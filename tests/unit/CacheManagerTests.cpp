#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Source/CacheManager.h"
#include "MockFileSystem.h"
#include "PresetManager.h"

class CacheManagerTests : public juce::UnitTest
{
public:
    CacheManagerTests() : juce::UnitTest("CacheManagerTests") {}

    void runTest() override
    {
        // Use the singleton instance for the mock file system
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFileSystem.reset(); // Clear state before each test

        // Reset the singleton to use the mock file system with a mock cache root path
        CacheManager::resetInstance(mockFileSystem, "/mock/cache/root");
        CacheManager &cacheManager = CacheManager::getInstance();
        PresetManager::resetInstance(mockFileSystem, cacheManager);

        beginTest("Singleton Pattern");
        {
            CacheManager &instance1 = CacheManager::getInstance();
            CacheManager &instance2 = CacheManager::getInstance();
            expect(&instance1 == &instance2, "Singleton instances should be the same");
        }

        beginTest("Cache Initialization");
        {
            CacheManager &cache = CacheManager::getInstance();
            expect(cache.initializeCache(), "Cache initialization should succeed");
            // Don't test JUCE File.exists() - test through IFileSystem interface instead
        }

        beginTest("Directory Structure");
        {
            CacheManager &cache = CacheManager::getInstance();

            // Test that cache initialization succeeds (which creates directories)
            expect(cache.initializeCache(), "Cache initialization should succeed");

            // Test that we can save and load files (which verifies directories work)
            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testJsonData = "{\"test\": \"data\"}";

            expect(cache.saveUnitToCache(testUnitId, testJsonData), "Should be able to save unit");
            expect(cache.isUnitCached(testUnitId), "Unit should be cached after saving");
        }

        beginTest("Unit JSON Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testJsonData = R"({
                "unitId": "test-unit-1.0.0",
                "name": "Test Unit",
                "manufacturer": "Test Manufacturer",
                "category": "compressor",
                "version": "1.0.0",
                "controls": [
                    {
                        "id": "test-control",
                        "label": "Test Control",
                        "type": "knob"
                    }
                ]
            })";

            // Test unit JSON caching
            expect(!cache.isUnitCached(testUnitId), "Unit should not be cached initially");
            expect(cache.saveUnitToCache(testUnitId, testJsonData), "Saving unit should succeed");
            expect(cache.isUnitCached(testUnitId), "Unit should be cached after saving");

            // Test loading cached unit JSON
            juce::String loadedJson = cache.loadUnitFromCache(testUnitId);
            expect(loadedJson.isNotEmpty(), "Loaded JSON should not be empty");
            expect(loadedJson == testJsonData, "Loaded JSON should match original data");

            // Test unit path
            juce::String unitPath = cache.getCachedUnitPath(testUnitId);
            expect(mockFileSystem.getFileName(unitPath) == testUnitId + ".json", "Unit path should have correct filename");
        }

        beginTest("Image Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testFaceplateFilename = "test-unit-1.0.0.jpg";
            juce::String testThumbnailFilename = "test-unit-1.0.0.jpg";

            // Create a test image
            juce::Image testImage(juce::Image::RGB, 100, 100, true);
            testImage.clear(juce::Rectangle<int>(0, 0, 100, 100), juce::Colours::red);

            // Test faceplate caching
            expect(!cache.isFaceplateCached(testUnitId, testFaceplateFilename), "Faceplate should not be cached initially");
            expect(cache.saveFaceplateToCache(testUnitId, testFaceplateFilename, testImage), "Saving faceplate should succeed");
            expect(cache.isFaceplateCached(testUnitId, testFaceplateFilename), "Faceplate should be cached after saving");

            // Test thumbnail caching
            expect(!cache.isThumbnailCached(testUnitId, testThumbnailFilename), "Thumbnail should not be cached initially");
            expect(cache.saveThumbnailToCache(testUnitId, testThumbnailFilename, testImage), "Saving thumbnail should succeed");
            expect(cache.isThumbnailCached(testUnitId, testThumbnailFilename), "Thumbnail should be cached after saving");

            // Test loading cached images
            juce::Image loadedFaceplate = cache.loadFaceplateFromCache(testUnitId, testFaceplateFilename);
            expect(loadedFaceplate.isValid(), "Loaded faceplate should be valid");

            juce::Image loadedThumbnail = cache.loadThumbnailFromCache(testUnitId, testThumbnailFilename);
            expect(loadedThumbnail.isValid(), "Loaded thumbnail should be valid");
        }

        beginTest("Control Asset Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            juce::String testAssetPath = "knobs/test-knob.png";

            // Create test image data
            juce::Image testImage(juce::Image::RGB, 50, 50, true);
            testImage.clear(juce::Rectangle<int>(0, 0, 50, 50), juce::Colours::blue);

            juce::MemoryBlock imageData;
            juce::PNGImageFormat pngFormat;
            juce::MemoryOutputStream stream(imageData, false);
            pngFormat.writeImageToStream(testImage, stream);

            // Test control asset caching
            expect(!cache.isControlAssetCached(testAssetPath), "Control asset should not be cached initially");
            expect(cache.saveControlAssetToCache(testAssetPath, imageData), "Saving control asset should succeed");
            expect(cache.isControlAssetCached(testAssetPath), "Control asset should be cached after saving");

            // Test loading cached control asset
            juce::Image loadedAsset = cache.loadControlAssetFromCache(testAssetPath);
            expect(loadedAsset.isValid(), "Loaded control asset should be valid");
        }

        beginTest("Cache Size");
        {
            CacheManager &cache = CacheManager::getInstance();

            juce::int64 cacheSize = cache.getCacheSize();
            expect(cacheSize >= 0, "Cache size should be non-negative");
        }

        beginTest("File Path Generation");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testFaceplateFilename = "test-unit-1.0.0.jpg";
            juce::String testThumbnailFilename = "test-unit-1.0.0.png";

            // Test unit path
            juce::String unitPath = cache.getCachedUnitPath(testUnitId);
            expect(mockFileSystem.getFileName(unitPath) == testUnitId + ".json", "Unit path should have correct filename");

            // Test faceplate path
            juce::String faceplatePath = cache.getCachedFaceplatePath(testUnitId, testFaceplateFilename);
            expect(mockFileSystem.getFileName(faceplatePath) == testFaceplateFilename, "Faceplate path should have correct filename");

            // Test thumbnail path
            juce::String thumbnailPath = cache.getCachedThumbnailPath(testUnitId, testThumbnailFilename);
            expect(mockFileSystem.getFileName(thumbnailPath) == testThumbnailFilename, "Thumbnail path should have correct filename");

            // Test control asset path
            juce::String testAssetPath = "knobs/test-knob.png";
            juce::String assetPath = cache.getCachedControlAssetPath(testAssetPath);
            expect(mockFileSystem.getFileName(assetPath) == "test-knob.png", "Control asset path should have correct filename");
        }

        beginTest("Error Handling");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            // Test loading non-existent files
            juce::String nonExistentUnitId = "non-existent-unit-1.0.0";
            juce::String nonExistentFilename = "non-existent-unit-1.0.0.jpg";
            expect(!cache.isUnitCached(nonExistentUnitId), "Non-existent unit should not be cached");

            juce::String loadedData = cache.loadUnitFromCache(nonExistentUnitId);
            expect(loadedData.isEmpty(), "Loading non-existent unit should return empty string");

            juce::Image loadedImage = cache.loadFaceplateFromCache(nonExistentUnitId, nonExistentFilename);
            expect(!loadedImage.isValid(), "Loading non-existent faceplate should return invalid image");
        }

        beginTest("Recently Used Functionality");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            // Test adding to recently used
            expect(cache.addToRecentlyUsed("test.unit.1"), "Should add to recently used");
            expect(cache.addToRecentlyUsed("test.unit.2"), "Should add to recently used");
            expect(cache.addToRecentlyUsed("test.unit.1"), "Should move existing item to front");

            // Test getting recently used
            auto recentlyUsed = cache.getRecentlyUsed();
            expectEquals(recentlyUsed.size(), 2, "Should have 2 recently used items");
            expectEquals(recentlyUsed[0], juce::String("test.unit.1"), "First item should be test.unit.1");
            expectEquals(recentlyUsed[1], juce::String("test.unit.2"), "Second item should be test.unit.2");

            // Test checking if recently used
            expect(cache.isRecentlyUsed("test.unit.1"), "test.unit.1 should be recently used");
            expect(cache.isRecentlyUsed("test.unit.2"), "test.unit.2 should be recently used");
            expect(!cache.isRecentlyUsed("test.unit.3"), "test.unit.3 should not be recently used");

            // Test count
            expectEquals(cache.getRecentlyUsed().size(), 2, "Should have 2 recently used items");

            // Test removing from recently used
            expect(cache.removeFromRecentlyUsed("test.unit.1"), "Should remove from recently used");
            expect(!cache.isRecentlyUsed("test.unit.1"), "test.unit.1 should no longer be recently used");
            expect(cache.isRecentlyUsed("test.unit.2"), "test.unit.2 should still be recently used");

            // Test clearing recently used
            expect(cache.clearRecentlyUsed(), "Should clear recently used");
            expectEquals(cache.getRecentlyUsed().size(), 0, "Should have 0 recently used items");
        }

        beginTest("Favorites Functionality");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            CacheManager &cache = CacheManager::getInstance();

            // Test adding to favorites
            expect(cache.addToFavorites("test.unit.1"), "Should add to favorites");
            expect(cache.addToFavorites("test.unit.2"), "Should add to favorites");
            expect(cache.addToFavorites("test.unit.1"), "Should not duplicate existing favorite");

            // Test getting favorites
            auto favorites = cache.getFavorites();
            expectEquals(favorites.size(), 2, "Should have 2 favorite items");
            expect(favorites.contains("test.unit.1"), "Should contain test.unit.1");
            expect(favorites.contains("test.unit.2"), "Should contain test.unit.2");

            // Test checking if favorite
            expect(cache.isFavorite("test.unit.1"), "test.unit.1 should be a favorite");
            expect(cache.isFavorite("test.unit.2"), "test.unit.2 should be a favorite");
            expect(!cache.isFavorite("test.unit.3"), "test.unit.3 should not be a favorite");

            // Test count
            expectEquals(cache.getFavorites().size(), 2, "Should have 2 favorite items");

            // Test removing from favorites
            expect(cache.removeFromFavorites("test.unit.1"), "Should remove from favorites");
            expect(!cache.isFavorite("test.unit.1"), "test.unit.1 should no longer be a favorite");
            expect(cache.isFavorite("test.unit.2"), "test.unit.2 should still be a favorite");

            // Test clearing favorites
            expect(cache.clearFavorites(), "Should clear favorites");
            expectEquals(cache.getFavorites().size(), 0, "Should have 0 favorite items");
        }
    }
};

static CacheManagerTests cacheManagerTests;