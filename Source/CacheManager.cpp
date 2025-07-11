#include "CacheManager.h"
#include "FileSystem.h"
#include "IFileSystem.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

CacheManager::CacheManager(IFileSystem &fileSystem, const juce::String &cacheRootPath)
    : fileSystem(fileSystem)
{
    // Initialize cache root directory
    if (cacheRootPath.isNotEmpty())
    {
        // Use provided cache root path (for testing)
        cacheRoot = cacheRootPath;
    }
    else
    {
        // Use OS-agnostic approach through the injected fileSystem
        cacheRoot = fileSystem.getCacheRootDirectory();
    }
}

bool CacheManager::initializeCache()
{
    try
    {
        // Create all necessary directories
        if (!createDirectoryIfNeeded(cacheRoot))
            return false;

        if (!createDirectoryIfNeeded(getUnitsDirectory()))
            return false;

        if (!createDirectoryIfNeeded(getAssetsDirectory()))
            return false;

        if (!createDirectoryIfNeeded(getFaceplatesDirectory()))
            return false;

        if (!createDirectoryIfNeeded(getThumbnailsDirectory()))
            return false;

        if (!createDirectoryIfNeeded(getControlsDirectory()))
            return false;

        // Create subdirectories for different control types
        if (!createDirectoryIfNeeded(fileSystem.joinPath(getControlsDirectory(), "buttons")))
            return false;

        if (!createDirectoryIfNeeded(fileSystem.joinPath(getControlsDirectory(), "faders")))
            return false;

        if (!createDirectoryIfNeeded(fileSystem.joinPath(getControlsDirectory(), "knobs")))
            return false;

        if (!createDirectoryIfNeeded(fileSystem.joinPath(getControlsDirectory(), "switches")))
            return false;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::String CacheManager::getCacheRoot() const
{
    return cacheRoot;
}

bool CacheManager::isUnitCached(const juce::String &unitId) const
{
    juce::String unitFilePath = getCachedUnitPath(unitId);
    return fileSystem.fileExists(unitFilePath);
}

bool CacheManager::isFaceplateCached(const juce::String &unitId, const juce::String &filename) const
{
    juce::String faceplateFilePath = getCachedFaceplatePath(unitId, filename);
    return fileSystem.fileExists(faceplateFilePath);
}

bool CacheManager::isThumbnailCached(const juce::String &unitId, const juce::String &filename) const
{
    juce::String thumbnailFilePath = getCachedThumbnailPath(unitId, filename);
    return fileSystem.fileExists(thumbnailFilePath);
}

bool CacheManager::isControlAssetCached(const juce::String &assetPath) const
{
    juce::String assetFilePath = getCachedControlAssetPath(assetPath);
    return fileSystem.fileExists(assetFilePath);
}

juce::String CacheManager::getCachedUnitPath(const juce::String &unitId) const
{
    return fileSystem.joinPath(getUnitsDirectory(), unitId + ".json");
}

juce::String CacheManager::getCachedFaceplatePath(const juce::String &unitId, const juce::String &filename) const
{
    return fileSystem.joinPath(getFaceplatesDirectory(), filename);
}

juce::String CacheManager::getCachedThumbnailPath(const juce::String &unitId, const juce::String &filename) const
{
    return fileSystem.joinPath(getThumbnailsDirectory(), filename);
}

juce::String CacheManager::getCachedControlAssetPath(const juce::String &assetPath) const
{
    // Strip "assets/controls/" prefix if present to prevent redundant nesting
    juce::String cleanAssetPath = assetPath;
    if (cleanAssetPath.startsWith("assets/controls/"))
    {
        cleanAssetPath = cleanAssetPath.substring(16); // Remove "assets/controls/" prefix
    }
    else if (cleanAssetPath.startsWith("controls/"))
    {
        cleanAssetPath = cleanAssetPath.substring(9); // Remove "controls/" prefix
    }

    return fileSystem.joinPath(getControlsDirectory(), cleanAssetPath);
}

bool CacheManager::saveUnitToCache(const juce::String &unitId, const juce::String &jsonData)
{
    try
    {
        juce::String unitFilePath = getCachedUnitPath(unitId);

        // Ensure the units directory exists
        if (!createDirectoryIfNeeded(getUnitsDirectory()))
            return false;

        // Write the JSON data to file
        return fileSystem.writeFile(unitFilePath, jsonData);
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::saveFaceplateToCache(const juce::String &unitId, const juce::String &filename, const juce::Image &image)
{
    try
    {
        juce::String faceplateFilePath = getCachedFaceplatePath(unitId, filename);

        // Ensure the faceplates directory exists
        if (!createDirectoryIfNeeded(getFaceplatesDirectory()))
            return false;

        // Save the image as JPEG
        juce::JPEGImageFormat jpegFormat;
        juce::MemoryBlock imageData;
        juce::MemoryOutputStream stream(imageData, false);

        if (jpegFormat.writeImageToStream(image, stream))
        {
            return fileSystem.writeFile(faceplateFilePath, imageData);
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::saveThumbnailToCache(const juce::String &unitId, const juce::String &filename, const juce::Image &image)
{
    try
    {
        juce::String thumbnailFilePath = getCachedThumbnailPath(unitId, filename);

        // Ensure the thumbnails directory exists
        if (!createDirectoryIfNeeded(getThumbnailsDirectory()))
            return false;

        // Save the image as JPEG
        juce::JPEGImageFormat jpegFormat;
        juce::MemoryBlock imageData;
        juce::MemoryOutputStream stream(imageData, false);

        if (jpegFormat.writeImageToStream(image, stream))
        {
            return fileSystem.writeFile(thumbnailFilePath, imageData);
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::saveControlAssetToCache(const juce::String &assetPath, const juce::MemoryBlock &imageData)
{
    try
    {
        juce::String assetFilePath = getCachedControlAssetPath(assetPath);

        // Ensure the parent directory exists
        juce::String parentDir = fileSystem.getParentDirectory(assetFilePath);
        if (!createDirectoryIfNeeded(parentDir))
            return false;

        // Write the image data to file
        return fileSystem.writeFile(assetFilePath, imageData);
    }
    catch (...)
    {
        return false;
    }
}

juce::String CacheManager::loadUnitFromCache(const juce::String &unitId) const
{
    try
    {
        juce::String unitFilePath = getCachedUnitPath(unitId);

        if (fileSystem.fileExists(unitFilePath))
        {
            return fileSystem.readFile(unitFilePath);
        }

        return juce::String();
    }
    catch (...)
    {
        return juce::String();
    }
}

juce::Image CacheManager::loadFaceplateFromCache(const juce::String &unitId, const juce::String &filename) const
{
    try
    {
        juce::String faceplateFilePath = getCachedFaceplatePath(unitId, filename);

        if (fileSystem.fileExists(faceplateFilePath))
        {
            juce::MemoryBlock imageData = fileSystem.readBinaryFile(faceplateFilePath);
            if (imageData.getSize() > 0)
            {
                juce::MemoryInputStream stream(imageData, false);
                return juce::ImageFileFormat::loadFrom(stream);
            }
        }

        return juce::Image();
    }
    catch (...)
    {
        return juce::Image();
    }
}

juce::Image CacheManager::loadThumbnailFromCache(const juce::String &unitId, const juce::String &filename) const
{
    try
    {
        juce::String thumbnailFilePath = getCachedThumbnailPath(unitId, filename);

        if (fileSystem.fileExists(thumbnailFilePath))
        {
            juce::MemoryBlock imageData = fileSystem.readBinaryFile(thumbnailFilePath);
            if (imageData.getSize() > 0)
            {
                juce::MemoryInputStream stream(imageData, false);
                return juce::ImageFileFormat::loadFrom(stream);
            }
        }

        return juce::Image();
    }
    catch (...)
    {
        return juce::Image();
    }
}

juce::Image CacheManager::loadControlAssetFromCache(const juce::String &assetPath) const
{
    try
    {
        juce::String assetFilePath = getCachedControlAssetPath(assetPath);

        if (fileSystem.fileExists(assetFilePath))
        {
            juce::MemoryBlock imageData = fileSystem.readBinaryFile(assetFilePath);
            if (imageData.getSize() > 0)
            {
                juce::MemoryInputStream stream(imageData, false);
                return juce::ImageFileFormat::loadFrom(stream);
            }
        }

        return juce::Image();
    }
    catch (...)
    {
        return juce::Image();
    }
}

bool CacheManager::clearCache()
{
    try
    {
        if (fileSystem.directoryExists(cacheRoot))
        {
            return fileSystem.deleteDirectory(cacheRoot);
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::int64 CacheManager::getCacheSize() const
{
    try
    {
        if (fileSystem.directoryExists(cacheRoot))
        {
            return calculateDirectorySize(cacheRoot);
        }

        return 0;
    }
    catch (...)
    {
        return 0;
    }
}

juce::int64 CacheManager::calculateDirectorySize(const juce::String &directory) const
{
    juce::int64 totalSize = 0;

    if (fileSystem.directoryExists(directory))
    {
        // Get all files in the directory
        auto files = fileSystem.getFiles(directory);
        for (const auto &filename : files)
        {
            juce::String fullPath = directory + "/" + filename;
            totalSize += fileSystem.getFileSize(fullPath);
        }

        // Recursively calculate size of subdirectories
        auto subdirs = fileSystem.getDirectories(directory);
        for (const auto &dirname : subdirs)
        {
            juce::String fullPath = directory + "/" + dirname;
            totalSize += calculateDirectorySize(fullPath);
        }
    }

    return totalSize;
}

bool CacheManager::createDirectoryIfNeeded(const juce::String &directory) const
{
    try
    {
        if (!fileSystem.directoryExists(directory))
        {
            return fileSystem.createDirectory(directory);
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::String CacheManager::getUnitsDirectory() const
{
    return fileSystem.joinPath(cacheRoot, "units");
}

juce::String CacheManager::getAssetsDirectory() const
{
    return fileSystem.joinPath(cacheRoot, "assets");
}

juce::String CacheManager::getFaceplatesDirectory() const
{
    return fileSystem.joinPath(getAssetsDirectory(), "faceplates");
}

juce::String CacheManager::getThumbnailsDirectory() const
{
    return fileSystem.joinPath(getAssetsDirectory(), "thumbnails");
}

juce::String CacheManager::getControlsDirectory() const
{
    return fileSystem.joinPath(getAssetsDirectory(), "controls");
}

// Recently Used functionality
bool CacheManager::addToRecentlyUsed(const juce::String &unitId)
{
    try
    {
        juce::String recentlyUsedFilePath = fileSystem.joinPath(cacheRoot, "recently_used.json");

        // Load existing recently used list
        juce::StringArray recentlyUsed;
        if (fileSystem.fileExists(recentlyUsedFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(recentlyUsedFilePath));
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                for (const auto &item : *array)
                {
                    recentlyUsed.add(item.toString());
                }
            }
        }

        // Add the unit to the beginning of the list (most recent first)
        if (recentlyUsed.contains(unitId))
        {
            recentlyUsed.removeString(unitId);
        }
        recentlyUsed.insert(0, unitId);

        // Limit the list size
        if (recentlyUsed.size() > MAX_RECENTLY_USED)
        {
            recentlyUsed.removeRange(MAX_RECENTLY_USED, recentlyUsed.size() - MAX_RECENTLY_USED);
        }

        // Save the updated list
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        juce::Array<juce::var> array;
        for (const auto &item : recentlyUsed)
        {
            array.add(item);
        }
        jsonObj->setProperty("recentlyUsed", array);

        return fileSystem.writeFile(recentlyUsedFilePath, juce::JSON::toString(juce::var(jsonObj)));
    }
    catch (...)
    {
        return false;
    }
}

juce::StringArray CacheManager::getRecentlyUsed(int maxCount) const
{
    try
    {
        juce::String recentlyUsedFilePath = fileSystem.joinPath(cacheRoot, "recently_used.json");

        juce::StringArray recentlyUsed;

        if (fileSystem.fileExists(recentlyUsedFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(recentlyUsedFilePath));
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                for (const auto &item : *array)
                {
                    recentlyUsed.add(item.toString());
                }
            }
        }

        // Limit the returned list to maxCount
        if (recentlyUsed.size() > maxCount)
        {
            recentlyUsed.removeRange(maxCount, recentlyUsed.size() - maxCount);
        }

        return recentlyUsed;
    }
    catch (...)
    {
        return juce::StringArray();
    }
}

bool CacheManager::removeFromRecentlyUsed(const juce::String &unitId)
{
    try
    {
        juce::String recentlyUsedFilePath = fileSystem.joinPath(cacheRoot, "recently_used.json");

        // Load existing recently used list
        juce::StringArray recentlyUsed;
        if (fileSystem.fileExists(recentlyUsedFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(recentlyUsedFilePath));
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                for (const auto &item : *array)
                {
                    recentlyUsed.add(item.toString());
                }
            }
        }

        // Remove the unit
        bool wasRemoved = recentlyUsed.contains(unitId);
        if (wasRemoved)
        {
            recentlyUsed.removeString(unitId);
        }

        if (wasRemoved)
        {
            // Save the updated list
            juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
            juce::Array<juce::var> array;
            for (const auto &item : recentlyUsed)
            {
                array.add(item);
            }
            jsonObj->setProperty("recentlyUsed", array);

            return fileSystem.writeFile(recentlyUsedFilePath, juce::JSON::toString(juce::var(jsonObj)));
        }

        return true; // Unit wasn't in the list, so "successfully" removed
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::clearRecentlyUsed()
{
    try
    {
        juce::String recentlyUsedFilePath = fileSystem.joinPath(cacheRoot, "recently_used.json");

        if (fileSystem.fileExists(recentlyUsedFilePath))
        {
            return fileSystem.deleteFile(recentlyUsedFilePath);
        }

        return true; // File doesn't exist, so "successfully" cleared
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::isRecentlyUsed(const juce::String &unitId) const
{
    try
    {
        juce::String recentlyUsedFilePath = fileSystem.joinPath(cacheRoot, "recently_used.json");

        if (fileSystem.fileExists(recentlyUsedFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(recentlyUsedFilePath));
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                for (const auto &item : *array)
                {
                    if (item.toString() == unitId)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

// Favorites functionality
bool CacheManager::addToFavorites(const juce::String &unitId)
{
    try
    {
        juce::String favoritesFilePath = fileSystem.joinPath(cacheRoot, "favorites.json");

        // Load existing favorites list
        juce::StringArray favorites;
        if (fileSystem.fileExists(favoritesFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(favoritesFilePath));
            if (json.hasProperty("favorites") && json["favorites"].isArray())
            {
                auto array = json["favorites"].getArray();
                for (const auto &item : *array)
                {
                    favorites.add(item.toString());
                }
            }
        }

        // Add the unit if it doesn't already exist
        if (!favorites.contains(unitId))
        {
            favorites.add(unitId);
        }

        // Save the updated list
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        juce::Array<juce::var> array;
        for (const auto &item : favorites)
        {
            array.add(item);
        }
        jsonObj->setProperty("favorites", array);

        return fileSystem.writeFile(favoritesFilePath, juce::JSON::toString(juce::var(jsonObj)));
    }
    catch (...)
    {
        return false;
    }
}

juce::StringArray CacheManager::getFavorites() const
{
    try
    {
        juce::String favoritesFilePath = fileSystem.joinPath(cacheRoot, "favorites.json");
        juce::StringArray favorites;

        if (fileSystem.fileExists(favoritesFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(favoritesFilePath));
            if (json.hasProperty("favorites") && json["favorites"].isArray())
            {
                auto array = json["favorites"].getArray();
                for (const auto &item : *array)
                {
                    favorites.add(item.toString());
                }
            }
        }

        return favorites;
    }
    catch (...)
    {
        return juce::StringArray();
    }
}

bool CacheManager::removeFromFavorites(const juce::String &unitId)
{
    try
    {
        juce::String favoritesFilePath = fileSystem.joinPath(cacheRoot, "favorites.json");

        // Load existing favorites list
        juce::StringArray favorites;
        if (fileSystem.fileExists(favoritesFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(favoritesFilePath));
            if (json.hasProperty("favorites") && json["favorites"].isArray())
            {
                auto array = json["favorites"].getArray();
                for (const auto &item : *array)
                {
                    favorites.add(item.toString());
                }
            }
        }

        // Remove the unit
        bool wasRemoved = favorites.contains(unitId);
        if (wasRemoved)
        {
            favorites.removeString(unitId);
        }

        if (wasRemoved)
        {
            // Save the updated list
            juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
            juce::Array<juce::var> array;
            for (const auto &item : favorites)
            {
                array.add(item);
            }
            jsonObj->setProperty("favorites", array);

            return fileSystem.writeFile(favoritesFilePath, juce::JSON::toString(juce::var(jsonObj)));
        }

        return true; // Unit wasn't in the list, so "successfully" removed
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::clearFavorites()
{
    try
    {
        juce::String favoritesFilePath = fileSystem.joinPath(cacheRoot, "favorites.json");

        if (fileSystem.fileExists(favoritesFilePath))
        {
            return fileSystem.deleteFile(favoritesFilePath);
        }

        return true; // File doesn't exist, so "successfully" cleared
    }
    catch (...)
    {
        return false;
    }
}

bool CacheManager::isFavorite(const juce::String &unitId) const
{
    try
    {
        juce::String favoritesFilePath = fileSystem.joinPath(cacheRoot, "favorites.json");

        if (fileSystem.fileExists(favoritesFilePath))
        {
            auto json = juce::JSON::parse(fileSystem.readFile(favoritesFilePath));
            if (json.hasProperty("favorites") && json["favorites"].isArray())
            {
                auto array = json["favorites"].getArray();
                for (const auto &item : *array)
                {
                    if (item.toString() == unitId)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

CacheManager &CacheManager::getDummy()
{
    static IFileSystem &dummyFileSystem = IFileSystem::getDummy();
    static CacheManager dummyCacheManager(dummyFileSystem, "");
    return dummyCacheManager;
}