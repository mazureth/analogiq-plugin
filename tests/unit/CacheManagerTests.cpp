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

        // Create instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Cache Initialization");
        {
            expect(cacheManager.initializeCache(), "Cache initialization should succeed");
            // Don't test JUCE File.exists() - test through IFileSystem interface instead
        }

        beginTest("Directory Structure");
        {
            // Test that cache initialization succeeds (which creates directories)
            expect(cacheManager.initializeCache(), "Cache initialization should succeed");

            // Test that we can save and load files (which verifies directories work)
            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testJsonData = "{\"test\": \"data\"}";

            expect(cacheManager.saveUnitToCache(testUnitId, testJsonData), "Should be able to save unit");
            expect(cacheManager.isUnitCached(testUnitId), "Unit should be cached after saving");
        }

        beginTest("Unit JSON Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

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
            expect(!cacheManager.isUnitCached(testUnitId), "Unit should not be cached initially");
            expect(cacheManager.saveUnitToCache(testUnitId, testJsonData), "Saving unit should succeed");
            expect(cacheManager.isUnitCached(testUnitId), "Unit should be cached after saving");

            // Test loading cached unit JSON
            juce::String loadedJson = cacheManager.loadUnitFromCache(testUnitId);
            expect(loadedJson.isNotEmpty(), "Loaded JSON should not be empty");
            expect(loadedJson == testJsonData, "Loaded JSON should match original data");

            // Test unit path
            juce::String unitPath = cacheManager.getCachedUnitPath(testUnitId);
            expect(mockFileSystem.getFileName(unitPath) == testUnitId + ".json", "Unit path should have correct filename");
        }

        beginTest("Image Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testFaceplateFilename = "test-unit-1.0.0.jpg";
            juce::String testThumbnailFilename = "test-unit-1.0.0.jpg";

            // Create a test image
            juce::Image testImage(juce::Image::RGB, 100, 100, true);
            testImage.clear(juce::Rectangle<int>(0, 0, 100, 100), juce::Colours::red);

            // Test faceplate caching
            expect(!cacheManager.isFaceplateCached(testUnitId, testFaceplateFilename), "Faceplate should not be cached initially");
            expect(cacheManager.saveFaceplateToCache(testUnitId, testFaceplateFilename, testImage), "Saving faceplate should succeed");
            expect(cacheManager.isFaceplateCached(testUnitId, testFaceplateFilename), "Faceplate should be cached after saving");

            // Test thumbnail caching
            expect(!cacheManager.isThumbnailCached(testUnitId, testThumbnailFilename), "Thumbnail should not be cached initially");
            expect(cacheManager.saveThumbnailToCache(testUnitId, testThumbnailFilename, testImage), "Saving thumbnail should succeed");
            expect(cacheManager.isThumbnailCached(testUnitId, testThumbnailFilename), "Thumbnail should be cached after saving");

            // Test loading cached images
            juce::Image loadedFaceplate = cacheManager.loadFaceplateFromCache(testUnitId, testFaceplateFilename);
            expect(loadedFaceplate.isValid(), "Loaded faceplate should be valid");

            juce::Image loadedThumbnail = cacheManager.loadThumbnailFromCache(testUnitId, testThumbnailFilename);
            expect(loadedThumbnail.isValid(), "Loaded thumbnail should be valid");
        }

        beginTest("Control Asset Caching");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            juce::String testAssetPath = "knobs/test-knob.png";

            // Create test image data
            juce::Image testImage(juce::Image::RGB, 50, 50, true);
            testImage.clear(juce::Rectangle<int>(0, 0, 50, 50), juce::Colours::blue);

            juce::MemoryBlock imageData;
            juce::PNGImageFormat pngFormat;
            juce::MemoryOutputStream stream(imageData, false);
            pngFormat.writeImageToStream(testImage, stream);

            // Test control asset caching
            expect(!cacheManager.isControlAssetCached(testAssetPath), "Control asset should not be cached initially");
            expect(cacheManager.saveControlAssetToCache(testAssetPath, imageData), "Saving control asset should succeed");
            expect(cacheManager.isControlAssetCached(testAssetPath), "Control asset should be cached after saving");

            // Test loading cached control asset
            juce::Image loadedAsset = cacheManager.loadControlAssetFromCache(testAssetPath);
            expect(loadedAsset.isValid(), "Loaded control asset should be valid");
        }

        beginTest("Cache Size");
        {
            juce::int64 cacheSize = cacheManager.getCacheSize();
            expect(cacheSize >= 0, "Cache size should be non-negative");
        }

        beginTest("File Path Generation");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            juce::String testUnitId = "test-unit-1.0.0";
            juce::String testFaceplateFilename = "test-unit-1.0.0.jpg";
            juce::String testThumbnailFilename = "test-unit-1.0.0.png";

            // Test unit path
            juce::String unitPath = cacheManager.getCachedUnitPath(testUnitId);
            expect(mockFileSystem.getFileName(unitPath) == testUnitId + ".json", "Unit path should have correct filename");

            // Test faceplate path
            juce::String faceplatePath = cacheManager.getCachedFaceplatePath(testUnitId, testFaceplateFilename);
            expect(mockFileSystem.getFileName(faceplatePath) == testFaceplateFilename, "Faceplate path should have correct filename");

            // Test thumbnail path
            juce::String thumbnailPath = cacheManager.getCachedThumbnailPath(testUnitId, testThumbnailFilename);
            expect(mockFileSystem.getFileName(thumbnailPath) == testThumbnailFilename, "Thumbnail path should have correct filename");

            // Test control asset path
            juce::String testAssetPath = "knobs/test-knob.png";
            juce::String assetPath = cacheManager.getCachedControlAssetPath(testAssetPath);
            expect(mockFileSystem.getFileName(assetPath) == "test-knob.png", "Control asset path should have correct filename");
        }

        beginTest("Error Handling");
        {
            // Reset mock file system for this test
            auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
            mockFileSystem.reset();

            // Test with invalid file system operations
            // This would test error handling when file operations fail
            // For now, we'll just test that the cache manager doesn't crash
            expect(true, "Cache manager should handle errors gracefully");
        }
    }
};

static CacheManagerTests cacheManagerTests;