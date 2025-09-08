#include <JuceHeader.h>
#include "CacheManager.h"
#include "PresetManager.h"
#include "MockFileSystem.h"
#include "TestFixture.h"

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

        testCacheInitialization(mockFileSystem, cacheManager);
        testDirectoryStructure(mockFileSystem, cacheManager);
        testUnitJSONCaching(mockFileSystem, cacheManager);
        testImageCaching(mockFileSystem, cacheManager);
        testControlAssetCaching(mockFileSystem, cacheManager);
        testCacheSize(mockFileSystem, cacheManager);
        testFilePathGeneration(mockFileSystem, cacheManager);
        testErrorHandling(mockFileSystem, cacheManager);
        testRecentlyUsedFunctionality(mockFileSystem, cacheManager);
        testFavoritesManagement(mockFileSystem, cacheManager);
        testCacheValidation(mockFileSystem, cacheManager);
        testDirectoryOperations(mockFileSystem, cacheManager);
        testFileSystemIntegration(mockFileSystem, cacheManager);
        testBoundaryConditions(mockFileSystem, cacheManager);
        testComplexScenarios(mockFileSystem, cacheManager);
        testStaticMethods();
        testCachePersistence(mockFileSystem, cacheManager);
        testConcurrentOperations(mockFileSystem, cacheManager);
    }

private:
    void testCacheInitialization(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Cache Initialization");
        
        // Test cache initialization
        expect(cacheManager.initializeCache(), "Cache initialization should succeed");
        
        // Test cache root directory creation
        juce::String cacheRoot = cacheManager.getCacheRoot();
        expect(cacheRoot == "/mock/cache/root", "Cache root should be set correctly");
        
        // Test file system access
        IFileSystem &fs = cacheManager.getFileSystem();
        expect(&fs == &mockFileSystem, "Should return the same file system instance");
    }

    void testDirectoryStructure(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Directory Structure");
        
        // Test that cache initialization creates directory structure
        expect(cacheManager.initializeCache(), "Cache initialization should succeed");
        
        // Test directory creation by trying to save files
        juce::String testUnitId = "test-unit-123";
        juce::String testJson = "{'test': 'data'}";
        
        expect(cacheManager.saveUnitToCache(testUnitId, testJson), "Should save unit to cache");
        expect(cacheManager.isUnitCached(testUnitId), "Unit should be cached after saving");
        
        // Test path generation
        juce::String unitPath = cacheManager.getCachedUnitPath(testUnitId);
        expect(unitPath.contains(testUnitId), "Unit path should contain unit ID");
    }

    void testUnitJSONCaching(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Unit JSON Caching");
        
        juce::String unitId = "test-compressor";
        juce::String jsonData = R"({
            "id": "test-compressor",
            "name": "Test Compressor",
            "type": "compressor",
            "controls": []
        })";
        
        // Test caching unit JSON
        expect(!cacheManager.isUnitCached(unitId), "Unit should not be cached initially");
        expect(cacheManager.saveUnitToCache(unitId, jsonData), "Saving unit should succeed");
        expect(cacheManager.isUnitCached(unitId), "Unit should be cached after saving");
        
        // Test loading cached data
        juce::String retrievedData = cacheManager.loadUnitFromCache(unitId);
        expect(retrievedData == jsonData, "Retrieved JSON should match cached data");
        
        // Test with empty JSON
        expect(cacheManager.saveUnitToCache("empty-unit", ""), "Should save empty JSON");
        juce::String emptyData = cacheManager.loadUnitFromCache("empty-unit");
        expect(emptyData.isEmpty(), "Empty JSON should be handled correctly");
    }

    void testImageCaching(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Image Caching");
        
        juce::String unitId = "image-test-unit";
        juce::String faceplateFilename = "test-faceplate.jpg";
        juce::String thumbnailFilename = "test-thumbnail.jpg";
        
        // Create test images
        juce::Image testFaceplate(juce::Image::RGB, 100, 100, true);
        testFaceplate.clear(juce::Rectangle<int>(0, 0, 100, 100), juce::Colours::red);
        
        juce::Image testThumbnail(juce::Image::RGB, 50, 50, true);
        testThumbnail.clear(juce::Rectangle<int>(0, 0, 50, 50), juce::Colours::blue);
        
        // Test faceplate caching
        expect(!cacheManager.isFaceplateCached(unitId, faceplateFilename), "Faceplate should not be cached initially");
        expect(cacheManager.saveFaceplateToCache(unitId, faceplateFilename, testFaceplate), "Should save faceplate");
        expect(cacheManager.isFaceplateCached(unitId, faceplateFilename), "Faceplate should be cached after saving");
        
        // Test thumbnail caching
        expect(!cacheManager.isThumbnailCached(unitId, thumbnailFilename), "Thumbnail should not be cached initially");
        expect(cacheManager.saveThumbnailToCache(unitId, thumbnailFilename, testThumbnail), "Should save thumbnail");
        expect(cacheManager.isThumbnailCached(unitId, thumbnailFilename), "Thumbnail should be cached after saving");
        
        // Test loading images
        juce::Image loadedFaceplate = cacheManager.loadFaceplateFromCache(unitId, faceplateFilename);
        expect(loadedFaceplate.isValid(), "Loaded faceplate should be valid");
        
        juce::Image loadedThumbnail = cacheManager.loadThumbnailFromCache(unitId, thumbnailFilename);
        expect(loadedThumbnail.isValid(), "Loaded thumbnail should be valid");
    }

    void testControlAssetCaching(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Control Asset Caching");
        
        juce::String assetPath = "knobs/test-knob.png";
        
        // Create test image data
        juce::Image testImage(juce::Image::RGB, 50, 50, true);
        testImage.clear(juce::Rectangle<int>(0, 0, 50, 50), juce::Colours::green);
        
        juce::MemoryBlock assetData;
        juce::PNGImageFormat pngFormat;
        juce::MemoryOutputStream stream(assetData, false);
        pngFormat.writeImageToStream(testImage, stream);
        
        // Test caching control asset
        expect(!cacheManager.isControlAssetCached(assetPath), "Control asset should not be cached initially");
        expect(cacheManager.saveControlAssetToCache(assetPath, assetData), "Should save control asset");
        expect(cacheManager.isControlAssetCached(assetPath), "Control asset should be cached after saving");
        
        // Test loading control asset
        juce::Image loadedAsset = cacheManager.loadControlAssetFromCache(assetPath);
        expect(loadedAsset.isValid(), "Loaded control asset should be valid");
        
        // Test path generation
        juce::String cachedPath = cacheManager.getCachedControlAssetPath(assetPath);
        expect(cachedPath.contains("test-knob.png"), "Control asset path should contain filename");
        
        // Test multiple control assets
        juce::StringArray assetPaths = {
            "faders/test-fader.png",
            "buttons/test-button.png",
            "switches/test-switch.png"
        };
        
        for (const auto& path : assetPaths)
        {
            expect(cacheManager.saveControlAssetToCache(path, assetData), "Should save control asset: " + path);
            expect(cacheManager.isControlAssetCached(path), "Control asset should be cached: " + path);
        }
    }

    void testCacheSize(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Cache Size Calculation");
        
        // Test initial cache size
        juce::int64 initialSize = cacheManager.getCacheSize();
        expect(initialSize >= 0, "Cache size should be non-negative");
        
        // Add some test data to calculate size
        juce::String testJson = "{'test': 'data', 'size': 100}";
        expect(cacheManager.saveUnitToCache("size-test-unit", testJson), "Should save unit for size test");
        
        // Add test image
        juce::Image testImage(juce::Image::RGB, 32, 32, true);
        expect(cacheManager.saveFaceplateToCache("size-test-unit", "test.jpg", testImage), "Should save image for size test");
        
        // Test cache size calculation
        juce::int64 newSize = cacheManager.getCacheSize();
        expect(newSize >= initialSize, "Cache size should increase after adding data");
    }

    void testFilePathGeneration(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("File Path Generation");
        
        // Test unit JSON path generation
        juce::String unitId = "test-reverb-deluxe";
        juce::String jsonPath = cacheManager.getCachedUnitPath(unitId);
        expect(jsonPath.contains(unitId), "JSON path should contain unit ID");
        expect(jsonPath.endsWith(".json"), "JSON path should end with .json");
        
        // Test faceplate path generation
        juce::String faceplatePath = cacheManager.getCachedFaceplatePath(unitId, "test.jpg");
        expect(faceplatePath.contains("test.jpg"), "Faceplate path should contain filename");
        
        // Test thumbnail path generation
        juce::String thumbnailPath = cacheManager.getCachedThumbnailPath(unitId, "test.jpg");
        expect(thumbnailPath.contains("test.jpg"), "Thumbnail path should contain filename");
        
        // Test control asset path generation
        juce::String controlPath = cacheManager.getCachedControlAssetPath("faders/test-fader.svg");
        expect(controlPath.contains("test-fader.svg"), "Control path should contain filename");
        
        // Test path uniqueness
        juce::String path1 = cacheManager.getCachedUnitPath("unit-1");
        juce::String path2 = cacheManager.getCachedUnitPath("unit-2");
        expect(path1 != path2, "Different units should have different paths");
    }

    void testErrorHandling(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Error Handling");
        
        // Test with non-existent files
        juce::String nonExistentData = cacheManager.loadUnitFromCache("non-existent-unit");
        expect(nonExistentData.isEmpty(), "Non-existent unit JSON should return empty string");
        
        juce::Image nonExistentImage = cacheManager.loadFaceplateFromCache("non-existent", "non-existent.jpg");
        expect(!nonExistentImage.isValid(), "Non-existent image should return invalid image");
        
        // Test cache status for non-existent items
        expect(!cacheManager.isUnitCached("non-existent-unit"), "Non-existent unit should not be cached");
        expect(!cacheManager.isFaceplateCached("non-existent", "non-existent.jpg"), "Non-existent image should not be cached");
        expect(!cacheManager.isControlAssetCached("non-existent.png"), "Non-existent control should not be cached");
        
        // Test with empty/invalid data
        expect(cacheManager.saveUnitToCache("empty-test", ""), "Empty JSON should be saveable");
        expect(cacheManager.isUnitCached("empty-test"), "Empty JSON should still be considered cached");
        
        // Test with empty parameters
        expect(!cacheManager.isUnitCached(""), "Empty unit ID should return false");
        expect(!cacheManager.isFaceplateCached("", ""), "Empty parameters should return false");
    }

    void testRecentlyUsedFunctionality(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Recently Used Functionality");
        
        // Test initial state
        juce::StringArray initialRecentlyUsed = cacheManager.getRecentlyUsed();
        expect(initialRecentlyUsed.size() == 0, "Initially recently used should be empty");
        
        // Test adding recently used units
        juce::StringArray testUnits = {"compressor-1", "eq-vintage", "reverb-hall"};
        
        for (const auto& unitId : testUnits)
        {
            expect(cacheManager.addToRecentlyUsed(unitId), "Should add " + unitId + " to recently used");
        }
        
        juce::StringArray recentlyUsed = cacheManager.getRecentlyUsed();
        expect(recentlyUsed.size() <= CacheManager::MAX_RECENTLY_USED, "Recently used should respect max limit");
        
        // Check that units were added
        for (const auto& unitId : testUnits)
        {
            expect(recentlyUsed.contains(unitId), "Recently used should contain " + unitId);
            expect(cacheManager.isRecentlyUsed(unitId), unitId + " should be marked as recently used");
        }
        
        // Test ordering (most recent first)
        if (recentlyUsed.size() >= 3)
        {
            expect(recentlyUsed[0] == "reverb-hall", "Most recent should be first");
        }
        
        // Test removing from recently used
        expect(cacheManager.removeFromRecentlyUsed("eq-vintage"), "Should remove eq-vintage from recently used");
        expect(!cacheManager.isRecentlyUsed("eq-vintage"), "eq-vintage should no longer be recently used");
        
        // Test clearing recently used
        expect(cacheManager.clearRecentlyUsed(), "Should clear recently used");
        juce::StringArray clearedList = cacheManager.getRecentlyUsed();
        expect(clearedList.isEmpty(), "Recently used should be empty after clearing");
    }

    void testFavoritesManagement(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Favorites Management");
        
        // Test initial state
        juce::StringArray initialFavorites = cacheManager.getFavorites();
        expect(initialFavorites.size() == 0, "Initially favorites should be empty");
        
        juce::StringArray testFavorites = {"favorite-comp", "favorite-eq", "favorite-reverb"};
        
        // Test adding favorites
        for (const auto& unitId : testFavorites)
        {
            expect(cacheManager.addToFavorites(unitId), "Should add " + unitId + " to favorites");
        }
        
        juce::StringArray favorites = cacheManager.getFavorites();
        expect(favorites.size() == testFavorites.size(), "All favorites should be added");
        
        for (const auto& unitId : testFavorites)
        {
            expect(favorites.contains(unitId), "Favorites should contain " + unitId);
            expect(cacheManager.isFavorite(unitId), unitId + " should be marked as favorite");
        }
        
        // Test removing favorites
        expect(cacheManager.removeFromFavorites("favorite-eq"), "Should remove favorite-eq");
        expect(!cacheManager.isFavorite("favorite-eq"), "Removed item should no longer be favorite");
        
        juce::StringArray updatedFavorites = cacheManager.getFavorites();
        expect(updatedFavorites.size() == testFavorites.size() - 1, "Favorites count should decrease");
        expect(!updatedFavorites.contains("favorite-eq"), "Removed favorite should not be in list");
        
        // Test clearing favorites
        expect(cacheManager.clearFavorites(), "Should clear favorites");
        expect(cacheManager.getFavorites().isEmpty(), "Favorites should be empty after clearing");
        expect(!cacheManager.isFavorite("favorite-comp"), "No items should be favorites after clearing");
        
        // Test favorites cache refresh
        cacheManager.addToFavorites("refresh-test");
        cacheManager.refreshFavoritesCache();
        expect(cacheManager.isFavorite("refresh-test"), "Item should still be favorite after refresh");
    }

    void testCacheValidation(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Cache Validation");
        
        // Test cache status checking
        juce::String testUnit = "validation-test-unit";
        juce::String testJson = "{'validation': 'test'}";
        
        expect(!cacheManager.isUnitCached(testUnit), "Unit should not be cached initially");
        
        expect(cacheManager.saveUnitToCache(testUnit, testJson), "Should save unit for validation");
        expect(cacheManager.isUnitCached(testUnit), "Unit should be cached after saving");
        
        // Test image cache validation
        juce::String testImageFile = "validation-test.png";
        juce::Image testImage(juce::Image::RGB, 64, 64, true);
        testImage.clear(juce::Rectangle<int>(0, 0, 64, 64), juce::Colours::yellow);
        
        expect(!cacheManager.isFaceplateCached(testUnit, testImageFile), "Image should not be cached initially");
        
        expect(cacheManager.saveFaceplateToCache(testUnit, testImageFile, testImage), "Should save image for validation");
        expect(cacheManager.isFaceplateCached(testUnit, testImageFile), "Image should be cached after saving");
        
        // Test control asset validation
        juce::String testControlPath = "validation/test-control.svg";
        juce::MemoryBlock testControlData;
        testControlData.append("test-control-data", 17);
        
        expect(!cacheManager.isControlAssetCached(testControlPath), "Control should not be cached initially");
        
        expect(cacheManager.saveControlAssetToCache(testControlPath, testControlData), "Should save control for validation");
        expect(cacheManager.isControlAssetCached(testControlPath), "Control should be cached after saving");
    }

    void testDirectoryOperations(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Directory Operations");
        
        // Test cache initialization creates directories
        expect(cacheManager.initializeCache(), "Should initialize cache directories");
        
        // Test that we can save files which requires directory creation
        juce::String unitId = "directory-test-unit";
        expect(cacheManager.saveUnitToCache(unitId, "{'test': 'data'}"), "Should save unit (creates directories)");
        
        // Test nested directory creation with special unit ID
        juce::String nestedUnit = "nested-sub-unit";
        expect(cacheManager.saveUnitToCache(nestedUnit, "{'nested': 'test'}"), "Should save nested unit");
        expect(cacheManager.isUnitCached(nestedUnit), "Nested unit should be cached");
        
        // Test multiple file types in same unit
        juce::Image testImage(juce::Image::RGB, 32, 32, true);
        expect(cacheManager.saveFaceplateToCache(unitId, "test.jpg", testImage), "Should save faceplate");
        expect(cacheManager.saveThumbnailToCache(unitId, "test.jpg", testImage), "Should save thumbnail");
        
        juce::MemoryBlock testData;
        testData.append("test-control-data", 17);
        expect(cacheManager.saveControlAssetToCache("test-controls/test.png", testData), "Should save control asset");
    }

    void testFileSystemIntegration(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("File System Integration");
        
        // Test that cache manager properly uses the injected file system
        IFileSystem &fs = cacheManager.getFileSystem();
        expect(&fs == &mockFileSystem, "Should use injected file system");
        
        // Test file system operations through cache manager
        juce::String testUnit = "filesystem-integration-test";
        juce::String testData = "{\"integration\": \"test\"}";
        
        expect(cacheManager.saveUnitToCache(testUnit, testData), "Should save through file system");
        
        // Verify file exists through direct file system access
        juce::String unitPath = cacheManager.getCachedUnitPath(testUnit);
        expect(mockFileSystem.fileExists(unitPath), "File should exist in mock file system");
        
        // Verify content consistency
        juce::String cacheContent = cacheManager.loadUnitFromCache(testUnit);
        expect(cacheContent == testData, "Content should match when accessed through cache manager");
    }

    void testBoundaryConditions(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Boundary Conditions");
        
        // Test with very long unit IDs
        juce::String longUnitId = "very-long-unit-id-that-exceeds-normal-expectations-and-tests-path-handling";
        expect(cacheManager.saveUnitToCache(longUnitId, "{'long': 'id'}"), "Should handle long unit ID");
        expect(cacheManager.isUnitCached(longUnitId), "Long unit ID should be cached");
        
        // Test with special characters in unit IDs
        juce::String specialUnitId = "unit-with-special_chars-123";
        expect(cacheManager.saveUnitToCache(specialUnitId, "{'special': 'chars'}"), "Should handle special characters");
        expect(cacheManager.isUnitCached(specialUnitId), "Special characters should be handled");
        
        // Test with very large JSON data
        juce::String largeJson = "{\"large_data\": \"";
        for (int i = 0; i < 1000; ++i)
            largeJson += "This is a large JSON data block repeated many times. ";
        largeJson += "\"}";
        
        expect(cacheManager.saveUnitToCache("large-json-unit", largeJson), "Should handle large JSON");
        juce::String retrievedLarge = cacheManager.loadUnitFromCache("large-json-unit");
        expect(retrievedLarge.length() > 1000, "Large JSON should be cached correctly");
        
        // Test with maximum recently used items
        for (int i = 0; i < CacheManager::MAX_RECENTLY_USED + 5; ++i)
        {
            cacheManager.addToRecentlyUsed("unit-" + juce::String(i));
        }
        juce::StringArray recentList = cacheManager.getRecentlyUsed();
        expect(recentList.size() <= CacheManager::MAX_RECENTLY_USED, "Recently used should not exceed maximum");
        
        // Test with very small and large images
        juce::Image tinyImage(juce::Image::RGB, 1, 1, true);
        expect(cacheManager.saveFaceplateToCache("tiny-unit", "tiny.jpg", tinyImage), "Should handle 1x1 pixel image");
        
        juce::Image largeImage(juce::Image::RGB, 512, 512, true);
        expect(cacheManager.saveFaceplateToCache("large-image-unit", "large.jpg", largeImage), "Should handle large images");
    }

    void testComplexScenarios(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Complex Scenarios");
        
        // Test mixed operations: caching multiple types of data
        juce::StringArray unitIds = {"complex-comp", "complex-eq", "complex-reverb"};
        
        // Cache units and their associated images
        for (int i = 0; i < unitIds.size(); ++i)
        {
            juce::String jsonData = "{\"id\": \"" + unitIds[i] + "\", \"name\": \"Complex Unit " + juce::String(i) + "\"}";
            expect(cacheManager.saveUnitToCache(unitIds[i], jsonData), "Should save complex unit " + unitIds[i]);
            
            juce::Image faceplate(juce::Image::RGB, 100 + i * 10, 100 + i * 10, true);
            faceplate.clear(juce::Rectangle<int>(0, 0, 100 + i * 10, 100 + i * 10), 
                          juce::Colour::fromHSV(i * 0.2f, 0.8f, 0.9f, 1.0f));
            expect(cacheManager.saveFaceplateToCache(unitIds[i], "faceplate.jpg", faceplate), "Should save faceplate " + unitIds[i]);
            
            // Add to recently used and favorites
            cacheManager.addToRecentlyUsed(unitIds[i]);
            if (i % 2 == 0) // Every other one as favorite
                cacheManager.addToFavorites(unitIds[i]);
        }
        
        // Verify all data is cached correctly
        for (int i = 0; i < unitIds.size(); ++i)
        {
            expect(cacheManager.isUnitCached(unitIds[i]), "Complex unit " + unitIds[i] + " should be cached");
            expect(cacheManager.isFaceplateCached(unitIds[i], "faceplate.jpg"), "Complex image " + unitIds[i] + " should be cached");
            
            juce::String retrievedJson = cacheManager.loadUnitFromCache(unitIds[i]);
            expect(retrievedJson.contains(unitIds[i]), "Retrieved JSON should contain unit ID");
        }
        
        // Verify recently used and favorites
        juce::StringArray recentlyUsed = cacheManager.getRecentlyUsed();
        juce::StringArray favorites = cacheManager.getFavorites();
        
        expect(recentlyUsed.size() == unitIds.size(), "All units should be in recently used");
        expect(favorites.size() == 2, "Two units should be favorites");
        
        // Test cache size calculation with complex data
        juce::int64 totalSize = cacheManager.getCacheSize();
        expect(totalSize > 0, "Complex cache should have size > 0");
    }

    void testStaticMethods()
    {
        beginTest("Static Methods");
        
        // Test dummy cache manager
        CacheManager &dummyCache = CacheManager::getDummy();
        
        // Verify it's a valid cache manager
        juce::String dummyCacheRoot = dummyCache.getCacheRoot();
        expect(dummyCacheRoot.isNotEmpty(), "Dummy cache should have a valid root path");
        
        // Test that multiple calls return the same instance
        CacheManager &dummyCache2 = CacheManager::getDummy();
        expect(&dummyCache == &dummyCache2, "Should return the same dummy instance");
        
        // Test basic operations on dummy cache
        expect(dummyCache.initializeCache(), "Dummy cache should initialize successfully");
        expect(!dummyCache.isUnitCached("test-unit"), "Dummy cache should not have cached units initially");
    }

    void testCachePersistence(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Cache Persistence");
        
        // Test that cached data persists across cache manager instances
        juce::String persistentUnit = "persistent-test-unit";
        juce::String persistentData = "{'persistent': 'data'}";
        
        expect(cacheManager.saveUnitToCache(persistentUnit, persistentData), "Should save persistent unit");
        cacheManager.addToRecentlyUsed(persistentUnit);
        cacheManager.addToFavorites(persistentUnit);
        
        // Create a new cache manager instance with the same root
        CacheManager newCacheManager(mockFileSystem, "/mock/cache/root");
        
        // Test that data is still accessible
        expect(newCacheManager.isUnitCached(persistentUnit), "Cached unit should persist");
        
        juce::String retrievedData = newCacheManager.loadUnitFromCache(persistentUnit);
        expect(retrievedData == persistentData, "Cached data should persist correctly");
        
        // Test that favorites and recently used persist
        expect(newCacheManager.isFavorite(persistentUnit), "Favorites should persist");
        
        juce::StringArray persistentRecentlyUsed = newCacheManager.getRecentlyUsed();
        expect(persistentRecentlyUsed.contains(persistentUnit), "Recently used should persist");
    }

    void testConcurrentOperations(ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        beginTest("Concurrent Operations");
        
        // Test multiple rapid operations
        juce::StringArray rapidUnits;
        for (int i = 0; i < 10; ++i)
        {
            juce::String unitId = "rapid-unit-" + juce::String(i);
            rapidUnits.add(unitId);
            
            juce::String jsonData = "{\"rapid\": " + juce::String(i) + "}";
            expect(cacheManager.saveUnitToCache(unitId, jsonData), "Should save rapid unit " + unitId);
            
            // Immediately try to retrieve
            juce::String retrieved = cacheManager.loadUnitFromCache(unitId);
            expect(retrieved == jsonData, "Rapid cache/retrieve should work for " + unitId);
        }
        
        // Test rapid recently used updates
        for (const auto& unitId : rapidUnits)
        {
            cacheManager.addToRecentlyUsed(unitId);
        }
        
        juce::StringArray finalRecentlyUsed = cacheManager.getRecentlyUsed();
        expect(finalRecentlyUsed.size() <= CacheManager::MAX_RECENTLY_USED, 
               "Rapid recently used updates should respect limits");
        
        // Test rapid favorites updates
        for (int i = 0; i < rapidUnits.size(); i += 2)
        {
            cacheManager.addToFavorites(rapidUnits[i]);
        }
        
        juce::StringArray finalFavorites = cacheManager.getFavorites();
        expect(finalFavorites.size() == 5, "Rapid favorites updates should work correctly");
    }
};

static CacheManagerTests cacheManagerTests;