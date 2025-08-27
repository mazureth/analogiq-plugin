#pragma once
#include "../Shared/ICacheManager.h"
#include "../Shared/IFileSystem.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <unordered_map>

class CacheManager : public ICacheManager
{
public:
    explicit CacheManager(IFileSystem &fileSystem);
    ~CacheManager() override;

    // ICacheManager implementation
    bool isCached(const juce::String &assetId) override;
    juce::String getCachedPath(const juce::String &assetId) override;
    bool addToCache(const juce::String &assetId, const juce::MemoryBlock &data) override;
    bool addToCache(const juce::String &assetId, const juce::Image &image) override;
    bool cacheData(const juce::String &assetId, const juce::String &data) override;
    bool cacheBinaryData(const juce::String &assetId, const juce::MemoryBlock &data) override;
    bool removeFromCache(const juce::String &assetId) override;
    void clearCache() override;
    void clearCache(const juce::String &assetId) override;
    juce::int64 getCacheSize() override;
    juce::int64 getMaxCacheSize() override;
    void setMaxCacheSize(juce::int64 maxSize) override;

    bool validateCachedAsset(const juce::String &assetId) override;
    juce::Time getAssetTimestamp(const juce::String &assetId) override;
    bool isAssetExpired(const juce::String &assetId, juce::int64 maxAgeSeconds) override;

    void optimizeCache() override;
    void removeExpiredAssets(juce::int64 maxAgeSeconds) override;
    void removeLeastUsedAssets(int count) override;

    int getCachedAssetCount() override;
    juce::StringArray getCachedAssetIds() override;
    juce::int64 getAssetSize(const juce::String &assetId) override;

private:
    struct CacheEntry
    {
        juce::String assetId;
        juce::String filePath;
        juce::int64 size;
        juce::Time timestamp;
        int accessCount;
        juce::Time lastAccessed;
    };

    IFileSystem &fileSystem;
    juce::String cacheRootDir;
    juce::int64 maxCacheSize;
    juce::int64 currentCacheSize;
    std::unordered_map<juce::String, CacheEntry> cacheEntries;

    // Helper methods
    void initializeCacheDirectory();
    juce::String generateAssetPath(const juce::String &assetId);
    void updateAccessCount(const juce::String &assetId);
    void loadCacheIndex();
    void saveCacheIndex();
    void cleanupCache();
    juce::String hashAssetId(const juce::String &assetId);
};
