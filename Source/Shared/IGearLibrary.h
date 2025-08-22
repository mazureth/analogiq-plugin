#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include "../Model/GearItem.h"

class IGearLibrary
{
public:
    virtual ~IGearLibrary() = default;

    // Gear item management
    virtual bool addGearItem(const GearItem& gearItem) = 0;
    virtual bool removeGearItem(const juce::String& gearId) = 0;
    virtual bool updateGearItem(const GearItem& gearItem) = 0;
    virtual GearItem* getGearItem(const juce::String& gearId) = 0;
    virtual const GearItem* getGearItem(const juce::String& gearId) const = 0;
    virtual bool gearItemExists(const juce::String& gearId) = 0;

    // Gear collection operations
    virtual juce::Array<GearItem*> getAllGearItems() = 0;
    virtual juce::Array<const GearItem*> getAllGearItems() const = 0;
    virtual int getTotalGearItemCount() const = 0;
    virtual void clearAllGearItems() = 0;

    // Gear categorization and organization
    virtual bool createGearCategory(const juce::String& categoryName) = 0;
    virtual bool deleteGearCategory(const juce::String& categoryName) = 0;
    virtual juce::StringArray getGearCategories() const = 0;
    virtual bool assignGearToCategory(const juce::String& gearId, const juce::String& categoryName) = 0;
    virtual juce::Array<const GearItem*> getGearItemsInCategory(const juce::String& categoryName) const = 0;
    virtual juce::String getGearCategory(const juce::String& gearId) const = 0;

    // Gear filtering and search
    virtual juce::Array<const GearItem*> searchGearItems(const juce::String& searchTerm) const = 0;
    virtual juce::Array<const GearItem*> filterGearByType(GearItem::GearType type) const = 0;
    virtual juce::Array<const GearItem*> filterGearByCategory(GearItem::GearCategory category) const = 0;
    virtual juce::Array<const GearItem*> filterGearByManufacturer(const juce::String& manufacturer) const = 0;
    virtual juce::Array<const GearItem*> filterGearByDateRange(const juce::Time& startDate, const juce::Time& endDate) const = 0;

    // Gear validation and integrity
    virtual bool validateGearItem(const juce::String& gearId) = 0;
    virtual bool isGearItemCorrupted(const juce::String& gearId) = 0;
    virtual bool repairGearItem(const juce::String& gearId) = 0;
    virtual juce::StringArray getCorruptedGearItems() const = 0;

    // Gear import/export
    virtual bool exportGearItem(const juce::String& gearId, const juce::String& exportPath) = 0;
    virtual bool importGearItem(const juce::String& importPath, const juce::String& gearId = "") = 0;
    virtual bool exportGearLibrary(const juce::String& exportPath) = 0;
    virtual bool importGearLibrary(const juce::String& importPath) = 0;
    virtual bool exportGearCategory(const juce::String& categoryName, const juce::String& exportPath) = 0;

    // Gear statistics and metadata
    virtual juce::int64 getTotalGearStorageSize() const = 0;
    virtual juce::int64 getGearItemSize(const juce::String& gearId) const = 0;
    virtual juce::Time getGearItemCreationTime(const juce::String& gearId) const = 0;
    virtual juce::Time getGearItemLastModifiedTime(const juce::String& gearId) const = 0;
    virtual juce::String getGearItemAuthor(const juce::String& gearId) const = 0;

    // Gear library management
    virtual bool createGearLibraryBackup(const juce::String& backupPath) = 0;
    virtual bool restoreGearLibraryFromBackup(const juce::String& backupPath) = 0;
    virtual juce::StringArray getAvailableBackups() const = 0;
    virtual bool optimizeGearLibrary() = 0;
    virtual bool validateGearLibraryIntegrity() = 0;

    // Gear library synchronization
    virtual bool syncWithRemoteLibrary(const juce::String& remoteUrl) = 0;
    virtual bool uploadGearItem(const juce::String& gearId, const juce::String& remoteUrl) = 0;
    virtual bool downloadGearItem(const juce::String& gearId, const juce::String& remoteUrl) = 0;
    virtual bool checkForUpdates(const juce::String& remoteUrl) = 0;

    // Gear library configuration
    virtual void setMaxGearItems(int maxItems) = 0;
    virtual int getMaxGearItems() const = 0;
    virtual void setMaxStorageSize(juce::int64 maxSize) = 0;
    virtual juce::int64 getMaxStorageSize() const = 0;
    virtual void setAutoBackupEnabled(bool enabled) = 0;
    virtual bool isAutoBackupEnabled() const = 0;

    // Null Object Pattern implementation
    static IGearLibrary& getDummy();
};
