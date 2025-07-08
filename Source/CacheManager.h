#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "IFileSystem.h"
#include "FileSystem.h"

/**
 * @brief Manages local caching of unit data and assets for the Analogiq plugin.
 *
 * This singleton class handles caching of unit JSON definitions, faceplate images,
 * thumbnails, and control assets to improve performance and enable offline usage.
 * The cache is stored in the user's application data directory and mirrors the
 * remote structure for consistency.
 */
class CacheManager
{
public:
    /**
     * @brief Maximum number of recently used items that can be stored and displayed.
     */
    static constexpr int MAX_RECENTLY_USED = 20;

    /**
     * @brief Gets the singleton instance of CacheManager.
     *
     * @return Reference to the CacheManager instance
     */
    static CacheManager &getInstance();

    /**
     * @brief Gets a dummy instance for testing and default initialization.
     *
     * @return Reference to a dummy CacheManager instance
     */
    static CacheManager &getDummy();

    /**
     * @brief Resets the singleton instance with a new file system (for testing).
     *
     * @param fileSystem The file system implementation to use
     * @param cacheRootPath Optional custom cache root path (for testing)
     */
    static void resetInstance(IFileSystem &fileSystem, const juce::String &cacheRootPath = "");

    /**
     * @brief Destructor.
     */
    ~CacheManager() = default;

    // Prevent copying and assignment
    CacheManager(const CacheManager &) = delete;
    CacheManager &operator=(const CacheManager &) = delete;

    /**
     * @brief Initializes the cache directory structure.
     *
     * Creates the cache root directory and all necessary subdirectories
     * if they don't already exist.
     *
     * @return true if initialization was successful, false otherwise
     */
    bool initializeCache();

    /**
     * @brief Gets the cache root directory path.
     *
     * @return The cache root directory path as a string
     */
    juce::String getCacheRoot() const;

    /**
     * @brief Gets the file system used by this cache manager.
     *
     * @return Reference to the file system implementation
     */
    IFileSystem &getFileSystem() const { return fileSystem; }

    /**
     * @brief Checks if a unit JSON file is cached locally.
     *
     * @param unitId The unit identifier (e.g., "la2a-compressor-1.0.0")
     * @return true if the unit JSON is cached, false otherwise
     */
    bool isUnitCached(const juce::String &unitId) const;

    /**
     * @brief Checks if a faceplate image is cached locally.
     *
     * @param unitId The unit identifier
     * @param filename The faceplate filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return true if the faceplate image is cached, false otherwise
     */
    bool isFaceplateCached(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Checks if a thumbnail image is cached locally.
     *
     * @param unitId The unit identifier
     * @param filename The thumbnail filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return true if the thumbnail image is cached, false otherwise
     */
    bool isThumbnailCached(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Checks if a control asset is cached locally.
     *
     * @param assetPath The relative path to the control asset (e.g., "knobs/bakelite-lg-black.png")
     * @return true if the control asset is cached, false otherwise
     */
    bool isControlAssetCached(const juce::String &assetPath) const;

    /**
     * @brief Gets the cached file path for a unit JSON.
     *
     * @param unitId The unit identifier
     * @return The cached file path as a string
     */
    juce::String getCachedUnitPath(const juce::String &unitId) const;

    /**
     * @brief Gets the cached file path for a faceplate image.
     *
     * @param unitId The unit identifier
     * @param filename The faceplate filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached file path as a string
     */
    juce::String getCachedFaceplatePath(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Gets the cached file path for a thumbnail image.
     *
     * @param unitId The unit identifier
     * @param filename The thumbnail filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached file path as a string
     */
    juce::String getCachedThumbnailPath(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Gets the cached file path for a control asset.
     *
     * @param assetPath The relative path to the control asset
     * @return The cached file path as a string
     */
    juce::String getCachedControlAssetPath(const juce::String &assetPath) const;

    /**
     * @brief Saves unit JSON data to the cache.
     *
     * @param unitId The unit identifier
     * @param jsonData The JSON data to cache
     * @return true if saving was successful, false otherwise
     */
    bool saveUnitToCache(const juce::String &unitId, const juce::String &jsonData);

    /**
     * @brief Saves a faceplate image to the cache.
     *
     * @param unitId The unit identifier
     * @param filename The faceplate filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @param image The image to cache
     * @return true if saving was successful, false otherwise
     */
    bool saveFaceplateToCache(const juce::String &unitId, const juce::String &filename, const juce::Image &image);

    /**
     * @brief Saves a thumbnail image to the cache.
     *
     * @param unitId The unit identifier
     * @param filename The thumbnail filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @param image The image to cache
     * @return true if saving was successful, false otherwise
     */
    bool saveThumbnailToCache(const juce::String &unitId, const juce::String &filename, const juce::Image &image);

    /**
     * @brief Saves a control asset to the cache.
     *
     * @param assetPath The relative path to the control asset
     * @param imageData The image data to cache
     * @return true if saving was successful, false otherwise
     */
    bool saveControlAssetToCache(const juce::String &assetPath, const juce::MemoryBlock &imageData);

    /**
     * @brief Loads unit JSON data from the cache.
     *
     * @param unitId The unit identifier
     * @return The cached JSON data, or empty string if not found
     */
    juce::String loadUnitFromCache(const juce::String &unitId) const;

    /**
     * @brief Loads a faceplate image from the cache.
     *
     * @param unitId The unit identifier
     * @param filename The faceplate filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached image, or invalid image if not found
     */
    juce::Image loadFaceplateFromCache(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Loads a thumbnail image from the cache.
     *
     * @param unitId The unit identifier
     * @param filename The thumbnail filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached image, or invalid image if not found
     */
    juce::Image loadThumbnailFromCache(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Loads a control asset from the cache.
     *
     * @param assetPath The relative path to the control asset
     * @return The cached image, or invalid image if not found
     */
    juce::Image loadControlAssetFromCache(const juce::String &assetPath) const;

    /**
     * @brief Clears all cached data.
     *
     * Removes all files and directories in the cache.
     *
     * @return true if clearing was successful, false otherwise
     */
    bool clearCache();

    /**
     * @brief Gets the total size of the cache in bytes.
     *
     * @return The cache size in bytes
     */
    juce::int64 getCacheSize() const;

    /**
     * @brief Adds a unit to the recently used list.
     *
     * @param unitId The unit identifier to add
     * @return true if the unit was added successfully, false otherwise
     */
    bool addToRecentlyUsed(const juce::String &unitId);

    /**
     * @brief Gets the list of recently used units.
     *
     * @param maxCount The maximum number of items to return (default: MAX_RECENTLY_USED)
     * @return Array of recently used unit identifiers
     */
    juce::StringArray getRecentlyUsed(int maxCount = MAX_RECENTLY_USED) const;

    /**
     * @brief Removes a unit from the recently used list.
     *
     * @param unitId The unit identifier to remove
     * @return true if the unit was removed successfully, false otherwise
     */
    bool removeFromRecentlyUsed(const juce::String &unitId);

    /**
     * @brief Clears the recently used list.
     *
     * @return true if clearing was successful, false otherwise
     */
    bool clearRecentlyUsed();

    /**
     * @brief Checks if a unit is in the recently used list.
     *
     * @param unitId The unit identifier to check
     * @return true if the unit is recently used, false otherwise
     */
    bool isRecentlyUsed(const juce::String &unitId) const;

    /**
     * @brief Adds a unit to the favorites list.
     *
     * @param unitId The unit identifier to add
     * @return true if the unit was added successfully, false otherwise
     */
    bool addToFavorites(const juce::String &unitId);

    /**
     * @brief Removes a unit from the favorites list.
     *
     * @param unitId The unit identifier to remove
     * @return true if the unit was removed successfully, false otherwise
     */
    bool removeFromFavorites(const juce::String &unitId);

    /**
     * @brief Clears the favorites list.
     *
     * @return true if clearing was successful, false otherwise
     */
    bool clearFavorites();

    /**
     * @brief Checks if a unit is in the favorites list.
     *
     * @param unitId The unit identifier to check
     * @return true if the unit is favorited, false otherwise
     */
    bool isFavorite(const juce::String &unitId) const;

    /**
     * @brief Gets the list of favorite units.
     *
     * @return Array of favorite unit identifiers
     */
    juce::StringArray getFavorites() const;

public:
    /**
     * @brief Constructor for CacheManager.
     *
     * @param fileSystem Reference to the file system implementation
     * @param cacheRootPath Optional custom cache root path (for testing)
     */
    CacheManager(IFileSystem &fileSystem, const juce::String &cacheRootPath = "");

private:
    IFileSystem &fileSystem;
    juce::String cacheRoot;

    // Directory path getters
    juce::String getUnitsDirectory() const;
    juce::String getAssetsDirectory() const;
    juce::String getFaceplatesDirectory() const;
    juce::String getThumbnailsDirectory() const;
    juce::String getControlsDirectory() const;

    /**
     * @brief Creates a directory if it doesn't exist.
     *
     * @param directoryPath The directory path to create
     * @return true if the directory was created or already exists, false otherwise
     */
    bool createDirectoryIfNeeded(const juce::String &directoryPath) const;

    /**
     * @brief Calculates the size of a directory recursively.
     *
     * @param directoryPath The directory path to calculate size for
     * @return The total size in bytes
     */
    juce::int64 calculateDirectorySize(const juce::String &directoryPath) const;
};