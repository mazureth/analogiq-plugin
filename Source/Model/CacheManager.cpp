#include "CacheManager.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

CacheManager::CacheManager(IFileSystem &fs)
    : fileSystem(fs), maxCacheSize(100 * 1024 * 1024) // 100MB default
      ,
      currentCacheSize(0)
{
    initializeCacheDirectory();
    loadCacheIndex();
}

CacheManager::~CacheManager()
{
    saveCacheIndex();
}

void CacheManager::initializeCacheDirectory()
{
    cacheRootDir = fileSystem.getCacheRootDirectory();
    if (!fileSystem.directoryExists(cacheRootDir))
    {
        fileSystem.createDirectory(cacheRootDir);
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
    return fileSystem.joinPath(cacheRootDir, hash + ".cache");
}

bool CacheManager::isCached(const juce::String &assetId)
{
    return cacheEntries.find(assetId) != cacheEntries.end() &&
           fileSystem.fileExists(cacheEntries[assetId].filePath);
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
    if (assetId.isEmpty() || image.isNull())
        return false;

    juce::MemoryBlock data;
    juce::JPEGImageFormat format;
    juce::MemoryOutputStream stream(data, false);

    if (format.writeImageToStream(image, stream))
    {
        return addToCache(assetId, data);
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
