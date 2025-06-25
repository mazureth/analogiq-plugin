#include "CacheManager.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

CacheManager::CacheManager()
{
    // Initialize cache root directory using JUCE's cross-platform path resolution
    cacheRoot = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AnalogiqCache");
}

CacheManager &CacheManager::getInstance()
{
    static CacheManager instance;
    return instance;
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
        if (!createDirectoryIfNeeded(getControlsDirectory().getChildFile("buttons")))
            return false;

        if (!createDirectoryIfNeeded(getControlsDirectory().getChildFile("faders")))
            return false;

        if (!createDirectoryIfNeeded(getControlsDirectory().getChildFile("knobs")))
            return false;

        if (!createDirectoryIfNeeded(getControlsDirectory().getChildFile("switches")))
            return false;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::File CacheManager::getCacheRoot() const
{
    return cacheRoot;
}

bool CacheManager::isUnitCached(const juce::String &unitId) const
{
    juce::File unitFile = getCachedUnitPath(unitId);
    return unitFile.existsAsFile();
}

bool CacheManager::isFaceplateCached(const juce::String &unitId, const juce::String &filename) const
{
    juce::File faceplateFile = getCachedFaceplatePath(unitId, filename);
    return faceplateFile.existsAsFile();
}

bool CacheManager::isThumbnailCached(const juce::String &unitId, const juce::String &filename) const
{
    juce::File thumbnailFile = getCachedThumbnailPath(unitId, filename);
    return thumbnailFile.existsAsFile();
}

bool CacheManager::isControlAssetCached(const juce::String &assetPath) const
{
    juce::File assetFile = getCachedControlAssetPath(assetPath);
    return assetFile.existsAsFile();
}

juce::File CacheManager::getCachedUnitPath(const juce::String &unitId) const
{
    return getUnitsDirectory().getChildFile(unitId + ".json");
}

juce::File CacheManager::getCachedFaceplatePath(const juce::String &unitId, const juce::String &filename) const
{
    return getFaceplatesDirectory().getChildFile(filename);
}

juce::File CacheManager::getCachedThumbnailPath(const juce::String &unitId, const juce::String &filename) const
{
    return getThumbnailsDirectory().getChildFile(filename);
}

juce::File CacheManager::getCachedControlAssetPath(const juce::String &assetPath) const
{
    return getControlsDirectory().getChildFile(assetPath);
}

bool CacheManager::saveUnitToCache(const juce::String &unitId, const juce::String &jsonData)
{
    try
    {
        juce::File unitFile = getCachedUnitPath(unitId);

        // Ensure the units directory exists
        if (!createDirectoryIfNeeded(getUnitsDirectory()))
            return false;

        // Write the JSON data to file
        return unitFile.replaceWithText(jsonData);
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
        juce::File faceplateFile = getCachedFaceplatePath(unitId, filename);

        // Ensure the faceplates directory exists
        if (!createDirectoryIfNeeded(getFaceplatesDirectory()))
            return false;

        // Save the image as JPEG
        juce::JPEGImageFormat jpegFormat;
        juce::FileOutputStream stream(faceplateFile);

        if (stream.openedOk())
        {
            return jpegFormat.writeImageToStream(image, stream);
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
        juce::File thumbnailFile = getCachedThumbnailPath(unitId, filename);

        // Ensure the thumbnails directory exists
        if (!createDirectoryIfNeeded(getThumbnailsDirectory()))
            return false;

        // Save the image as JPEG
        juce::JPEGImageFormat jpegFormat;
        juce::FileOutputStream stream(thumbnailFile);

        if (stream.openedOk())
        {
            return jpegFormat.writeImageToStream(image, stream);
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
        juce::File assetFile = getCachedControlAssetPath(assetPath);

        // Ensure the parent directory exists
        juce::File parentDir = assetFile.getParentDirectory();
        if (!createDirectoryIfNeeded(parentDir))
            return false;

        // Write the image data to file
        juce::FileOutputStream stream(assetFile);

        if (stream.openedOk())
        {
            return stream.write(imageData.getData(), imageData.getSize()) == imageData.getSize();
        }

        return false;
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
        juce::File unitFile = getCachedUnitPath(unitId);

        if (unitFile.existsAsFile())
        {
            return unitFile.loadFileAsString();
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
        juce::File faceplateFile = getCachedFaceplatePath(unitId, filename);

        if (faceplateFile.existsAsFile())
        {
            juce::Image image = juce::ImageFileFormat::loadFrom(faceplateFile);
            return image;
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
        juce::File thumbnailFile = getCachedThumbnailPath(unitId, filename);

        if (thumbnailFile.existsAsFile())
        {
            juce::Image image = juce::ImageFileFormat::loadFrom(thumbnailFile);
            return image;
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
        juce::File assetFile = getCachedControlAssetPath(assetPath);

        if (assetFile.existsAsFile())
        {
            juce::Image image = juce::ImageFileFormat::loadFrom(assetFile);
            return image;
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
        if (cacheRoot.exists())
        {
            return cacheRoot.deleteRecursively();
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
        if (cacheRoot.exists())
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

juce::int64 CacheManager::calculateDirectorySize(const juce::File &directory) const
{
    juce::int64 totalSize = 0;

    if (directory.isDirectory())
    {
        juce::Array<juce::File> children;
        directory.findChildFiles(children, juce::File::findFilesAndDirectories, false);

        for (const auto &child : children)
        {
            if (child.isDirectory())
            {
                totalSize += calculateDirectorySize(child);
            }
            else
            {
                totalSize += child.getSize();
            }
        }
    }

    return totalSize;
}

bool CacheManager::createDirectoryIfNeeded(const juce::File &directory) const
{
    try
    {
        if (!directory.exists())
        {
            return directory.createDirectory();
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

juce::File CacheManager::getUnitsDirectory() const
{
    return cacheRoot.getChildFile("units");
}

juce::File CacheManager::getAssetsDirectory() const
{
    return cacheRoot.getChildFile("assets");
}

juce::File CacheManager::getFaceplatesDirectory() const
{
    return getAssetsDirectory().getChildFile("faceplates");
}

juce::File CacheManager::getThumbnailsDirectory() const
{
    return getAssetsDirectory().getChildFile("thumbnails");
}

juce::File CacheManager::getControlsDirectory() const
{
    return getAssetsDirectory().getChildFile("controls");
}

// Recently Used functionality
bool CacheManager::addToRecentlyUsed(const juce::String &unitId)
{
    try
    {
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");

        // Load existing recently used list
        juce::StringArray recentlyUsed;
        if (recentlyUsedFile.existsAsFile())
        {
            auto json = juce::JSON::parse(recentlyUsedFile);
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                for (const auto &item : *array)
                {
                    recentlyUsed.add(item.toString());
                }
            }
        }

        // Remove the unit if it already exists (to move it to the front)
        recentlyUsed.removeString(unitId);

        // Add the unit to the front of the list
        recentlyUsed.insert(0, unitId);

        // Limit the list to MAX_RECENTLY_USED items
        while (recentlyUsed.size() > MAX_RECENTLY_USED)
        {
            recentlyUsed.remove(recentlyUsed.size() - 1);
        }

        // Save the updated list
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        juce::Array<juce::var> array;
        for (const auto &item : recentlyUsed)
        {
            array.add(item);
        }
        jsonObj->setProperty("recentlyUsed", array);

        return recentlyUsedFile.replaceWithText(juce::JSON::toString(juce::var(jsonObj)));
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
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");
        juce::StringArray recentlyUsed;

        if (recentlyUsedFile.existsAsFile())
        {
            auto json = juce::JSON::parse(recentlyUsedFile);
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
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");

        // Load existing recently used list
        juce::StringArray recentlyUsed;
        if (recentlyUsedFile.existsAsFile())
        {
            auto json = juce::JSON::parse(recentlyUsedFile);
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

            return recentlyUsedFile.replaceWithText(juce::JSON::toString(juce::var(jsonObj)));
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
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");

        if (recentlyUsedFile.existsAsFile())
        {
            return recentlyUsedFile.deleteFile();
        }

        return true; // File doesn't exist, so "successfully" cleared
    }
    catch (...)
    {
        return false;
    }
}

int CacheManager::getRecentlyUsedCount() const
{
    try
    {
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");

        if (recentlyUsedFile.existsAsFile())
        {
            auto json = juce::JSON::parse(recentlyUsedFile);
            if (json.hasProperty("recentlyUsed") && json["recentlyUsed"].isArray())
            {
                auto array = json["recentlyUsed"].getArray();
                return array->size();
            }
        }

        return 0;
    }
    catch (...)
    {
        return 0;
    }
}

bool CacheManager::isRecentlyUsed(const juce::String &unitId) const
{
    try
    {
        juce::File recentlyUsedFile = cacheRoot.getChildFile("recently_used.json");

        if (recentlyUsedFile.existsAsFile())
        {
            auto json = juce::JSON::parse(recentlyUsedFile);
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
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");

        // Load existing favorites list
        juce::StringArray favorites;
        if (favoritesFile.existsAsFile())
        {
            auto json = juce::JSON::parse(favoritesFile);
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

        return favoritesFile.replaceWithText(juce::JSON::toString(juce::var(jsonObj)));
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
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");
        juce::StringArray favorites;

        if (favoritesFile.existsAsFile())
        {
            auto json = juce::JSON::parse(favoritesFile);
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
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");

        // Load existing favorites list
        juce::StringArray favorites;
        if (favoritesFile.existsAsFile())
        {
            auto json = juce::JSON::parse(favoritesFile);
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

            return favoritesFile.replaceWithText(juce::JSON::toString(juce::var(jsonObj)));
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
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");

        if (favoritesFile.existsAsFile())
        {
            return favoritesFile.deleteFile();
        }

        return true; // File doesn't exist, so "successfully" cleared
    }
    catch (...)
    {
        return false;
    }
}

int CacheManager::getFavoritesCount() const
{
    try
    {
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");

        if (favoritesFile.existsAsFile())
        {
            auto json = juce::JSON::parse(favoritesFile);
            if (json.hasProperty("favorites") && json["favorites"].isArray())
            {
                auto array = json["favorites"].getArray();
                return array->size();
            }
        }

        return 0;
    }
    catch (...)
    {
        return 0;
    }
}

bool CacheManager::isFavorite(const juce::String &unitId) const
{
    try
    {
        juce::File favoritesFile = cacheRoot.getChildFile("favorites.json");

        if (favoritesFile.existsAsFile())
        {
            auto json = juce::JSON::parse(favoritesFile);
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