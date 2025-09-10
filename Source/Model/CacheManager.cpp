#include "CacheManager.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

CacheManager::CacheManager(IFileSystem &fs)
    : fileSystem(fs), maxCacheSize(100 * 1024 * 1024) // 100MB default
      ,
      currentCacheSize(0), initialized(false)
{
    // Lazy initialization - no file system operations during construction
}

CacheManager::~CacheManager()
{
    saveCacheIndex();
}

void CacheManager::initializeLazy()
{
    if (initialized)
        return;

    initializeCacheDirectory();
    loadCacheIndex();

    // Initialize favorites and recently used file paths
    favoritesFilePath = fileSystem.joinPath(cacheRootDir, "favorites.json");
    recentlyUsedFilePath = fileSystem.joinPath(cacheRootDir, "recently_used.json");

    // Load favorites and recently used data
    loadFavorites();
    loadRecentlyUsed();

    initialized = true;
}

void CacheManager::initializeCacheDirectory()
{
    cacheRootDir = fileSystem.getCacheRootDirectory();
    if (!fileSystem.directoryExists(cacheRootDir))
    {
        fileSystem.createDirectory(cacheRootDir);
    }

    // Create Assets subfolder for image cache files
    assetsDir = fileSystem.joinPath(cacheRootDir, "Assets");
    if (!fileSystem.directoryExists(assetsDir))
    {
        fileSystem.createDirectory(assetsDir);
    }
}

juce::String CacheManager::hashAssetId(const juce::String &assetId)
{
    // Simple hash function for asset IDs
    juce::int64 hash = 0;
    for (auto c : assetId)
        hash = ((hash << 5) - hash) + c;
    return juce::String(std::abs(hash));
}

juce::String CacheManager::generateAssetPath(const juce::String &assetId)
{
    auto hash = hashAssetId(assetId);
    return fileSystem.joinPath(assetsDir, hash + ".cache");
}

bool CacheManager::isCached(const juce::String &assetId)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    initializeLazy();
    bool cached = cacheEntries.find(assetId) != cacheEntries.end() &&
                  fileSystem.fileExists(cacheEntries[assetId].filePath);
    if (assetId.startsWith("faceplate_"))
    {
    }
    return cached;
}

juce::String CacheManager::getCachedPath(const juce::String &assetId)
{
    if (isCached(assetId))
    {
        updateAccessCount(assetId);
        return cacheEntries[assetId].filePath;
    }
    return "";
}

bool CacheManager::addToCache(const juce::String &assetId, const juce::MemoryBlock &data)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    initializeLazy();
    if (assetId.isEmpty())
        return false;

    auto filePath = generateAssetPath(assetId);

    // Check if we need to make space
    if (currentCacheSize + data.getSize() > maxCacheSize)
    {
        cleanupCache();
        if (currentCacheSize + data.getSize() > maxCacheSize)
            return false; // Still too big
    }

    if (fileSystem.writeFile(filePath, data))
    {
        CacheEntry entry;
        entry.assetId = assetId;
        entry.filePath = filePath;
        entry.size = data.getSize();
        entry.timestamp = juce::Time::getCurrentTime();
        entry.accessCount = 1;
        entry.lastAccessed = entry.timestamp;

        cacheEntries[assetId] = entry;
        currentCacheSize += data.getSize();

        saveCacheIndex();
        return true;
    }

    return false;
}

bool CacheManager::addToCache(const juce::String &assetId, const juce::Image &image)
{
    initializeLazy();
    if (assetId.isEmpty() || image.isNull())
        return false;

    if (assetId.startsWith("faceplate_"))
    {
    }

    juce::MemoryBlock data;
    juce::JPEGImageFormat format;
    juce::MemoryOutputStream stream(data, false);

    if (format.writeImageToStream(image, stream))
    {
        bool result = addToCache(assetId, data);
        if (assetId.startsWith("faceplate_"))
        {
        }
        return result;
    }

    if (assetId.startsWith("faceplate_"))
    {
    }
    return false;
}

bool CacheManager::removeFromCache(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        auto &entry = it->second;
        if (fileSystem.deleteFile(entry.filePath))
        {
            currentCacheSize -= entry.size;
            cacheEntries.erase(it);
            saveCacheIndex();
            return true;
        }
    }
    return false;
}

void CacheManager::clearCache()
{
    for (auto &entry : cacheEntries)
    {
        fileSystem.deleteFile(entry.second.filePath);
    }
    cacheEntries.clear();
    currentCacheSize = 0;
    saveCacheIndex();
}

juce::int64 CacheManager::getCacheSize()
{
    return currentCacheSize;
}

juce::int64 CacheManager::getMaxCacheSize()
{
    return maxCacheSize;
}

void CacheManager::setMaxCacheSize(juce::int64 maxSize)
{
    maxCacheSize = maxSize;
    if (currentCacheSize > maxCacheSize)
    {
        cleanupCache();
    }
}

bool CacheManager::validateCachedAsset(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        auto &entry = it->second;
        return fileSystem.fileExists(entry.filePath) &&
               fileSystem.getFileSize(entry.filePath) == entry.size;
    }
    return false;
}

juce::Time CacheManager::getAssetTimestamp(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        return it->second.timestamp;
    }
    return juce::Time();
}

bool CacheManager::isAssetExpired(const juce::String &assetId, juce::int64 maxAgeSeconds)
{
    auto timestamp = getAssetTimestamp(assetId);
    if (timestamp.toMilliseconds() == 0)
        return true;

    auto now = juce::Time::getCurrentTime();
    auto age = now.toMilliseconds() - timestamp.toMilliseconds();
    return age > (maxAgeSeconds * 1000);
}

void CacheManager::optimizeCache()
{
    if (currentCacheSize <= maxCacheSize)
        return;

    // Remove expired assets first
    removeExpiredAssets(24 * 60 * 60); // 24 hours

    // If still over limit, remove least used
    if (currentCacheSize > maxCacheSize)
    {
        int toRemove = static_cast<int>(cacheEntries.size() * 0.1); // Remove 10%
        removeLeastUsedAssets(toRemove);
    }
}

void CacheManager::removeExpiredAssets(juce::int64 maxAgeSeconds)
{
    juce::StringArray toRemove;

    for (auto &entry : cacheEntries)
    {
        if (isAssetExpired(entry.first, maxAgeSeconds))
        {
            toRemove.add(entry.first);
        }
    }

    for (auto &assetId : toRemove)
    {
        removeFromCache(assetId);
    }
}

void CacheManager::removeLeastUsedAssets(int count)
{
    if (count <= 0)
        return;

    // Sort by access count and last accessed time
    std::vector<std::pair<juce::String, int>> sortedEntries;
    for (auto &entry : cacheEntries)
    {
        sortedEntries.emplace_back(entry.first, entry.second.accessCount);
    }

    std::sort(sortedEntries.begin(), sortedEntries.end(),
              [this](const auto &a, const auto &b)
              {
                  auto &entryA = cacheEntries[a.first];
                  auto &entryB = cacheEntries[b.first];

                  if (entryA.accessCount != entryB.accessCount)
                      return entryA.accessCount < entryB.accessCount;

                  return entryA.lastAccessed < entryB.lastAccessed;
              });

    // Remove the least used
    for (int i = 0; i < count && i < static_cast<int>(sortedEntries.size()); ++i)
    {
        removeFromCache(sortedEntries[i].first);
    }
}

int CacheManager::getCachedAssetCount()
{
    return static_cast<int>(cacheEntries.size());
}

juce::StringArray CacheManager::getCachedAssetIds()
{
    juce::StringArray ids;
    for (auto &entry : cacheEntries)
    {
        ids.add(entry.first);
    }
    return ids;
}

juce::int64 CacheManager::getAssetSize(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        return it->second.size;
    }
    return 0;
}

void CacheManager::updateAccessCount(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        it->second.accessCount++;
        it->second.lastAccessed = juce::Time::getCurrentTime();
    }
}

void CacheManager::loadCacheIndex()
{
    auto indexPath = fileSystem.joinPath(cacheRootDir, "cache_index.txt");
    if (fileSystem.fileExists(indexPath))
    {
        auto content = fileSystem.readFile(indexPath);
        auto lines = juce::StringArray::fromLines(content);

        for (auto &line : lines)
        {
            auto parts = juce::StringArray::fromTokens(line, "|", "");
            if (parts.size() >= 6)
            {
                CacheEntry entry;
                entry.assetId = parts[0];
                entry.filePath = parts[1];
                entry.size = parts[2].getLargeIntValue();
                entry.timestamp = juce::Time(parts[3].getLargeIntValue());
                entry.accessCount = parts[4].getIntValue();
                entry.lastAccessed = juce::Time(parts[5].getLargeIntValue());

                cacheEntries[entry.assetId] = entry;
                currentCacheSize += entry.size;
            }
        }
    }
}

void CacheManager::saveCacheIndex()
{
    auto indexPath = fileSystem.joinPath(cacheRootDir, "cache_index.txt");
    juce::String content;

    for (auto &entry : cacheEntries)
    {
        auto &cacheEntry = entry.second;
        content += cacheEntry.assetId + "|" +
                   cacheEntry.filePath + "|" +
                   juce::String(cacheEntry.size) + "|" +
                   juce::String(cacheEntry.timestamp.toMilliseconds()) + "|" +
                   juce::String(cacheEntry.accessCount) + "|" +
                   juce::String(cacheEntry.lastAccessed.toMilliseconds()) + "\n";
    }

    fileSystem.writeFile(indexPath, content);
}

void CacheManager::cleanupCache()
{
    if (currentCacheSize <= maxCacheSize)
        return;

    // Remove least used assets until under limit
    while (currentCacheSize > maxCacheSize && !cacheEntries.empty())
    {
        removeLeastUsedAssets(1);
    }
}

// Additional methods for remote gear library support
bool CacheManager::cacheData(const juce::String &assetId, const juce::String &data)
{
    if (assetId.isEmpty() || data.isEmpty())
        return false;

    juce::MemoryBlock dataBlock;
    dataBlock.append(data.toRawUTF8(), data.getNumBytesAsUTF8());

    return addToCache(assetId, dataBlock);
}

bool CacheManager::cacheBinaryData(const juce::String &assetId, const juce::MemoryBlock &data)
{
    return addToCache(assetId, data);
}

juce::Image CacheManager::getCachedImage(const juce::String &assetId)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cacheEntries.find(assetId);
    if (it == cacheEntries.end())
    {
        if (assetId.startsWith("faceplate_"))
        {
        }
        return juce::Image();
    }

    try
    {
        juce::File imageFile(it->second.filePath);
        if (!imageFile.existsAsFile())
        {
            if (assetId.startsWith("faceplate_"))
            {
            }
            return juce::Image();
        }

        // Load image from file
        juce::Image image = juce::ImageFileFormat::loadFrom(imageFile);
        if (assetId.startsWith("faceplate_"))
        {
        }
        return image;
    }
    catch (...)
    {
        if (assetId.startsWith("faceplate_"))
        {
        }
        return juce::Image();
    }
}

void CacheManager::clearCache(const juce::String &assetId)
{
    auto it = cacheEntries.find(assetId);
    if (it != cacheEntries.end())
    {
        // Remove the file
        fileSystem.deleteFile(it->second.filePath);

        // Update cache size
        currentCacheSize -= it->second.size;

        // Remove from entries
        cacheEntries.erase(it);

        // Save updated index
        saveCacheIndex();
    }
}

// Favorites management implementation
bool CacheManager::addToFavorites(const juce::String &unitId)
{
    if (unitId.isEmpty() || isInFavorites(unitId))
        return false;

    if (favorites.size() >= MAX_FAVORITES)
        favorites.remove(0); // Remove oldest favorite

    favorites.add(unitId);
    saveFavorites();
    return true;
}

bool CacheManager::removeFromFavorites(const juce::String &unitId)
{
    if (unitId.isEmpty())
        return false;

    int index = favorites.indexOf(unitId);
    if (index >= 0)
    {
        favorites.remove(index);
        saveFavorites();
        return true;
    }
    return false;
}

bool CacheManager::isInFavorites(const juce::String &unitId)
{
    return favorites.contains(unitId);
}

juce::StringArray CacheManager::getFavorites()
{
    initializeLazy();
    return favorites;
}

void CacheManager::clearFavorites()
{
    favorites.clear();
    saveFavorites();
}

// Recently used management implementation
bool CacheManager::addToRecentlyUsed(const juce::String &unitId)
{
    if (unitId.isEmpty())
        return false;

    addToRecentlyUsedInternal(unitId);
    saveRecentlyUsed();
    return true;
}

bool CacheManager::removeFromRecentlyUsed(const juce::String &unitId)
{
    if (unitId.isEmpty())
        return false;

    int index = recentlyUsed.indexOf(unitId);
    if (index >= 0)
    {
        recentlyUsed.remove(index);
        saveRecentlyUsed();
        return true;
    }
    return false;
}

bool CacheManager::isInRecentlyUsed(const juce::String &unitId)
{
    return recentlyUsed.contains(unitId);
}

juce::StringArray CacheManager::getRecentlyUsed(int maxCount)
{
    initializeLazy();
    if (maxCount <= 0 || maxCount > MAX_RECENTLY_USED)
        maxCount = MAX_RECENTLY_USED;

    juce::StringArray result;
    int count = 0;
    for (int i = recentlyUsed.size() - 1; i >= 0 && count < maxCount; --i)
    {
        result.add(recentlyUsed[i]);
        count++;
    }
    return result;
}

void CacheManager::clearRecentlyUsed()
{
    recentlyUsed.clear();
    saveRecentlyUsed();
}

// Helper methods for favorites and recently used
void CacheManager::loadFavorites()
{
    if (fileSystem.fileExists(favoritesFilePath))
    {
        auto content = fileSystem.readFile(favoritesFilePath);
        if (!content.isEmpty())
        {
            auto var = juce::JSON::parse(content);
            if (var.isArray())
            {
                favorites.clear();
                auto array = var.getArray();
                for (auto &item : *array)
                {
                    if (item.isString())
                        favorites.add(item.toString());
                }
            }
        }
    }
}

void CacheManager::saveFavorites()
{
    auto var = juce::var(favorites);
    auto json = juce::JSON::toString(var);
    fileSystem.writeFile(favoritesFilePath, json);
}

void CacheManager::loadRecentlyUsed()
{
    if (fileSystem.fileExists(recentlyUsedFilePath))
    {
        auto content = fileSystem.readFile(recentlyUsedFilePath);
        if (!content.isEmpty())
        {
            auto var = juce::JSON::parse(content);
            if (var.isArray())
            {
                recentlyUsed.clear();
                auto array = var.getArray();
                for (auto &item : *array)
                {
                    if (item.isString())
                        recentlyUsed.add(item.toString());
                }
            }
        }
    }
}

void CacheManager::saveRecentlyUsed()
{
    auto var = juce::var(recentlyUsed);
    auto json = juce::JSON::toString(var);
    fileSystem.writeFile(recentlyUsedFilePath, json);
}

void CacheManager::addToRecentlyUsedInternal(const juce::String &unitId)
{
    // Remove if already exists (to move to front)
    recentlyUsed.removeString(unitId);

    // Add to end (most recent)
    recentlyUsed.add(unitId);

    // Limit size
    while (recentlyUsed.size() > MAX_RECENTLY_USED)
        recentlyUsed.remove(0);
}
