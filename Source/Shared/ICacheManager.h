#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

class ICacheManager
{
public:
    // Constants for favorites and recently used
    static constexpr int MAX_RECENTLY_USED = 20;
    static constexpr int MAX_FAVORITES = 100;

    virtual ~ICacheManager() = default;

    // Cache management
    virtual bool isCached(const juce::String &assetId) = 0;
    virtual juce::String getCachedPath(const juce::String &assetId) = 0;
    virtual bool addToCache(const juce::String &assetId, const juce::MemoryBlock &data) = 0;
    virtual bool addToCache(const juce::String &assetId, const juce::Image &image) = 0;
    virtual bool cacheData(const juce::String &assetId, const juce::String &data) = 0;
    virtual bool cacheBinaryData(const juce::String &assetId, const juce::MemoryBlock &data) = 0;
    virtual juce::Image getCachedImage(const juce::String &assetId) = 0;
    virtual bool removeFromCache(const juce::String &assetId) = 0;
    virtual void clearCache() = 0;
    virtual void clearCache(const juce::String &assetId) = 0;
    virtual juce::int64 getCacheSize() = 0;
    virtual juce::int64 getMaxCacheSize() = 0;
    virtual void setMaxCacheSize(juce::int64 maxSize) = 0;

    // Asset validation
    virtual bool validateCachedAsset(const juce::String &assetId) = 0;
    virtual juce::Time getAssetTimestamp(const juce::String &assetId) = 0;
    virtual bool isAssetExpired(const juce::String &assetId, juce::int64 maxAgeSeconds) = 0;

    // Cache optimization
    virtual void optimizeCache() = 0;
    virtual void removeExpiredAssets(juce::int64 maxAgeSeconds) = 0;
    virtual void removeLeastUsedAssets(int count) = 0;

    // Statistics
    virtual int getCachedAssetCount() = 0;
    virtual juce::StringArray getCachedAssetIds() = 0;
    virtual juce::int64 getAssetSize(const juce::String &assetId) = 0;

    // Favorites management
    virtual bool addToFavorites(const juce::String &unitId) = 0;
    virtual bool removeFromFavorites(const juce::String &unitId) = 0;
    virtual bool isInFavorites(const juce::String &unitId) = 0;
    virtual juce::StringArray getFavorites() = 0;
    virtual void clearFavorites() = 0;

    // Recently used management
    virtual bool addToRecentlyUsed(const juce::String &unitId) = 0;
    virtual bool removeFromRecentlyUsed(const juce::String &unitId) = 0;
    virtual bool isInRecentlyUsed(const juce::String &unitId) = 0;
    virtual juce::StringArray getRecentlyUsed(int maxCount = MAX_RECENTLY_USED) = 0;
    virtual void clearRecentlyUsed() = 0;

    // Null Object Pattern implementation
    static ICacheManager &getDummy();
};

// Dummy implementation for testing and fallback
class DummyCacheManager : public ICacheManager
{
public:
    bool isCached(const juce::String &) override { return false; }
    juce::String getCachedPath(const juce::String &) override { return ""; }
    bool addToCache(const juce::String &, const juce::MemoryBlock &) override { return false; }
    bool addToCache(const juce::String &, const juce::Image &) override { return false; }
    bool cacheData(const juce::String &, const juce::String &) override { return false; }
    bool cacheBinaryData(const juce::String &, const juce::MemoryBlock &) override { return false; }
    juce::Image getCachedImage(const juce::String &) override { return juce::Image(); }
    bool removeFromCache(const juce::String &) override { return false; }
    void clearCache() override {}
    void clearCache(const juce::String &) override {}
    juce::int64 getCacheSize() override { return 0; }
    juce::int64 getMaxCacheSize() override { return 0; }
    void setMaxCacheSize(juce::int64) override {}
    bool validateCachedAsset(const juce::String &) override { return false; }
    juce::Time getAssetTimestamp(const juce::String &) override { return juce::Time(); }
    bool isAssetExpired(const juce::String &, juce::int64) override { return true; }
    void optimizeCache() override {}
    void removeExpiredAssets(juce::int64) override {}
    void removeLeastUsedAssets(int) override {}
    int getCachedAssetCount() override { return 0; }
    juce::StringArray getCachedAssetIds() override { return juce::StringArray(); }
    juce::int64 getAssetSize(const juce::String &) override { return 0; }

    // Favorites management
    bool addToFavorites(const juce::String &) override { return false; }
    bool removeFromFavorites(const juce::String &) override { return false; }
    bool isInFavorites(const juce::String &) override { return false; }
    juce::StringArray getFavorites() override { return juce::StringArray(); }
    void clearFavorites() override {}

    // Recently used management
    bool addToRecentlyUsed(const juce::String &) override { return false; }
    bool removeFromRecentlyUsed(const juce::String &) override { return false; }
    bool isInRecentlyUsed(const juce::String &) override { return false; }
    juce::StringArray getRecentlyUsed(int) override { return juce::StringArray(); }
    void clearRecentlyUsed() override {}
};

inline ICacheManager &ICacheManager::getDummy()
{
    static DummyCacheManager dummy;
    return dummy;
}
