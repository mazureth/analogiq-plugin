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