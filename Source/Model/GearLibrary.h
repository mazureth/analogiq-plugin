#pragma once
#include "../Shared/IGearLibrary.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/INetworkFetcher.h"
#include "GearItem.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <unordered_map>

class GearLibrary : public IGearLibrary
{
public:
    explicit GearLibrary(IFileSystem &fileSystem, ICacheManager &cacheManager, INetworkFetcher &networkFetcher);
    ~GearLibrary() override;

    // IGearLibrary implementation
    bool addGearItem(const GearItem &gearItem) override;
    bool removeGearItem(const juce::String &gearId) override;
    bool updateGearItem(const GearItem &gearItem) override;
    GearItem *getGearItem(const juce::String &gearId) override;
    const GearItem *getGearItem(const juce::String &gearId) const override;
    bool gearItemExists(const juce::String &gearId) override;

    juce::Array<GearItem *> getAllGearItems() override;
    juce::Array<const GearItem *> getAllGearItems() const override;
    int getTotalGearItemCount() const override;
    void clearAllGearItems() override;

    bool createGearCategory(const juce::String &categoryName) override;
    bool deleteGearCategory(const juce::String &categoryName) override;
    juce::StringArray getGearCategories() const override;
    bool assignGearToCategory(const juce::String &gearId, const juce::String &categoryName) override;
    juce::Array<const GearItem *> getGearItemsInCategory(const juce::String &categoryName) const override;
    juce::String getGearCategory(const juce::String &gearId) const override;

    juce::Array<const GearItem *> searchGearItems(const juce::String &searchTerm) const override;
    juce::Array<const GearItem *> filterGearByType(GearItem::GearType type) const override;
    juce::Array<const GearItem *> filterGearByCategory(GearItem::GearCategory category) const override;
    juce::Array<const GearItem *> filterGearByManufacturer(const juce::String &manufacturer) const override;
    juce::Array<const GearItem *> filterGearByDateRange(const juce::Time &startDate, const juce::Time &endDate) const override;

    bool validateGearItem(const juce::String &gearId) override;
    bool isGearItemCorrupted(const juce::String &gearId) override;
    bool repairGearItem(const juce::String &gearId) override;
    juce::StringArray getCorruptedGearItems() const override;

    bool exportGearItem(const juce::String &gearId, const juce::String &exportPath) override;
    bool importGearItem(const juce::String &importPath, const juce::String &gearId = "") override;
    bool exportGearLibrary(const juce::String &exportPath) override;
    bool importGearLibrary(const juce::String &importPath) override;
    bool exportGearCategory(const juce::String &categoryName, const juce::String &exportPath) override;

    juce::int64 getTotalGearStorageSize() const override;
    juce::int64 getGearItemSize(const juce::String &gearId) const override;
    juce::Time getGearItemCreationTime(const juce::String &gearId) const override;
    juce::Time getGearItemLastModifiedTime(const juce::String &gearId) const override;
    juce::String getGearItemAuthor(const juce::String &gearId) const override;

    bool createGearLibraryBackup(const juce::String &backupPath) override;
    bool restoreGearLibraryFromBackup(const juce::String &backupPath) override;
    juce::StringArray getAvailableBackups() const override;
    bool optimizeGearLibrary() override;
    bool validateGearLibraryIntegrity() override;

    bool syncWithRemoteLibrary(const juce::String &remoteUrl) override;
    bool uploadGearItem(const juce::String &gearId, const juce::String &remoteUrl) override;
    bool downloadGearItem(const juce::String &gearId, const juce::String &remoteUrl) override;
    bool checkForUpdates(const juce::String &remoteUrl) override;

    void setMaxGearItems(int maxItems) override;
    int getMaxGearItems() const override;
    void setMaxStorageSize(juce::int64 maxSize) override;
    juce::int64 getMaxStorageSize() const override;
    void setAutoBackupEnabled(bool enabled) override;
    bool isAutoBackupEnabled() const override;

    // New methods for enhanced remote gear library functionality
    bool refreshRemoteGearLibrary();
    bool downloadGearImages();
    bool downloadGearSchemas();
    bool checkGearCompatibility(const juce::String &gearId);
    bool updateGearVersions();
    juce::String getRemoteLibraryStatus() const;
    bool isRemoteLibraryAvailable() const;
    juce::Time getLastRemoteSyncTime() const;

private:
    struct GearCategory
    {
        juce::String name;
        juce::StringArray gearIds;
        juce::Time creationTime;
        juce::String description;
    };

    struct GearMetadata
    {
        juce::String id;
        juce::String name;
        juce::String manufacturer;
        juce::String category;
        juce::Time creationTime;
        juce::Time lastModifiedTime;
        juce::int64 fileSize;
        bool isValid;
        juce::String author;
        GearItem::GearType type;
        GearItem::GearCategory gearCategory;
    };

    struct RemoteLibraryInfo
    {
        juce::String url;
        juce::Time lastSyncTime;
        juce::String lastSyncVersion;
        bool isAvailable;
        juce::String lastError;
        int lastHttpCode;
    };

    // Member variables
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    INetworkFetcher &networkFetcher;
    juce::String libraryRootDir;
    juce::Array<GearItem *> gearItems;
    std::unordered_map<juce::String, GearMetadata> gearMetadata;
    std::unordered_map<juce::String, GearCategory> categories;
    RemoteLibraryInfo remoteLibraryInfo;

    int maxGearItems;
    juce::int64 maxStorageSize;
    bool autoBackupEnabled;

    // Private helper methods
    void initializeLibraryDirectory();
    void loadGearMetadata();
    void saveGearMetadata();
    void loadCategories();
    void saveCategories();
    void loadRemoteGearLibrary();
    void createSampleGearItems();
    void createCategoriesSection();

    // New private helper methods for remote operations
    bool parseRemoteGearLibrary(const juce::String &jsonData);
    bool downloadGearAsset(const juce::String &gearId, const juce::String &assetUrl, const juce::String &assetType);
    bool validateRemoteGearData(const juce::var &gearObject);
    void updateRemoteLibraryStatus(bool available, const juce::String &error = "", int httpCode = 0);
    bool shouldRefreshRemoteLibrary() const;
    juce::String getRemoteLibraryUrl() const;
    void setRemoteLibraryUrl(const juce::String &url);

    // Additional helper methods that are referenced but not yet implemented
    juce::String generateGearItemPath(const juce::String &gearId);
    juce::String generateMetadataPath(const juce::String &gearId);
    juce::String generateCategoryPath(const juce::String &categoryName);
    juce::String sanitizeGearId(const juce::String &gearId);
    bool checkStorageLimits();

    // Search and filtering helpers
    bool shouldShowItem(const GearItem *item, const juce::String &normalizedSearch) const;
    juce::String normalizeForSearch(const juce::String &text) const;
    juce::StringArray getIgnoredCharacters() const;

    // Metadata and category management
    void updateGearMetadata();
    void updateCategories();
};
