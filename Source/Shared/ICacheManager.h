#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

class ICacheManager
{
public:
    virtual ~ICacheManager() = default;

    // Cache management
    virtual bool isCached(const juce::String &assetId) = 0;
    virtual juce::String getCachedPath(const juce::String &assetId) = 0;
    virtual bool addToCache(const juce::String &assetId, const juce::MemoryBlock &data) = 0;
    virtual bool addToCache(const juce::String &assetId, const juce::Image &image) = 0;
    virtual bool removeFromCache(const juce::String &assetId) = 0;
    virtual void clearCache() = 0;
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
    bool removeFromCache(const juce::String &) override { return false; }
    void clearCache() override {}
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
};

inline ICacheManager &ICacheManager::getDummy()
{
    static DummyCacheManager dummy;
    return dummy;
}
