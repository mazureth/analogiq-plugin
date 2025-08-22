#include "GearLibrary.h"
#include "GearItem.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

GearLibrary::GearLibrary(IFileSystem& fs, ICacheManager& cm)
    : fileSystem(fs)
    , cacheManager(cm)
    , maxGearItems(1000)
    , maxStorageSize(1 * 1024 * 1024 * 1024) // 1GB default
    , autoBackupEnabled(true)
{
    initializeLibraryDirectory();
    loadGearMetadata();
    loadCategories();
}

GearLibrary::~GearLibrary()
{
    saveGearMetadata();
    saveCategories();
}

void GearLibrary::initializeLibraryDirectory()
{
    libraryRootDir = fileSystem.joinPath(fileSystem.getCacheRootDirectory(), "GearLibrary");
    if (!fileSystem.directoryExists(libraryRootDir))
    {
        fileSystem.createDirectory(libraryRootDir);
    }
}

juce::String GearLibrary::generateGearItemPath(const juce::String& gearId)
{
    auto sanitizedId = sanitizeGearId(gearId);
    return fileSystem.joinPath(libraryRootDir, sanitizedId + ".gear");
}

juce::String GearLibrary::generateMetadataPath(const juce::String& gearId)
{
    auto sanitizedId = sanitizeGearId(gearId);
    return fileSystem.joinPath(libraryRootDir, sanitizedId + ".meta");
}

juce::String GearLibrary::generateCategoryPath(const juce::String& categoryName)
{
    return fileSystem.joinPath(libraryRootDir, "categories.txt");
}

juce::String GearLibrary::sanitizeGearId(const juce::String& gearId)
{
    // Remove invalid characters for filenames
    auto sanitized = gearId.replaceCharacters("<>:\"/\\|?*", "_");
    return sanitized.trim();
}

bool GearLibrary::addGearItem(const GearItem& gearItem)
{
    if (gearItem.unitId.isEmpty()) return false;

    // Check storage limits
    if (!checkStorageLimits()) return false;

    auto gearId = gearItem.unitId;
    auto gearPath = generateGearItemPath(gearId);
    auto metadataPath = generateMetadataPath(gearId);

    // For now, we'll create a simple serialization
    // In a real implementation, this would serialize the GearItem properly
    juce::MemoryBlock gearData;
    juce::MemoryOutputStream stream(gearData, false);
    
    // Write basic gear information
    stream.writeString(gearItem.unitId + "\n");
    stream.writeString(gearItem.name + "\n");
    stream.writeString(gearItem.manufacturer + "\n");
    stream.writeString(juce::String(static_cast<int>(gearItem.type)) + "\n");
    stream.writeString(juce::String(static_cast<int>(gearItem.category)) + "\n");

    if (fileSystem.writeFile(gearPath, gearData))
    {
        // Create metadata
        GearMetadata metadata;
        metadata.id = gearId;
        metadata.name = gearItem.name;
        metadata.manufacturer = gearItem.manufacturer;
        metadata.category = "";
        metadata.creationTime = juce::Time::getCurrentTime();
        metadata.lastModifiedTime = metadata.creationTime;
        metadata.fileSize = gearData.getSize();
        metadata.isValid = true;
        metadata.author = "";
        metadata.type = gearItem.type;
        metadata.gearCategory = gearItem.category;

        gearMetadata[gearId] = metadata;
        
        // Create a copy of the gear item in memory
        gearItems[gearId] = std::make_unique<GearItem>(gearItem);
        
        saveGearMetadata();
        return true;
    }
    
    return false;
}

bool GearLibrary::removeGearItem(const juce::String& gearId)
{
    if (gearId.isEmpty() || !gearItemExists(gearId)) return false;

    auto gearPath = generateGearItemPath(gearId);
    auto metadataPath = generateMetadataPath(gearId);

    bool success = true;
    success &= fileSystem.deleteFile(gearPath);
    success &= fileSystem.deleteFile(metadataPath);

    if (success)
    {
        gearItems.erase(gearId);
        gearMetadata.erase(gearId);
        saveGearMetadata();
    }

    return success;
}

bool GearLibrary::updateGearItem(const GearItem& gearItem)
{
    if (gearItem.unitId.isEmpty() || !gearItemExists(gearItem.unitId)) return false;

    // Remove existing and add updated
    if (removeGearItem(gearItem.unitId))
    {
        return addGearItem(gearItem);
    }
    
    return false;
}

GearItem* GearLibrary::getGearItem(const juce::String& gearId)
{
    if (gearId.isEmpty()) return nullptr;
    
    auto it = gearItems.find(gearId);
    if (it != gearItems.end())
        return it->second.get();
    
    return nullptr;
}

const GearItem* GearLibrary::getGearItem(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return nullptr;
    
    auto it = gearItems.find(gearId);
    if (it != gearItems.end())
        return it->second.get();
    
    return nullptr;
}

bool GearLibrary::gearItemExists(const juce::String& gearId)
{
    if (gearId.isEmpty()) return false;
    auto gearPath = generateGearItemPath(gearId);
    return fileSystem.fileExists(gearPath);
}

juce::Array<GearItem*> GearLibrary::getAllGearItems()
{
    juce::Array<GearItem*> items;
    for (auto& gear : gearItems)
    {
        if (gear.second)
            items.add(gear.second.get());
    }
    return items;
}

juce::Array<const GearItem*> GearLibrary::getAllGearItems() const
{
    juce::Array<const GearItem*> items;
    for (auto& gear : gearItems)
    {
        if (gear.second)
            items.add(gear.second.get());
    }
    return items;
}

int GearLibrary::getTotalGearItemCount() const
{
    return static_cast<int>(gearItems.size());
}

void GearLibrary::clearAllGearItems()
{
    for (auto& gear : gearItems)
    {
        auto gearPath = generateGearItemPath(gear.first);
        fileSystem.deleteFile(gearPath);
    }
    
    gearItems.clear();
    gearMetadata.clear();
    saveGearMetadata();
}

bool GearLibrary::createGearCategory(const juce::String& categoryName)
{
    if (categoryName.isEmpty()) return false;

    if (categories.find(categoryName) != categories.end())
        return true; // Already exists

    GearCategory category;
    category.name = categoryName;
    category.creationTime = juce::Time::getCurrentTime();
    categories[categoryName] = category;
    
    saveCategories();
    return true;
}

bool GearLibrary::deleteGearCategory(const juce::String& categoryName)
{
    if (categoryName.isEmpty()) return false;

    auto it = categories.find(categoryName);
    if (it == categories.end()) return false;

    // Remove category from all gear items
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.category == categoryName)
            metadata.second.category = "";
    }

    categories.erase(it);
    saveCategories();
    saveGearMetadata();
    
    return true;
}

juce::StringArray GearLibrary::getGearCategories() const
{
    juce::StringArray categoryNames;
    for (auto& category : categories)
    {
        categoryNames.add(category.first);
    }
    return categoryNames;
}

bool GearLibrary::assignGearToCategory(const juce::String& gearId, const juce::String& categoryName)
{
    if (gearId.isEmpty() || categoryName.isEmpty()) return false;
    if (categories.find(categoryName) == categories.end()) return false;
    if (gearMetadata.find(gearId) == gearMetadata.end()) return false;

    gearMetadata[gearId].category = categoryName;
    categories[categoryName].gearIds.addIfNotAlreadyThere(gearId);
    
    saveGearMetadata();
    saveCategories();
    return true;
}

juce::Array<const GearItem*> GearLibrary::getGearItemsInCategory(const juce::String& categoryName) const
{
    if (categoryName.isEmpty()) return juce::Array<const GearItem*>();
    
    auto it = categories.find(categoryName);
    if (it == categories.end()) return juce::Array<const GearItem*>();
    
    juce::Array<const GearItem*> items;
    for (auto& gearId : it->second.gearIds)
    {
        if (auto gear = getGearItem(gearId))
            items.add(gear);
    }
    
    return items;
}

juce::String GearLibrary::getGearCategory(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return "";
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
        return it->second.category;
    
    return "";
}

juce::Array<const GearItem*> GearLibrary::searchGearItems(const juce::String& searchTerm) const
{
    if (searchTerm.isEmpty()) return getAllGearItems();
    
    juce::Array<const GearItem*> results;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid)
        {
            auto& meta = metadata.second;
            if (meta.name.containsIgnoreCase(searchTerm) ||
                meta.manufacturer.containsIgnoreCase(searchTerm) ||
                meta.author.containsIgnoreCase(searchTerm))
            {
                if (auto gear = getGearItem(meta.id))
                    results.add(gear);
            }
        }
    }
    return results;
}

juce::Array<const GearItem*> GearLibrary::filterGearByType(GearItem::GearType type) const
{
    juce::Array<const GearItem*> results;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid && metadata.second.type == type)
        {
            if (auto gear = getGearItem(metadata.second.id))
                results.add(gear);
        }
    }
    return results;
}

juce::Array<const GearItem*> GearLibrary::filterGearByCategory(GearItem::GearCategory category) const
{
    juce::Array<const GearItem*> results;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid && metadata.second.gearCategory == category)
        {
            if (auto gear = getGearItem(metadata.second.id))
                results.add(gear);
        }
    }
    return results;
}

juce::Array<const GearItem*> GearLibrary::filterGearByManufacturer(const juce::String& manufacturer) const
{
    if (manufacturer.isEmpty()) return getAllGearItems();
    
    juce::Array<const GearItem*> results;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid && metadata.second.manufacturer.containsIgnoreCase(manufacturer))
        {
            if (auto gear = getGearItem(metadata.second.id))
                results.add(gear);
        }
    }
    return results;
}

juce::Array<const GearItem*> GearLibrary::filterGearByDateRange(const juce::Time& startDate, const juce::Time& endDate) const
{
    juce::Array<const GearItem*> results;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid)
        {
            auto creationTime = metadata.second.creationTime;
            if (creationTime >= startDate && creationTime <= endDate)
            {
                if (auto gear = getGearItem(metadata.second.id))
                    results.add(gear);
            }
        }
    }
    return results;
}

bool GearLibrary::validateGearItem(const juce::String& gearId)
{
    if (gearId.isEmpty()) return false;
    
    auto gearPath = generateGearItemPath(gearId);
    return validateGearItemFile(gearPath);
}

bool GearLibrary::isGearItemCorrupted(const juce::String& gearId)
{
    return !validateGearItem(gearId);
}

bool GearLibrary::repairGearItem(const juce::String& gearId)
{
    // For now, just mark as invalid - actual repair logic would be more complex
    if (gearId.isEmpty()) return false;
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
    {
        it->second.isValid = false;
        saveGearMetadata();
        return true;
    }
    
    return false;
}

juce::StringArray GearLibrary::getCorruptedGearItems() const
{
    juce::StringArray corrupted;
    for (auto& metadata : gearMetadata)
    {
        if (!metadata.second.isValid)
            corrupted.add(metadata.second.id);
    }
    return corrupted;
}

bool GearLibrary::exportGearItem(const juce::String& gearId, const juce::String& exportPath)
{
    if (gearId.isEmpty() || !gearItemExists(gearId)) return false;
    
    auto gearPath = generateGearItemPath(gearId);
    auto gearData = fileSystem.readBinaryFile(gearPath);
    
    if (gearData.getSize() == 0) return false;
    
    return fileSystem.writeFile(exportPath, gearData);
}

bool GearLibrary::importGearItem(const juce::String& importPath, const juce::String& gearId)
{
    if (importPath.isEmpty()) return false;
    
    auto gearData = fileSystem.readBinaryFile(importPath);
    if (gearData.getSize() == 0) return false;
    
    // This is a simplified import - would need proper GearItem deserialization
    auto finalGearId = gearId.isEmpty() ? 
                      fileSystem.getFileName(importPath).replaceCharacters(".gear", "") : 
                      gearId;
    
    auto gearPath = generateGearItemPath(finalGearId);
    
    if (fileSystem.writeFile(gearPath, gearData))
    {
        // Create metadata for imported gear
        GearMetadata metadata;
        metadata.id = finalGearId;
        metadata.creationTime = juce::Time::getCurrentTime();
        metadata.lastModifiedTime = metadata.creationTime;
        metadata.fileSize = gearData.getSize();
        metadata.isValid = true;
        
        gearMetadata[finalGearId] = metadata;
        saveGearMetadata();
        
        return true;
    }
    
    return false;
}

bool GearLibrary::exportGearLibrary(const juce::String& exportPath)
{
    if (exportPath.isEmpty()) return false;
    
    // Create a simple library format
    juce::MemoryBlock libraryData;
    juce::MemoryOutputStream libraryStream(libraryData, false);
    
    // Write library header
    libraryStream.writeString("AnalogIQ Gear Library\n");
    libraryStream.writeString("Version: 1.0\n");
    libraryStream.writeString("Gear Count: " + juce::String(getTotalGearItemCount()) + "\n");
    libraryStream.writeString("---\n");
    
    // Write each gear item
    for (auto& gear : gearItems)
    {
        if (gear.second)
        {
            auto gearPath = generateGearItemPath(gear.first);
            auto gearData = fileSystem.readBinaryFile(gearPath);
            
            libraryStream.writeString("GEAR: " + gear.first + "\n");
            libraryStream.writeString("SIZE: " + juce::String(gearData.getSize()) + "\n");
            libraryStream.write(gearData.getData(), gearData.getSize());
            libraryStream.writeString("\n---\n");
        }
    }
    
    return fileSystem.writeFile(exportPath, libraryData);
}

bool GearLibrary::importGearLibrary(const juce::String& importPath)
{
    // This is a simplified import - would need more robust parsing in production
    if (importPath.isEmpty()) return false;
    
    auto libraryData = fileSystem.readBinaryFile(importPath);
    if (libraryData.getSize() == 0) return false;
    
    auto libraryContent = libraryData.toString();
    auto lines = juce::StringArray::fromLines(libraryContent);
    
    // Simple parsing - look for gear markers
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].startsWith("GEAR: "))
        {
            auto gearId = lines[i].substring(6).trim();
            // Would need more sophisticated parsing to extract gear data
            // For now, just create a placeholder
            createGearCategory("Imported");
            assignGearToCategory(gearId, "Imported");
        }
    }
    
    return true;
}

bool GearLibrary::exportGearCategory(const juce::String& categoryName, const juce::String& exportPath)
{
    if (categoryName.isEmpty() || exportPath.isEmpty()) return false;
    
    auto gearItems = getGearItemsInCategory(categoryName);
    if (gearItems.isEmpty()) return false;
    
    // Create a simple category export format
    juce::MemoryBlock categoryData;
    juce::MemoryOutputStream categoryStream(categoryData, false);
    
    // Write category header
    categoryStream.writeString("AnalogIQ Gear Category: " + categoryName + "\n");
    categoryStream.writeString("Version: 1.0\n");
    categoryStream.writeString("Gear Count: " + juce::String(gearItems.size()) + "\n");
    categoryStream.writeString("---\n");
    
    // Write each gear item in the category
    for (auto& gear : gearItems)
    {
        if (gear)
        {
            auto gearPath = generateGearItemPath(gear->unitId);
            auto gearData = fileSystem.readBinaryFile(gearPath);
            
            categoryStream.writeString("GEAR: " + gear->unitId + "\n");
            categoryStream.writeString("SIZE: " + juce::String(gearData.getSize()) + "\n");
            categoryStream.write(gearData.getData(), gearData.getSize());
            categoryStream.writeString("\n---\n");
        }
    }
    
    return fileSystem.writeFile(exportPath, categoryData);
}

juce::int64 GearLibrary::getTotalGearStorageSize() const
{
    juce::int64 totalSize = 0;
    for (auto& metadata : gearMetadata)
    {
        if (metadata.second.isValid)
            totalSize += metadata.second.fileSize;
    }
    return totalSize;
}

juce::int64 GearLibrary::getGearItemSize(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return 0;
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
        return it->second.fileSize;
    
    return 0;
}

juce::Time GearLibrary::getGearItemCreationTime(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return juce::Time();
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
        return it->second.creationTime;
    
    return juce::Time();
}

juce::Time GearLibrary::getGearItemLastModifiedTime(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return juce::Time();
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
        return it->second.lastModifiedTime;
    
    return juce::Time();
}

juce::String GearLibrary::getGearItemAuthor(const juce::String& gearId) const
{
    if (gearId.isEmpty()) return "";
    
    auto it = gearMetadata.find(gearId);
    if (it != gearMetadata.end())
        return it->second.author;
    
    return "";
}

bool GearLibrary::createGearLibraryBackup(const juce::String& backupPath)
{
    if (backupPath.isEmpty()) return false;
    
    return exportGearLibrary(backupPath);
}

bool GearLibrary::restoreGearLibraryFromBackup(const juce::String& backupPath)
{
    return importGearLibrary(backupPath);
}

juce::StringArray GearLibrary::getAvailableBackups() const
{
    auto backupDir = fileSystem.joinPath(libraryRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
        return juce::StringArray();
    
    auto files = fileSystem.getFiles(backupDir);
    juce::StringArray backups;
    for (auto& file : files)
    {
        if (file.endsWith(".backup"))
            backups.add(fileSystem.getFileName(file));
    }
    return backups;
}

bool GearLibrary::optimizeGearLibrary()
{
    // Remove corrupted items
    auto corrupted = getCorruptedGearItems();
    for (auto& gearId : corrupted)
    {
        removeGearItem(gearId);
    }
    
    // Check storage limits
    checkStorageLimits();
    
    return true;
}

bool GearLibrary::validateGearLibraryIntegrity()
{
    bool allValid = true;
    
    for (auto& metadata : gearMetadata)
    {
        if (!validateGearItem(metadata.first))
        {
            metadata.second.isValid = false;
            allValid = false;
        }
    }
    
    if (!allValid)
        saveGearMetadata();
    
    return allValid;
}

bool GearLibrary::syncWithRemoteLibrary(const juce::String& remoteUrl)
{
    // Placeholder for remote synchronization
    // Would implement actual network sync logic here
    return false;
}

bool GearLibrary::uploadGearItem(const juce::String& gearId, const juce::String& remoteUrl)
{
    // Placeholder for remote upload
    return false;
}

bool GearLibrary::downloadGearItem(const juce::String& gearId, const juce::String& remoteUrl)
{
    // Placeholder for remote download
    return false;
}

bool GearLibrary::checkForUpdates(const juce::String& remoteUrl)
{
    // Placeholder for update checking
    return false;
}

void GearLibrary::setMaxGearItems(int maxItems)
{
    maxGearItems = maxItems;
    checkStorageLimits();
}

int GearLibrary::getMaxGearItems() const
{
    return maxGearItems;
}

void GearLibrary::setMaxStorageSize(juce::int64 maxSize)
{
    maxStorageSize = maxSize;
    checkStorageLimits();
}

juce::int64 GearLibrary::getMaxStorageSize() const
{
    return maxStorageSize;
}

void GearLibrary::setAutoBackupEnabled(bool enabled)
{
    autoBackupEnabled = enabled;
}

bool GearLibrary::isAutoBackupEnabled() const
{
    return autoBackupEnabled;
}

void GearLibrary::loadGearMetadata()
{
    auto metadataIndexPath = fileSystem.joinPath(libraryRootDir, "gear_index.txt");
    if (fileSystem.fileExists(metadataIndexPath))
    {
        auto content = fileSystem.readFile(metadataIndexPath);
        auto lines = juce::StringArray::fromLines(content);
        
        for (auto& line : lines)
        {
            auto parts = juce::StringArray::fromTokens(line, "|", "");
            if (parts.size() >= 11)
            {
                GearMetadata metadata;
                metadata.id = parts[0];
                metadata.name = parts[1];
                metadata.manufacturer = parts[2];
                metadata.category = parts[3];
                metadata.creationTime = juce::Time(parts[4].getLargeIntValue());
                metadata.lastModifiedTime = juce::Time(parts[5].getLargeIntValue());
                metadata.fileSize = parts[6].getLargeIntValue();
                metadata.isValid = parts[7].getIntValue() != 0;
                metadata.author = parts[8];
                metadata.type = static_cast<GearItem::GearType>(parts[9].getIntValue());
                metadata.gearCategory = static_cast<GearItem::GearCategory>(parts[10].getIntValue());
                
                gearMetadata[metadata.id] = metadata;
            }
        }
    }
}

void GearLibrary::saveGearMetadata()
{
    auto metadataIndexPath = fileSystem.joinPath(libraryRootDir, "gear_index.txt");
    juce::String content;
    
    for (auto& gear : gearMetadata)
    {
        auto& metadata = gear.second;
        content += metadata.id + "|" +
                  metadata.name + "|" +
                  metadata.manufacturer + "|" +
                  metadata.category + "|" +
                  juce::String(metadata.creationTime.toMilliseconds()) + "|" +
                  juce::String(metadata.lastModifiedTime.toMilliseconds()) + "|" +
                  juce::String(metadata.fileSize) + "|" +
                  juce::String(metadata.isValid ? 1 : 0) + "|" +
                  metadata.author + "|" +
                  juce::String(static_cast<int>(metadata.type)) + "|" +
                  juce::String(static_cast<int>(metadata.gearCategory)) + "\n";
    }
    
    fileSystem.writeFile(metadataIndexPath, content);
}

void GearLibrary::loadCategories()
{
    auto categoryPath = generateCategoryPath("");
    if (fileSystem.fileExists(categoryPath))
    {
        auto content = fileSystem.readFile(categoryPath);
        auto lines = juce::StringArray::fromLines(content);
        
        for (auto& line : lines)
        {
            auto parts = juce::StringArray::fromTokens(line, "|", "");
            if (parts.size() >= 4)
            {
                GearCategory category;
                category.name = parts[0];
                category.creationTime = juce::Time(parts[1].getLargeIntValue());
                category.description = parts[2];
                
                auto gearIds = juce::StringArray::fromTokens(parts[3], ",", "");
                category.gearIds = gearIds;
                
                categories[category.name] = category;
            }
        }
    }
}

void GearLibrary::saveCategories()
{
    auto categoryPath = generateCategoryPath("");
    juce::String content;
    
    for (auto& category : categories)
    {
        auto& cat = category.second;
        content += cat.name + "|" +
                  juce::String(cat.creationTime.toMilliseconds()) + "|" +
                  cat.description + "|" +
                  cat.gearIds.joinIntoString(",") + "\n";
    }
    
    fileSystem.writeFile(categoryPath, content);
}

bool GearLibrary::validateGearItemFile(const juce::String& gearPath)
{
    if (!fileSystem.fileExists(gearPath)) return false;
    
    auto gearData = fileSystem.readBinaryFile(gearPath);
    if (gearData.getSize() == 0) return false;
    
    // Simple validation - check if file contains expected format
    auto content = gearData.toString();
    auto lines = juce::StringArray::fromLines(content);
    
    // Basic format check
    return lines.size() >= 5; // At least unitId, name, manufacturer, type, category
}

juce::String GearLibrary::generateBackupPath()
{
    auto backupDir = fileSystem.joinPath(libraryRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
    {
        fileSystem.createDirectory(backupDir);
    }
    
    auto timestamp = juce::Time::getCurrentTime();
    auto backupName = "backup_" + timestamp.formatted("%Y%m%d_%H%M%S") + ".backup";
    return fileSystem.joinPath(backupDir, backupName);
}

bool GearLibrary::createBackupDirectory()
{
    auto backupDir = fileSystem.joinPath(libraryRootDir, "Backups");
    if (!fileSystem.directoryExists(backupDir))
    {
        return fileSystem.createDirectory(backupDir);
    }
    return true;
}

void GearLibrary::cleanupLibrary()
{
    // Remove corrupted items
    auto corrupted = getCorruptedGearItems();
    for (auto& gearId : corrupted)
    {
        removeGearItem(gearId);
    }
    
    // Check storage limits
    checkStorageLimits();
}

bool GearLibrary::checkStorageLimits()
{
    bool needsCleanup = false;
    
    // Check item count limit
    if (getTotalGearItemCount() > maxGearItems)
    {
        needsCleanup = true;
    }
    
    // Check storage size limit
    if (getTotalGearStorageSize() > maxStorageSize)
    {
        needsCleanup = true;
    }
    
    if (needsCleanup)
    {
        cleanupLibrary();
        return false; // Still over limit after cleanup
    }
    
    return true;
}
