#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

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
     * @brief Gets the cache root directory.
     *
     * @return The cache root directory as a JUCE File object
     */
    juce::File getCacheRoot() const;

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
     * @return The cached file path, or empty string if not cached
     */
    juce::File getCachedUnitPath(const juce::String &unitId) const;

    /**
     * @brief Gets the cached file path for a faceplate image.
     *
     * @param unitId The unit identifier
     * @param filename The faceplate filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached file path, or empty string if not cached
     */
    juce::File getCachedFaceplatePath(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Gets the cached file path for a thumbnail image.
     *
     * @param unitId The unit identifier
     * @param filename The thumbnail filename (e.g., "la2a-compressor-1.0.0.jpg")
     * @return The cached file path, or empty string if not cached
     */
    juce::File getCachedThumbnailPath(const juce::String &unitId, const juce::String &filename) const;

    /**
     * @brief Gets the cached file path for a control asset.
     *
     * @param assetPath The relative path to the control asset
     * @return The cached file path, or empty string if not cached
     */
    juce::File getCachedControlAssetPath(const juce::String &assetPath) const;

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
     * Removes all files from the cache directory.
     *
     * @return true if clearing was successful, false otherwise
     */
    bool clearCache();

    /**
     * @brief Gets the total size of the cache in bytes.
     *
     * @return The total cache size in bytes
     */
    juce::int64 getCacheSize() const;

    // Recently Used functionality
    /**
     * @brief Adds a gear item to the recently used list.
     *
     * @param unitId The unit identifier to add to recently used
     * @return true if the operation was successful, false otherwise
     */
    bool addToRecentlyUsed(const juce::String &unitId);

    /**
     * @brief Gets the list of recently used unit IDs.
     *
     * @param maxCount Maximum number of items to return (default: MAX_RECENTLY_USED)
     * @return Array of recently used unit IDs, most recent first
     */
    juce::StringArray getRecentlyUsed(int maxCount = MAX_RECENTLY_USED) const;

    /**
     * @brief Removes a unit from the recently used list.
     *
     * @param unitId The unit identifier to remove
     * @return true if the operation was successful, false otherwise
     */
    bool removeFromRecentlyUsed(const juce::String &unitId);

    /**
     * @brief Clears the recently used list.
     *
     * @return true if the operation was successful, false otherwise
     */
    bool clearRecentlyUsed();

    /**
     * @brief Gets the number of items in the recently used list.
     *
     * @return The number of recently used items
     */
    int getRecentlyUsedCount() const;

    /**
     * @brief Checks if a unit is in the recently used list.
     *
     * @param unitId The unit identifier to check
     * @return true if the unit is in the recently used list, false otherwise
     */
    bool isRecentlyUsed(const juce::String &unitId) const;

    // Favorites functionality
    /**
     * @brief Adds a gear item to the favorites list.
     *
     * @param unitId The unit identifier to add to favorites
     * @return true if the operation was successful, false otherwise
     */
    bool addToFavorites(const juce::String &unitId);

    /**
     * @brief Gets the list of favorite unit IDs.
     *
     * @return Array of favorite unit IDs
     */
    juce::StringArray getFavorites() const;

    /**
     * @brief Removes a unit from the favorites list.
     *
     * @param unitId The unit identifier to remove
     * @return true if the operation was successful, false otherwise
     */
    bool removeFromFavorites(const juce::String &unitId);

    /**
     * @brief Clears the favorites list.
     *
     * @return true if the operation was successful, false otherwise
     */
    bool clearFavorites();

    /**
     * @brief Gets the number of items in the favorites list.
     *
     * @return The number of favorite items
     */
    int getFavoritesCount() const;

    /**
     * @brief Checks if a unit is in the favorites list.
     *
     * @param unitId The unit identifier to check
     * @return true if the unit is in the favorites list, false otherwise
     */
    bool isFavorite(const juce::String &unitId) const;

    // Presets directory management
    /**
     * @brief Gets the presets directory.
     *
     * @return The presets directory as a JUCE File object
     */
    juce::File getPresetsDirectory() const;

    /**
     * @brief Initializes the presets directory structure.
     *
     * Creates the presets directory if it doesn't exist.
     *
     * @return true if initialization was successful, false otherwise
     */
    bool initializePresetsDirectory() const;

private:
    /**
     * @brief Private constructor for singleton pattern.
     */
    CacheManager();

    /**
     * @brief Creates a directory if it doesn't exist.
     *
     * @param directory The directory to create
     * @return true if creation was successful or directory already exists, false otherwise
     */
    bool createDirectoryIfNeeded(const juce::File &directory) const;

    /**
     * @brief Gets the units directory.
     *
     * @return The units directory
     */
    juce::File getUnitsDirectory() const;

    /**
     * @brief Gets the assets directory.
     *
     * @return The assets directory
     */
    juce::File getAssetsDirectory() const;

    /**
     * @brief Gets the faceplates directory.
     *
     * @return The faceplates directory
     */
    juce::File getFaceplatesDirectory() const;

    /**
     * @brief Gets the thumbnails directory.
     *
     * @return The thumbnails directory
     */
    juce::File getThumbnailsDirectory() const;

    /**
     * @brief Gets the controls directory.
     *
     * @return The controls directory
     */
    juce::File getControlsDirectory() const;

    /**
     * @brief Calculates the total size of a directory recursively.
     *
     * @param directory The directory to calculate size for
     * @return The total size in bytes
     */
    juce::int64 calculateDirectorySize(const juce::File &directory) const;

    juce::File cacheRoot; ///< The cache root directory
};