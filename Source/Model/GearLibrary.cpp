#include "GearLibrary.h"
#include "GearItem.h"
#include "../Shared/INetworkFetcher.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/RemoteConfig.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

GearLibrary::GearLibrary(IFileSystem &fs, ICacheManager &cm, INetworkFetcher &nf)
    : fileSystem(fs), cacheManager(cm), networkFetcher(nf), maxGearItems(1000), maxStorageSize(1 * 1024 * 1024 * 1024) // 1GB default
      ,
      autoBackupEnabled(true), initialized(false)
{
    // Lazy initialization - no file system or network operations during construction
}

GearLibrary::~GearLibrary()
{
    saveGearMetadata();
    saveCategories();
}

void GearLibrary::initializeLazy()
{
    if (initialized)
        return;

    try
    {
        initializeLibraryDirectory();

        loadGearMetadata();

        loadCategories();

        loadRemoteGearLibrary(); // Load gear from remote source (no fallback items)

        initialized = true;
    }
    catch (...)
    {
        // If initialization fails, continue with empty library
        // This prevents crashes when file system is not available
    }
}

void GearLibrary::initializeLibraryDirectory()
{
    try
    {
        libraryRootDir = fileSystem.joinPath(fileSystem.getCacheRootDirectory(), "GearLibrary");

        if (!fileSystem.directoryExists(libraryRootDir))
        {
            if (!fileSystem.createDirectory(libraryRootDir))
            {
                // If we can't create the directory, use a fallback path
                libraryRootDir = "/tmp/analogiq_gear_library";
            }
            else
            {
            }
        }
        else
        {
        }
    }
    catch (...)
    {
        // Use fallback path if anything goes wrong
        libraryRootDir = "/tmp/analogiq_gear_library";
    }
}

void GearLibrary::loadRemoteGearLibrary()
{

    // Load gear from remote GitHub repository using configuration
    juce::String remoteUrl = RemoteConfig::getGearLibraryIndexUrl();

    // Step 1: Try to load from local cache first
    auto cachedData = cacheManager.getCachedPath("remote_gear_library");
    if (!cachedData.isEmpty())
    {
        if (loadGearFromCache(cachedData))
        {
        }
        else
        {
            gearItems.clear();
        }
    }
    else
    {
        gearItems.clear();
    }

    // Step 2: Async remote fetch to update cache and add new/changed gear
    fetchRemoteGearAsync(remoteUrl);
}

void GearLibrary::createSampleGearItems()
{
    // Create sample gear items for development and testing
    // These will be replaced by real remote data when NetworkFetcher is integrated

    // Sample EQ
    auto eq = std::make_unique<GearItem>();
    eq->unitId = "eq_500_series";
    eq->name = "500 Series EQ";
    eq->manufacturer = "AnalogIQ";
    eq->type = GearItem::GearType::Series500;
    eq->category = GearItem::GearCategory::EQ;
    eq->categoryString = "EQ";
    eq->version = "1.0.0";
    eq->description = "Professional 500 series equalizer";

    // Add sample controls
    GearControl eqControl;
    eqControl.type = GearControl::ControlType::Knob;
    eqControl.name = "Frequency";
    eqControl.position = juce::Rectangle<float>(50, 30, 40, 40);
    eqControl.currentValue = 0.5f;
    eqControl.initialValue = 0.5f;
    eq->controls.add(eqControl);

    gearItems.add(eq.release());

    // Sample Preamp
    auto preamp = std::make_unique<GearItem>();
    preamp->unitId = "preamp_tube";
    preamp->name = "Tube Preamp";
    preamp->manufacturer = "AnalogIQ";
    preamp->type = GearItem::GearType::Rack19Inch;
    preamp->category = GearItem::GearCategory::Preamp;
    preamp->categoryString = "Preamp";
    preamp->version = "1.0.0";
    preamp->description = "Professional tube preamplifier";

    // Add sample controls
    GearControl preampControl;
    preampControl.type = GearControl::ControlType::Fader;
    preampControl.name = "Gain";
    preampControl.position = juce::Rectangle<float>(50, 30, 20, 60);
    preampControl.currentValue = 0.3f;
    preampControl.initialValue = 0.3f;
    preamp->controls.add(preampControl);

    gearItems.add(preamp.release());

    // Sample Compressor
    auto compressor = std::make_unique<GearItem>();
    compressor->unitId = "compressor_vca";
    compressor->name = "VCA Compressor";
    compressor->manufacturer = "AnalogIQ";
    compressor->type = GearItem::GearType::Rack19Inch;
    compressor->category = GearItem::GearCategory::Compressor;
    compressor->categoryString = "Compressor";
    compressor->version = "1.0.0";
    compressor->description = "Professional VCA compressor";

    // Add sample controls
    GearControl compControl;
    compControl.type = GearControl::ControlType::Switch;
    compControl.name = "Ratio";
    compControl.position = juce::Rectangle<float>(50, 30, 30, 20);
    compControl.currentValue = 0.0f;
    compControl.initialValue = 0.0f;
    compControl.options = {"2:1", "4:1", "8:1", "20:1"};
    compControl.currentIndex = 0;
    compressor->controls.add(compControl);

    gearItems.add(compressor.release());

    // Update metadata and categories
    updateGearMetadata();
    updateCategories();
}

void GearLibrary::updateGearMetadata()
{
    gearMetadata.clear();

    for (auto &gearItem : gearItems)
    {
        GearMetadata metadata;
        metadata.id = gearItem->unitId;
        metadata.name = gearItem->name;
        metadata.manufacturer = gearItem->manufacturer;
        metadata.category = gearItem->categoryString;
        metadata.creationTime = juce::Time::getCurrentTime();
        metadata.lastModifiedTime = juce::Time::getCurrentTime();
        metadata.fileSize = 0; // Will be calculated if needed
        metadata.isValid = true;
        metadata.author = "AnalogIQ";
        metadata.type = gearItem->type;
        metadata.gearCategory = gearItem->category;

        gearMetadata[metadata.id] = metadata;
    }
}

void GearLibrary::updateCategories()
{
    categories.clear();

    // Group gear items by category
    std::map<juce::String, juce::StringArray> categoryGroups;

    for (auto &gearItem : gearItems)
    {
        auto category = gearItem->categoryString;
        if (category.isEmpty())
            category = "Other";

        if (categoryGroups.find(category) == categoryGroups.end())
        {
            categoryGroups[category] = juce::StringArray();
        }
        categoryGroups[category].add(gearItem->unitId);
    }

    // Create category objects
    for (auto &[categoryName, gearIds] : categoryGroups)
    {
        GearCategory category;
        category.name = categoryName;
        category.gearIds = gearIds;
        category.creationTime = juce::Time::getCurrentTime();
        category.description = "Gear items in " + categoryName + " category";

        categories[categoryName] = category;
    }
}

void GearLibrary::createCategoriesSection()
{
    // Create default categories if none exist
    if (categories.empty())
    {

        // Create EQ category
        GearCategory eqCategory;
        eqCategory.name = "EQ";
        eqCategory.creationTime = juce::Time::getCurrentTime();
        eqCategory.description = "Equalizers and filters";
        categories["EQ"] = eqCategory;

        // Create Compressor category
        GearCategory compCategory;
        compCategory.name = "Compressor";
        compCategory.creationTime = juce::Time::getCurrentTime();
        compCategory.description = "Dynamic processors and compressors";
        categories["Compressor"] = compCategory;

        // Create Preamp category
        GearCategory preampCategory;
        preampCategory.name = "Preamp";
        preampCategory.creationTime = juce::Time::getCurrentTime();
        preampCategory.description = "Preamplifiers and gain stages";
        categories["Preamp"] = preampCategory;
    }
}

// Helper methods for remote gear loading
bool GearLibrary::loadGearFromCache(const juce::String &cachePath)
{

    try
    {
        juce::File cacheFile(cachePath);
        if (!cacheFile.existsAsFile())
        {
            return false;
        }

        juce::String jsonContent = cacheFile.loadFileAsString();
        if (jsonContent.isEmpty())
        {
            return false;
        }

        auto json = juce::JSON::parse(jsonContent);
        if (!json.isObject())
        {
            return false;
        }

        // Check if we have a "units" array in the new format (matching legacy system)
        if (!json.hasProperty("units") || !json["units"].isArray())
        {
            return false;
        }

        // Clear existing items and load from cache
        gearItems.clear();
        auto gearArray = json["units"].getArray();

        for (auto &gearObject : *gearArray)
        {
            if (gearObject.isObject())
            {
                auto gearItem = parseGearFromJson(gearObject);

                if (gearItem)
                {
                    gearItems.add(gearItem.release());
                }
                else
                {
                }
            }
            else
            {
            }
        }

        // Refresh all thumbnails after loading from cache
        refreshAllThumbnails();

        return true;
    }
    catch (...)
    {
        return false;
    }
}

void GearLibrary::fetchRemoteGearAsync(const juce::String &remoteUrl)
{

    // Use the injected NetworkFetcher to fetch remote data
    try
    {
        bool success = false;
        auto jsonData = networkFetcher.fetchRemoteGearLibrary(juce::URL(remoteUrl), success);
        if (!jsonData.isEmpty())
        {

            // Parse the remote data
            auto json = juce::JSON::parse(jsonData);
            if (!json.isObject())
            {
                return;
            }

            // Check if we have a "units" array in the new format (matching legacy system)
            if (!json.hasProperty("units") || !json["units"].isArray())
            {
                return;
            }

            auto gearArray = json["units"].getArray();

            // Cache the remote data
            cacheManager.cacheData("remote_gear_library", jsonData);

            // Parse and add new gear items
            int newItemsCount = 0;
            for (auto &gearObject : *gearArray)
            {
                if (auto gearItem = parseGearFromJson(gearObject))
                {
                    // Check if this gear item already exists
                    bool exists = false;
                    for (auto &existingItem : gearItems)
                    {
                        if (existingItem->unitId == gearItem->unitId)
                        {
                            exists = true;
                            break;
                        }
                    }

                    if (!exists)
                    {
                        gearItems.add(gearItem.release());
                        newItemsCount++;
                    }
                }
            }

            // Update metadata and categories
            updateGearMetadata();
            updateCategories();

            // Notify listeners that the library has been updated
            // This will trigger a repaint of the UI
            if (onLibraryUpdated)
            {
                onLibraryUpdated();
            }
        }
        else
        {
        }
    }
    catch (...)
    {
    }
}

std::unique_ptr<GearItem> GearLibrary::parseGearFromJson(const juce::var &gearObject)
{

    if (!gearObject.isObject())
    {
        return nullptr;
    }

    auto obj = gearObject.getDynamicObject();
    if (!obj)
    {
        return nullptr;
    }

    auto gearItem = std::make_unique<GearItem>();
    gearItem->setFileSystem(&fileSystem);
    gearItem->setNetworkFetcher(&networkFetcher);
    gearItem->setCacheManager(&cacheManager);

    // Extract properties using the new format (copying legacy system exactly)
    juce::String unitId = obj->getProperty("unitId");
    juce::String name = obj->getProperty("name");
    juce::String manufacturer = obj->getProperty("manufacturer");
    juce::String category = obj->getProperty("category");
    juce::String version = obj->getProperty("version");
    juce::String schemaPath = obj->getProperty("schemaPath");
    juce::String thumbnailImage = obj->getProperty("thumbnailImage");

    // Debug: Log the extracted properties

    // Process tags with explicit cleanup
    juce::StringArray tags;
    if (obj->hasProperty("tags") && obj->getProperty("tags").isArray())
    {
        auto tagsArray = obj->getProperty("tags").getArray();
        for (auto &tag : *tagsArray)
        {
            tags.add(tag.toString());
        }
        // Clear the temporary array reference to release memory
        tagsArray = nullptr;
    }

    // Determine slotSize (default to 1)
    int slotSize = obj->hasProperty("slotSize") ? static_cast<int>(obj->getProperty("slotSize")) : 1;

    // Create empty controls array (we'll populate this later when loading the full schema)
    juce::Array<GearControl> controls;

    // Set the properties on our gear item
    gearItem->unitId = unitId;
    gearItem->name = name;
    gearItem->manufacturer = manufacturer;
    gearItem->categoryString = category;
    gearItem->version = version;
    gearItem->schemaPath = schemaPath;
    gearItem->imageUrl = thumbnailImage; // Set the imageUrl for thumbnail caching
    gearItem->tags = tags;

    // Map category string to enum (copying legacy system exactly)
    if (category == "equalizer" || category == "eq")
        gearItem->category = GearItem::GearCategory::EQ;
    else if (category == "compressor")
        gearItem->category = GearItem::GearCategory::Compressor;
    else if (category == "preamp")
        gearItem->category = GearItem::GearCategory::Preamp;
    else
        gearItem->category = GearItem::GearCategory::Other;

    // Try to determine type from tags (copying legacy system exactly)
    gearItem->type = GearItem::GearType::Other;
    if (tags.contains("500 series"))
        gearItem->type = GearItem::GearType::Series500;
    else if (tags.contains("rack") || tags.contains("19 inch"))
        gearItem->type = GearItem::GearType::Rack19Inch;

    // Parse controls if they exist (simplified for now)
    if (obj->hasProperty("controls") && obj->getProperty("controls").isArray())
    {
        // TODO: Implement controls parsing when needed
    }

    // Load thumbnail if imageUrl is available
    if (!gearItem->imageUrl.isEmpty())
    {
        loadGearThumbnail(gearItem.get());
    }

    // Note: Full schema loading (faceplate and controls) is deferred until the unit is dropped onto the rack
    // This prevents loading full schemas for all units during initial GearLibrary initialization

    return gearItem;
}

// Additional helper methods that were removed
juce::String GearLibrary::generateGearItemPath(const juce::String &gearId)
{
    return fileSystem.joinPath(libraryRootDir, "gear/" + gearId + ".json");
}

juce::String GearLibrary::generateMetadataPath(const juce::String &gearId)
{
    return fileSystem.joinPath(libraryRootDir, "metadata/" + gearId + ".txt");
}

juce::String GearLibrary::generateCategoryPath(const juce::String &categoryName)
{
    return fileSystem.joinPath(libraryRootDir, "categories/" + categoryName + ".txt");
}

juce::String GearLibrary::sanitizeGearId(const juce::String &gearId)
{
    // Remove invalid characters for file names
    juce::String sanitized = gearId;
    sanitized = sanitized.replaceCharacters("\\/:*?\"<>|", "_");
    return sanitized;
}

void GearLibrary::setRemoteLibraryUrl(const juce::String &url)
{
    remoteLibraryInfo.url = url;
    // Clear cache to force refresh on next load
    cacheManager.clearCache("remote_gear_library");
}

bool GearLibrary::addGearItem(const GearItem &gearItem)
{
    initializeLazy();
    if (gearItem.unitId.isEmpty())
        return false;

    // Check storage limits
    if (!checkStorageLimits())
        return false;

    auto gearId = gearItem.unitId;
    auto gearPath = generateGearItemPath(gearId);
    auto metadataPath = generateMetadataPath(gearId);

    // Create a copy of the gear item
    auto newGearItem = std::make_unique<GearItem>(gearItem);

    // Add to our collection
    gearItems.add(newGearItem.release());

    // Update metadata and categories
    updateGearMetadata();
    updateCategories();

    // Save to disk
    saveGearMetadata();
    saveCategories();

    return true;
}

bool GearLibrary::removeGearItem(const juce::String &gearId)
{
    // Find and remove the gear item
    for (int i = 0; i < gearItems.size(); ++i)
    {
        if (gearItems[i]->unitId == gearId)
        {
            gearItems.remove(i);

            // Update metadata and categories
            updateGearMetadata();
            updateCategories();

            // Save to disk
            saveGearMetadata();
            saveCategories();

            return true;
        }
    }

    return false;
}

bool GearLibrary::updateGearItem(const GearItem &gearItem)
{
    if (gearItem.unitId.isEmpty())
        return false;

    // Find and update the gear item
    for (auto &item : gearItems)
    {
        if (item->unitId == gearItem.unitId)
        {
            *item = gearItem;

            // Update metadata and categories
            updateGearMetadata();
            updateCategories();

            // Save to disk
            saveGearMetadata();
            saveCategories();

            return true;
        }
    }

    return false;
}

GearItem *GearLibrary::getGearItem(const juce::String &gearId)
{
    // Sanitize gearId to remove instance suffix if present
    juce::String sanitizedGearId = gearId;
    int instPos = gearId.indexOf("_inst_");
    if (instPos != -1)
    {
        sanitizedGearId = gearId.substring(0, instPos);
    }

    for (auto &item : gearItems)
    {
        if (item->unitId == sanitizedGearId)
        {
            return item;
        }
    }
    return nullptr;
}

const GearItem *GearLibrary::getGearItem(const juce::String &gearId) const
{

    // Sanitize gearId to remove instance suffix if present
    juce::String sanitizedGearId = gearId;
    int instPos = gearId.indexOf("_inst_");
    if (instPos != -1)
    {
        sanitizedGearId = gearId.substring(0, instPos);
    }

    for (auto &item : gearItems)
    {
        if (item->unitId == sanitizedGearId)
        {
            return item;
        }
    }
    return nullptr;
}

bool GearLibrary::gearItemExists(const juce::String &gearId)
{
    return getGearItem(gearId) != nullptr;
}

juce::Array<GearItem *> GearLibrary::getAllGearItems()
{
    initializeLazy();
    return gearItems;
}

juce::Array<const GearItem *> GearLibrary::getAllGearItems() const
{
    juce::Array<const GearItem *> result;
    for (auto &item : gearItems)
    {
        result.add(item);
    }
    return result;
}

int GearLibrary::getTotalGearItemCount() const
{
    return gearItems.size();
}

void GearLibrary::clearAllGearItems()
{
    gearItems.clear();
    gearMetadata.clear();
    categories.clear();

    saveGearMetadata();
    saveCategories();
}

void GearLibrary::resetInitialization()
{
    initialized = false;
}

bool GearLibrary::createGearCategory(const juce::String &categoryName)
{
    if (categoryName.isEmpty() || categories.find(categoryName) != categories.end())
        return false;

    GearCategory category;
    category.name = categoryName;
    category.creationTime = juce::Time::getCurrentTime();
    category.description = "Category for " + categoryName;

    categories[categoryName] = category;
    saveCategories();

    return true;
}

bool GearLibrary::deleteGearCategory(const juce::String &categoryName)
{
    if (categories.find(categoryName) == categories.end())
        return false;

    categories.erase(categoryName);
    saveCategories();

    return true;
}

juce::StringArray GearLibrary::getGearCategories() const
{
    juce::StringArray result;
    for (auto &[name, category] : categories)
    {
        result.add(name);
    }
    return result;
}

bool GearLibrary::assignGearToCategory(const juce::String &gearId, const juce::String &categoryName)
{
    if (categories.find(categoryName) == categories.end())
        return false;

    auto &category = categories[categoryName];

    // Remove from other categories first
    for (auto &[name, cat] : categories)
    {
        cat.gearIds.removeString(gearId);
    }

    // Add to specified category
    if (!category.gearIds.contains(gearId))
    {
        category.gearIds.add(gearId);
        saveCategories();
    }

    return true;
}

juce::Array<const GearItem *> GearLibrary::getGearItemsInCategory(const juce::String &categoryName) const
{
    juce::Array<const GearItem *> result;

    if (categories.find(categoryName) == categories.end())
        return result;

    auto &category = categories.at(categoryName);

    for (auto &gearId : category.gearIds)
    {
        auto item = getGearItem(gearId);
        if (item)
            result.add(item);
    }

    return result;
}

juce::String GearLibrary::getGearCategory(const juce::String &gearId) const
{
    for (auto &[name, category] : categories)
    {
        if (category.gearIds.contains(gearId))
            return name;
    }
    return "";
}

juce::Array<const GearItem *> GearLibrary::searchGearItems(const juce::String &searchTerm) const
{
    if (searchTerm.isEmpty())
        return getAllGearItems();

    juce::Array<const GearItem *> result;
    auto normalizedSearch = normalizeForSearch(searchTerm);

    for (auto &item : gearItems)
    {
        if (shouldShowItem(item, normalizedSearch))
            result.add(item);
    }

    return result;
}

juce::Array<const GearItem *> GearLibrary::filterGearByType(GearItem::GearType type) const
{
    juce::Array<const GearItem *> result;

    for (auto &item : gearItems)
    {
        if (item->type == type)
            result.add(item);
    }

    return result;
}

juce::Array<const GearItem *> GearLibrary::filterGearByCategory(GearItem::GearCategory category) const
{
    juce::Array<const GearItem *> result;

    for (auto &item : gearItems)
    {
        if (item->category == category)
            result.add(item);
    }

    return result;
}

juce::Array<const GearItem *> GearLibrary::filterGearByManufacturer(const juce::String &manufacturer) const
{
    juce::Array<const GearItem *> result;

    for (auto &item : gearItems)
    {
        if (item->manufacturer.equalsIgnoreCase(manufacturer))
            result.add(item);
    }

    return result;
}

juce::Array<const GearItem *> GearLibrary::filterGearByDateRange(const juce::Time &startDate, const juce::Time &endDate) const
{
    juce::Array<const GearItem *> result;

    for (auto &item : gearItems)
    {
        // For now, we'll use creation time from metadata
        // In a real implementation, this would use actual creation dates
        auto creationTime = juce::Time::getCurrentTime(); // Placeholder

        if (creationTime >= startDate && creationTime <= endDate)
            result.add(item);
    }

    return result;
}

bool GearLibrary::shouldShowItem(const GearItem *item, const juce::String &normalizedSearch) const
{
    if (!item)
        return false;

    // Search in name, manufacturer, category, and tags
    auto normalizedName = normalizeForSearch(item->name);
    auto normalizedManufacturer = normalizeForSearch(item->manufacturer);
    auto normalizedCategory = normalizeForSearch(item->categoryString);

    return normalizedName.contains(normalizedSearch) ||
           normalizedManufacturer.contains(normalizedSearch) ||
           normalizedCategory.contains(normalizedSearch);
}

juce::String GearLibrary::normalizeForSearch(const juce::String &text) const
{
    auto normalized = text.toLowerCase();

    // Remove ignored characters
    for (auto &ignoredChar : getIgnoredCharacters())
    {
        normalized = normalized.replace(ignoredChar, "");
    }

    return normalized.trim();
}

juce::StringArray GearLibrary::getIgnoredCharacters() const
{
    return {"-", " ", "_", ".", "(", ")", "[", "]", "/", "\\", "&", "+", "=", "#"};
}

bool GearLibrary::validateGearItem(const juce::String &gearId)
{
    auto item = getGearItem(gearId);
    if (!item)
        return false;

    // Basic validation
    return !item->unitId.isEmpty() &&
           !item->name.isEmpty() &&
           !item->manufacturer.isEmpty();
}

bool GearLibrary::isGearItemCorrupted(const juce::String &gearId)
{
    // For now, assume no corruption
    // In a real implementation, this would check file integrity
    return false;
}

bool GearLibrary::repairGearItem(const juce::String &gearId)
{
    // For now, just return success
    // In a real implementation, this would attempt repair
    return true;
}

juce::StringArray GearLibrary::getCorruptedGearItems() const
{
    // For now, return empty array
    // In a real implementation, this would scan for corrupted items
    return juce::StringArray();
}

bool GearLibrary::exportGearItem(const juce::String &gearId, const juce::String &exportPath)
{
    auto item = getGearItem(gearId);
    if (!item)
        return false;

    // For now, just return success
    // In a real implementation, this would export the gear item
    return true;
}

bool GearLibrary::importGearItem(const juce::String &importPath, const juce::String &gearId)
{
    // For now, just return success
    // In a real implementation, this would import the gear item
    return true;
}

bool GearLibrary::exportGearLibrary(const juce::String &exportPath)
{
    // For now, just return success
    // In a real implementation, this would export the entire library
    return true;
}

bool GearLibrary::importGearLibrary(const juce::String &importPath)
{
    // For now, just return success
    // In a real implementation, this would import the entire library
    return true;
}

bool GearLibrary::exportGearCategory(const juce::String &categoryName, const juce::String &exportPath)
{
    if (categories.find(categoryName) == categories.end())
        return false;

    // For now, just return success
    // In a real implementation, this would export the category
    return true;
}

juce::int64 GearLibrary::getTotalGearStorageSize() const
{
    // For now, return a placeholder value
    // In a real implementation, this would calculate actual storage size
    return gearItems.size() * 1024; // 1KB per item placeholder
}

juce::int64 GearLibrary::getGearItemSize(const juce::String &gearId) const
{
    // For now, return a placeholder value
    // In a real implementation, this would calculate actual item size
    return 1024; // 1KB placeholder
}

juce::Time GearLibrary::getGearItemCreationTime(const juce::String &gearId) const
{
    auto metadata = gearMetadata.find(gearId);
    if (metadata != gearMetadata.end())
        return metadata->second.creationTime;
    return juce::Time(0);
}

juce::Time GearLibrary::getGearItemLastModifiedTime(const juce::String &gearId) const
{
    auto metadata = gearMetadata.find(gearId);
    if (metadata != gearMetadata.end())
        return metadata->second.lastModifiedTime;
    return juce::Time(0);
}

juce::String GearLibrary::getGearItemAuthor(const juce::String &gearId) const
{
    auto metadata = gearMetadata.find(gearId);
    if (metadata != gearMetadata.end())
        return metadata->second.author;
    return "";
}

bool GearLibrary::createGearLibraryBackup(const juce::String &backupPath)
{
    // For now, just return success
    // In a real implementation, this would create a backup
    return true;
}

bool GearLibrary::restoreGearLibraryFromBackup(const juce::String &backupPath)
{
    // For now, just return success
    // In a real implementation, this would restore from backup
    return true;
}

juce::StringArray GearLibrary::getAvailableBackups() const
{
    // For now, return empty array
    // In a real implementation, this would list available backups
    return juce::StringArray();
}

bool GearLibrary::optimizeGearLibrary()
{
    // For now, just return success
    // In a real implementation, this would optimize the library
    return true;
}

bool GearLibrary::validateGearLibraryIntegrity()
{
    // For now, just return true
    // In a real implementation, this would validate integrity
    return true;
}

bool GearLibrary::syncWithRemoteLibrary(const juce::String &remoteUrl)
{
    // For now, just return success
    // In a real implementation, this would sync with remote
    return true;
}

bool GearLibrary::uploadGearItem(const juce::String &gearId, const juce::String &remoteUrl)
{
    // For now, just return success
    // In a real implementation, this would upload the item
    return true;
}

bool GearLibrary::downloadGearItem(const juce::String &gearId, const juce::String &remoteUrl)
{
    // For now, just return success
    // In a real implementation, this would download the item
    return true;
}

bool GearLibrary::downloadGearAsset(const juce::String &gearId, const juce::String &assetUrl, const juce::String &assetType)
{

    if (assetUrl.isEmpty())
    {
        return false;
    }

    try
    {
        // Construct full URL from relative path based on asset type
        juce::String fullUrl = assetUrl;
        if (!fullUrl.startsWith("http"))
        {
            // Check if the path is already a full path or needs the base URL
            if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
            {
                if (assetType == "control")
                {
                    fullUrl = RemoteConfig::getControlImageUrl(fullUrl);
                }
                else if (assetType == "faceplate")
                {
                    fullUrl = RemoteConfig::getFaceplateUrl(fullUrl);
                }
                else if (assetType == "thumbnail")
                {
                    fullUrl = RemoteConfig::getThumbnailUrl(fullUrl);
                }
                else
                {
                    // Default to control images for unknown types
                    fullUrl = RemoteConfig::getControlImageUrl(fullUrl);
                }
            }
        }

        // Create URL from full path
        juce::URL assetUrlObj(fullUrl);
        if (!assetUrlObj.isWellFormed())
        {
            return false;
        }

        // Download the asset
        auto inputStream = assetUrlObj.createInputStream(false);
        if (inputStream == nullptr)
        {
            return false;
        }

        // Read the binary data
        juce::MemoryBlock binaryData;
        inputStream->readIntoMemoryBlock(binaryData);

        if (binaryData.getSize() == 0)
        {
            return false;
        }

        // Cache the asset
        bool cacheSuccess = cacheManager.cacheBinaryData(assetUrl, binaryData);
        if (!cacheSuccess)
        {
            return false;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool GearLibrary::checkForUpdates(const juce::String &remoteUrl)
{
    // For now, just return false (no updates)
    // In a real implementation, this would check for updates
    return false;
}

void GearLibrary::setMaxGearItems(int maxItems)
{
    maxGearItems = maxItems;
}

int GearLibrary::getMaxGearItems() const
{
    return maxGearItems;
}

void GearLibrary::setMaxStorageSize(juce::int64 maxSize)
{
    maxStorageSize = maxSize;
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

bool GearLibrary::checkStorageLimits()
{
    if (gearItems.size() >= maxGearItems)
        return false;

    if (getTotalGearStorageSize() >= maxStorageSize)
        return false;

    return true;
}

void GearLibrary::loadGearMetadata()
{
    try
    {
        auto metadataIndexPath = fileSystem.joinPath(libraryRootDir, "gear_index.txt");
        if (fileSystem.fileExists(metadataIndexPath))
        {
            auto content = fileSystem.readFile(metadataIndexPath);
            auto lines = juce::StringArray::fromLines(content);

            for (auto &line : lines)
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
    catch (...)
    {
        // If loading fails, continue with empty metadata
    }
}

void GearLibrary::loadCategories()
{
    try
    {
        auto categoryPath = generateCategoryPath("");
        if (fileSystem.fileExists(categoryPath))
        {
            auto content = fileSystem.readFile(categoryPath);
            auto lines = juce::StringArray::fromLines(content);

            for (auto &line : lines)
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
    catch (...)
    {
        // If loading fails, continue with empty categories
    }
}

void GearLibrary::saveGearMetadata()
{
    try
    {
        auto metadataIndexPath = fileSystem.joinPath(libraryRootDir, "gear_index.txt");
        juce::String content;

        for (auto &[id, metadata] : gearMetadata)
        {
            content += id + "|" +
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
    catch (...)
    {
        // If saving fails, continue without saving
    }
}

void GearLibrary::saveCategories()
{
    try
    {
        auto categoryPath = generateCategoryPath("");
        juce::String content;

        for (auto &[name, category] : categories)
        {
            content += name + "|" +
                       juce::String(category.creationTime.toMilliseconds()) + "|" +
                       category.description + "|" +
                       category.gearIds.joinIntoString(",") + "\n";
        }

        fileSystem.writeFile(categoryPath, content);
    }
    catch (...)
    {
        // If saving fails, continue without saving
    }
}

bool GearLibrary::loadGearThumbnail(GearItem *gearItem)
{

    if (!gearItem || gearItem->imageUrl.isEmpty())
    {
        return false;
    }

    // Check if thumbnail is already cached
    if (cacheManager.isCached("thumb_" + gearItem->unitId))
    {
        gearItem->thumbnailImage = cacheManager.getCachedImage("thumb_" + gearItem->unitId);
        bool success = !gearItem->thumbnailImage.isNull();

        // If cached thumbnail load failed, clear the cache entry and fetch again
        if (!success)
        {
            cacheManager.clearCache("thumb_" + gearItem->unitId);
        }
        else
        {
            return true;
        }
    }

    // Convert relative thumbnail path to absolute URL
    juce::String absoluteThumbnailUrl = RemoteConfig::getThumbnailUrl(gearItem->imageUrl);

    // Try to fetch thumbnail from remote
    try
    {
        juce::URL imageUrl(absoluteThumbnailUrl);
        bool success = false;
        auto imageData = networkFetcher.fetchGearImage(imageUrl, success);

        if (success && imageData.getSize() > 0)
        {
            // imageData is already a MemoryBlock, no conversion needed

            if (cacheManager.cacheBinaryData("thumb_" + gearItem->unitId, imageData))
            {
                // Load the cached image
                gearItem->thumbnailImage = cacheManager.getCachedImage("thumb_" + gearItem->unitId);
                bool finalSuccess = !gearItem->thumbnailImage.isNull();
                return finalSuccess;
            }
            else
            {
            }
        }
    }
    catch (...)
    {
    }

    // Create placeholder if loading fails
    gearItem->createPlaceholderImage();
    return false;
}

bool GearLibrary::loadGearSchema(GearItem *gearItem)
{

    if (!gearItem || gearItem->schemaPath.isEmpty())
    {
        return false;
    }

    // Check if schema is already cached
    if (cacheManager.isCached("schema_" + gearItem->unitId))
    {
        juce::String cachedPath = cacheManager.getCachedPath("schema_" + gearItem->unitId);
        if (cachedPath.isNotEmpty())
        {
            // Read the cached schema file
            juce::File cachedFile(cachedPath);
            if (cachedFile.existsAsFile())
            {
                juce::String cachedSchema = cachedFile.loadFileAsString();
                if (cachedSchema.isNotEmpty())
                {
                    return parseGearSchema(gearItem, cachedSchema);
                }
            }
        }
    }

    // Convert relative schema path to absolute URL
    juce::String absoluteSchemaUrl = RemoteConfig::getSchemaUrl(gearItem->schemaPath);

    // Try to fetch schema from remote
    try
    {
        juce::URL schemaUrl(absoluteSchemaUrl);
        auto inputStream = schemaUrl.createInputStream(false);

        if (inputStream != nullptr)
        {
            juce::String schemaData = inputStream->readEntireStreamAsString();

            if (schemaData.isNotEmpty())
            {
                // Cache the schema data
                cacheManager.cacheData("schema_" + gearItem->unitId, schemaData);

                // Parse the schema
                bool success = parseGearSchema(gearItem, schemaData);
                return success;
            }
        }
    }
    catch (const std::exception &e)
    {
    }

    return false;
}

bool GearLibrary::parseGearSchema(GearItem *gearItem, const juce::String &schemaData)
{

    // Parse JSON schema
    auto schemaJson = juce::JSON::parse(schemaData);
    if (!schemaJson.isObject())
    {
        return false;
    }

    // Look for faceplate image properties
    juce::StringArray faceplateProperties = {"faceplateImage", "thumbnailImage"};
    juce::String faceplateImagePath;
    bool foundFaceplate = false;

    for (const auto &propName : faceplateProperties)
    {
        if (schemaJson.hasProperty(propName))
        {
            faceplateImagePath = schemaJson.getProperty(propName, "").toString();
            if (faceplateImagePath.isNotEmpty())
            {
                foundFaceplate = true;
                gearItem->faceplateImagePath = faceplateImagePath;
                break;
            }
        }
    }

    // Parse controls if available
    if (schemaJson.hasProperty("controls") && schemaJson["controls"].isArray())
    {
        // Store existing control images before clearing the array
        juce::Array<juce::Image> existingKnobImages;
        juce::Array<juce::Image> existingButtonSprites;
        juce::Array<juce::Image> existingFaderImages;
        juce::Array<juce::Image> existingSwitchSprites;

        for (const auto &control : gearItem->controls)
        {
            existingKnobImages.add(control.loadedImage);
            existingButtonSprites.add(control.buttonSpriteSheet);
            existingFaderImages.add(control.faderImage);
            existingSwitchSprites.add(control.switchSpriteSheet);
        }

        // Only clear the controls array if we have existing controls with images
        // This prevents clearing on first load when there are no existing controls
        bool hasExistingControlsWithImages = false;
        for (const auto &control : gearItem->controls)
        {
            if (control.loadedImage.isValid() || control.buttonSpriteSheet.isValid() ||
                control.faderImage.isValid() || control.switchSpriteSheet.isValid())
            {
                hasExistingControlsWithImages = true;
                break;
            }
        }

        if (hasExistingControlsWithImages)
        {
            gearItem->controls.clear();
        }

        auto controlsArray = schemaJson["controls"].getArray();

        for (int i = 0; i < controlsArray->size(); ++i)
        {
            auto &controlVar = controlsArray->getReference(i);
            if (!controlVar.isObject())
            {
                continue;
            }

            // Get control type
            GearControl::ControlType controlType = GearControl::ControlType::Button;
            juce::String controlTypeStr = controlVar.getProperty("type", "button").toString().toLowerCase();
            if (controlTypeStr == "fader")
                controlType = GearControl::ControlType::Fader;
            else if (controlTypeStr == "switch")
                controlType = GearControl::ControlType::Switch;
            else if (controlTypeStr == "knob")
                controlType = GearControl::ControlType::Knob;

            // Get control orientation
            GearControl::Orientation orientation = GearControl::Orientation::Horizontal;
            juce::String orientationStr = controlVar.getProperty("orientation", "horizontal").toString().toLowerCase();
            if (orientationStr == "vertical")
                orientation = GearControl::Orientation::Vertical;

            // Get control position
            juce::Rectangle<float> position;
            if (controlVar.hasProperty("position"))
            {
                auto posObj = controlVar.getProperty("position", juce::var());
                if (posObj.isObject())
                {
                    // Default normalized sizes based on control type
                    float defaultWidth, defaultHeight;
                    if (controlType == GearControl::ControlType::Knob)
                    {
                        defaultWidth = 0.08f;  // 8% of faceplate width
                        defaultHeight = 0.08f; // 8% of faceplate height
                    }
                    else if (controlType == GearControl::ControlType::Button)
                    {
                        defaultWidth = 0.12f;  // 12% of faceplate width
                        defaultHeight = 0.08f; // 8% of faceplate height
                    }
                    else
                    {
                        defaultWidth = 0.1f;  // 10% of faceplate width
                        defaultHeight = 0.1f; // 10% of faceplate height
                    }

                    position = juce::Rectangle<float>(
                        static_cast<float>(posObj.getProperty("x", 0)),
                        static_cast<float>(posObj.getProperty("y", 0)),
                        static_cast<float>(posObj.getProperty("width", defaultWidth)),
                        static_cast<float>(posObj.getProperty("height", defaultHeight)));
                }
            }

            // Get control name (use label for human-readable name, fallback to id)
            juce::String controlName = controlVar.getProperty("label", "").toString();
            if (controlName.isEmpty())
            {
                controlName = controlVar.getProperty("id", "").toString();
            }

            // Create control
            GearControl control(controlType, position, 0.0f);
            control.name = controlName;
            control.orientation = orientation;

            // Add control to array first so we can reference it properly
            gearItem->controls.add(control);
            GearControl &addedControl = gearItem->controls.getReference(gearItem->controls.size() - 1);
            addedControl.name = controlName; // Ensure name is set on the added control

            // Restore existing images if available (only if controls array was cleared)
            if (hasExistingControlsWithImages)
            {
                if (i < existingKnobImages.size() && existingKnobImages[i].isValid())
                {
                    addedControl.loadedImage = existingKnobImages[i];
                }
                if (i < existingButtonSprites.size() && existingButtonSprites[i].isValid())
                {
                    addedControl.buttonSpriteSheet = existingButtonSprites[i];
                }
                if (i < existingFaderImages.size() && existingFaderImages[i].isValid())
                {
                    addedControl.faderImage = existingFaderImages[i];
                }
                if (i < existingSwitchSprites.size() && existingSwitchSprites[i].isValid())
                {
                    addedControl.switchSpriteSheet = existingSwitchSprites[i];
                }
            }

            // Parse control-specific data based on type
            if (controlType == GearControl::ControlType::Button)
            {
                // Parse button-specific data
                if (controlVar.hasProperty("image"))
                {
                    juce::String imagePath = controlVar.getProperty("image", "").toString();
                    if (imagePath.isNotEmpty())
                    {
                        // Store the image path for later loading (after controls are in final array)
                        addedControl.imagePath = imagePath;
                    }
                    else
                    {
                    }
                }
                else
                {
                }

                if (controlVar.hasProperty("options") && controlVar["options"].isArray())
                {
                    auto optionsArray = controlVar["options"].getArray();

                    for (auto &optionVar : *optionsArray)
                    {
                        if (optionVar.isObject())
                        {
                            GearControl::SwitchOptionFrame frame;
                            frame.value = static_cast<float>(optionVar.getProperty("value", 0));
                            frame.label = optionVar.getProperty("label", "").toString();

                            if (optionVar.hasProperty("frame") && optionVar["frame"].isObject())
                            {
                                auto frameObj = optionVar["frame"];
                                frame.position = juce::Rectangle<float>(
                                    static_cast<float>(frameObj.getProperty("x", 0)),
                                    static_cast<float>(frameObj.getProperty("y", 0)),
                                    static_cast<float>(frameObj.getProperty("width", 20)),
                                    static_cast<float>(frameObj.getProperty("height", 20)));
                                frame.size = frame.position;
                            }

                            addedControl.buttonFrames.add(frame);
                        }
                    }
                }
                else
                {
                }

                addedControl.isMomentary = controlVar.getProperty("momentary", true);
                addedControl.currentIndex = static_cast<int>(controlVar.getProperty("value", 0));
            }
            else if (controlType == GearControl::ControlType::Switch)
            {
                // Parse switch-specific data
                if (controlVar.hasProperty("image"))
                {
                    juce::String imagePath = controlVar.getProperty("image", "").toString();
                    if (imagePath.isNotEmpty())
                    {
                        addedControl.imagePath = imagePath;
                    }
                }

                // Clear existing options before parsing
                addedControl.options.clear();

                if (controlVar.hasProperty("options") && controlVar["options"].isArray())
                {
                    auto optionsArray = controlVar["options"].getArray();
                    for (auto &optionVar : *optionsArray)
                    {
                        if (optionVar.isObject())
                        {
                            GearControl::SwitchOptionFrame frame;
                            frame.value = static_cast<float>(optionVar.getProperty("value", 0));
                            frame.label = optionVar.getProperty("label", "").toString();

                            if (optionVar.hasProperty("frame") && optionVar["frame"].isObject())
                            {
                                auto frameObj = optionVar["frame"];
                                frame.position = juce::Rectangle<float>(
                                    static_cast<float>(frameObj.getProperty("x", 0)),
                                    static_cast<float>(frameObj.getProperty("y", 0)),
                                    static_cast<float>(frameObj.getProperty("width", 20)),
                                    static_cast<float>(frameObj.getProperty("height", 20)));
                                frame.size = frame.position;
                            }

                            addedControl.switchFrames.add(frame);

                            // Also add the option label to the options array
                            addedControl.options.add(frame.label);
                        }
                    }
                }

                addedControl.currentIndex = static_cast<int>(controlVar.getProperty("currentIndex", 0));
                addedControl.currentValue = (float)addedControl.currentIndex;
                addedControl.initialValue = addedControl.currentValue;
            }
            else if (controlType == GearControl::ControlType::Knob)
            {
                // Parse knob-specific data
                if (controlVar.hasProperty("image"))
                {
                    juce::String imagePath = controlVar.getProperty("image", "").toString();
                    if (imagePath.isNotEmpty())
                    {
                        // Store the image path for later loading (after controls are in final array)
                        addedControl.imagePath = imagePath;
                    }
                    else
                    {
                    }
                }
                else
                {
                }

                addedControl.startAngle = static_cast<float>(controlVar.getProperty("startAngle", 0));
                addedControl.endAngle = static_cast<float>(controlVar.getProperty("endAngle", 360));

                // Use the value directly as degrees (0-360)
                addedControl.currentValue = static_cast<float>(controlVar.getProperty("value", 180));
                addedControl.initialValue = addedControl.currentValue; // Store initial value in degrees

                // Load steps if they exist in the schema
                if (controlVar.hasProperty("steps"))
                {
                    auto stepsArray = controlVar.getProperty("steps", juce::var());
                    if (stepsArray.isArray())
                    {
                        for (int i = 0; i < stepsArray.size(); ++i)
                        {
                            addedControl.steps.add(static_cast<float>(stepsArray[i]));
                        }

                        // Initialize currentStepIndex to match the current value
                        addedControl.currentStepIndex = 0; // Default to first step
                        for (int i = 0; i < addedControl.steps.size(); ++i)
                        {
                            if (std::abs(addedControl.steps[i] - addedControl.currentValue) < 0.001f)
                            {
                                addedControl.currentStepIndex = i;
                                break;
                            }
                        }
                    }
                }
            }
            else if (controlType == GearControl::ControlType::Fader)
            {
                // Parse fader-specific data
                if (controlVar.hasProperty("image"))
                {
                    juce::String imagePath = controlVar.getProperty("image", "").toString();
                    if (imagePath.isNotEmpty())
                    {
                        // Store the image path for later loading (after controls are in final array)
                        addedControl.imagePath = imagePath;
                    }
                }

                addedControl.length = static_cast<float>(controlVar.getProperty("length", 100));
                addedControl.currentValue = static_cast<float>(controlVar.getProperty("value", 0));
            }
        }

        // Load control images after all controls are in the final array (like the old system)
        loadControlImages(gearItem);
    }

    return true;
}

void GearLibrary::loadControlImages(GearItem *gearItem)
{

    for (int i = 0; i < gearItem->controls.size(); ++i)
    {
        GearControl &control = gearItem->controls.getReference(i);
        if (!control.imagePath.isEmpty())
        {
            loadControlImage(gearItem, control, control.imagePath);
        }
        else
        {
        }
    }
}

bool GearLibrary::loadControlImage(GearItem *gearItem, GearControl &control, const juce::String &imagePath)
{

    if (!gearItem || imagePath.isEmpty())
    {
        return false;
    }

    try
    {
        // Check if image is already cached
        juce::String cachedPath = cacheManager.getCachedPath(imagePath);
        if (cachedPath.isNotEmpty() && fileSystem.fileExists(cachedPath))
        {

            // Load the cached image
            juce::File imageFile(cachedPath);
            juce::Image image = juce::ImageFileFormat::loadFrom(imageFile);

            if (image.isValid())
            {
                // Set the appropriate image based on control type
                switch (control.type)
                {
                case GearControl::ControlType::Button:
                    control.setButtonSpriteSheet(image);
                    break;
                case GearControl::ControlType::Switch:
                    control.setSwitchSpriteSheet(image);
                    break;
                case GearControl::ControlType::Fader:
                    control.setFaderImage(image);
                    break;
                case GearControl::ControlType::Knob:
                    control.setKnobImage(image);
                    break;
                }

                return true;
            }
        }

        // If not cached, download the image

        // Download the control image
        bool downloadSuccess = downloadGearAsset(gearItem->unitId, imagePath, "control");

        if (downloadSuccess)
        {
            // Try to load from cache again after download
            cachedPath = cacheManager.getCachedPath(imagePath);
            if (cachedPath.isNotEmpty() && fileSystem.fileExists(cachedPath))
            {
                juce::File imageFile(cachedPath);
                juce::Image image = juce::ImageFileFormat::loadFrom(imageFile);

                if (image.isValid())
                {
                    // Set the appropriate image based on control type
                    switch (control.type)
                    {
                    case GearControl::ControlType::Button:
                        control.setButtonSpriteSheet(image);
                        break;
                    case GearControl::ControlType::Switch:
                        control.setSwitchSpriteSheet(image);
                        break;
                    case GearControl::ControlType::Fader:
                        control.setFaderImage(image);
                        break;
                    case GearControl::ControlType::Knob:
                        control.setKnobImage(image);
                        break;
                    }

                    return true;
                }
            }
        }

        return false;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool GearLibrary::loadGearFaceplate(GearItem *gearItem)
{

    if (!gearItem || gearItem->faceplateImagePath.isEmpty())
    {
        return false;
    }

    // Check if faceplate is already loaded to prevent duplicate fetching
    if (gearItem->faceplateImage.isValid())
    {
        return true;
    }

    // Extract filename from faceplate path
    juce::String filename = fileSystem.getFileName(gearItem->faceplateImagePath);

    // Check cache first - use sourceUnitId for cache key since all instances of the same gear type share the same faceplate
    juce::String cacheKey = gearItem->isInstance ? "faceplate_" + gearItem->sourceUnitId : "faceplate_" + gearItem->unitId;
    if (cacheManager.isCached(cacheKey))
    {
        gearItem->faceplateImage = cacheManager.getCachedImage(cacheKey);
        if (gearItem->faceplateImage.isValid())
        {
            return true;
        }
        else
        {
            cacheManager.clearCache(cacheKey);
        }
    }

    // Construct the full URL if it's a relative path
    juce::String fullUrl = gearItem->faceplateImagePath;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = RemoteConfig::getFaceplateUrl(fullUrl);
        }
    }

    // Try to fetch faceplate from remote
    try
    {
        juce::URL imageUrl(fullUrl);
        auto inputStream = imageUrl.createInputStream(false);

        if (inputStream != nullptr)
        {
            juce::MemoryBlock binaryData;
            inputStream->readIntoMemoryBlock(binaryData);

            if (binaryData.getSize() > 0)
            {
                // Load image directly from binary data
                gearItem->faceplateImage = juce::ImageCache::getFromMemory(binaryData.getData(), binaryData.getSize());

                if (gearItem->faceplateImage.isValid())
                {
                    // Cache the faceplate image using the correct cache key
                    cacheManager.addToCache(cacheKey, gearItem->faceplateImage);
                    return true;
                }
                else
                {
                }
            }
        }
    }
    catch (const std::exception &e)
    {
    }

    return false;
}

void GearLibrary::loadGearFaceplateAsync(GearItem *gearItem, std::function<void()> onLoaded)
{
    if (!gearItem || gearItem->faceplateImagePath.isEmpty())
    {
        return;
    }

    // Use mutex to prevent race conditions in faceplate loading
    std::lock_guard<std::mutex> lock(faceplateLoadingMutex);

    // Check if faceplate is already loaded to prevent duplicate fetching
    if (gearItem->faceplateImage.isValid())
    {
        // Call the callback even for already loaded faceplates to maintain counter consistency
        if (onLoaded)
        {
            juce::MessageManager::callAsync(onLoaded);
        }
        return;
    }

    // Extract filename from faceplate path
    juce::String filename = fileSystem.getFileName(gearItem->faceplateImagePath);

    // Check cache first - use sourceUnitId for cache key since all instances of the same gear type share the same faceplate
    juce::String cacheKey = gearItem->isInstance ? "faceplate_" + gearItem->sourceUnitId : "faceplate_" + gearItem->unitId;

    if (cacheManager.isCached(cacheKey))
    {
        gearItem->faceplateImage = cacheManager.getCachedImage(cacheKey);
        if (gearItem->faceplateImage.isValid())
        {
            // Call the callback even for cached faceplates to maintain counter consistency
            if (onLoaded)
            {
                juce::MessageManager::callAsync(onLoaded);
            }
            return;
        }
        else
        {
            cacheManager.clearCache(cacheKey);
        }
    }
    else
    {
    }

    // Construct the full URL if it's a relative path
    juce::String fullUrl = gearItem->faceplateImagePath;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = RemoteConfig::getFaceplateUrl(fullUrl);
        }
    }

    /**
     * @brief Thread for downloading faceplate images asynchronously.
     *
     * This struct handles the asynchronous download and processing of faceplate images
     * for gear items, ensuring UI updates happen on the message thread.
     */
    struct FaceplateImageDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new FaceplateImageDownloader.
         *
         * @param urlToUse The URL to download the image from
         * @param itemToUpdate The gear item to update with the image
         * @param parentLibrary The gear library to notify when the image is loaded
         * @param filenameToCache The filename to use for caching
         * @param onLoadedCallback Callback to trigger when faceplate is loaded
         */
        FaceplateImageDownloader(juce::URL urlToUse, GearItem *itemToUpdate, GearLibrary *parentLibrary, const juce::String &filenameToCache, ICacheManager &cacheManagerRef, std::function<void()> onLoadedCallback)
            : juce::Thread("Faceplate Image Downloader"),
              url(urlToUse), item(itemToUpdate), library(parentLibrary), filename(filenameToCache), cacheManager(cacheManagerRef), onLoaded(onLoadedCallback)
        {
            startThread();
        }

        ~FaceplateImageDownloader() override
        {
            stopThread(5000); // Wait up to 5 seconds for thread to finish
        }

        void run() override
        {

            try
            {
                auto inputStream = url.createInputStream(false);
                if (inputStream != nullptr)
                {
                    juce::MemoryBlock binaryData;
                    inputStream->readIntoMemoryBlock(binaryData);

                    if (binaryData.getSize() > 0)
                    {
                        // Load image directly from binary data using MemoryInputStream
                        juce::MemoryInputStream stream(binaryData, false);
                        juce::Image downloadedImage = juce::ImageFileFormat::loadFrom(stream);

                        if (downloadedImage.isValid())
                        {
                            // Cache the faceplate image using the correct cache key
                            juce::String cacheKey = item->isInstance ? "faceplate_" + item->sourceUnitId : "faceplate_" + item->unitId;

                            // Check if there's already something in the cache with this key
                            if (cacheManager.isCached(cacheKey))
                            {
                                juce::Image existingImage = cacheManager.getCachedImage(cacheKey);
                            }

                            cacheManager.addToCache(cacheKey, downloadedImage);

                            // Store the item pointer and ID for validation in the async callback
                            GearItem *itemPtr = item;
                            juce::String itemId = item->isInstance ? item->instanceId : item->unitId;

                            // Update the gear item on the message thread with validation
                            juce::MessageManager::callAsync([this, itemPtr, downloadedImage, itemId]()
                                                            {
                                
                                // Validate that the item pointer is still valid and matches the expected ID
                                if (itemPtr && (itemPtr->isInstance ? itemPtr->instanceId : itemPtr->unitId) == itemId)
                                {
                                    itemPtr->faceplateImage = downloadedImage;
                                    
                                    // Call the specific callback if provided
                                    if (onLoaded)
                                    {
                                        onLoaded();
                                    }
                                    // Fallback to global library callback if no specific callback
                                    else if (library->onLibraryUpdated)
                                    {
                                        library->onLibraryUpdated();
                                    }
                                }
                                else
                                {
                                } });
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                    }
                }
                else
                {
                }
            }
            catch (const std::exception &e)
            {
            }
        }

    private:
        juce::URL url;
        GearItem *item;
        GearLibrary *library;
        juce::String filename;
        ICacheManager &cacheManager;
        std::function<void()> onLoaded;
    };

    // Start the async download
    new FaceplateImageDownloader(juce::URL(fullUrl), gearItem, this, filename, cacheManager, onLoaded);
}

void GearLibrary::refreshAllThumbnails()
{

    for (auto *gearItem : gearItems)
    {
        if (gearItem && !gearItem->imageUrl.isEmpty())
        {

            // Clear any existing cached thumbnail to force a fresh fetch
            cacheManager.clearCache("thumb_" + gearItem->unitId);

            // Load the thumbnail (this will fetch from remote if not cached)
            loadGearThumbnail(gearItem);
        }
    }

    // Notify listeners that the library has been updated
    if (onLibraryUpdated)
    {
        onLibraryUpdated();
    }
}

GearItem *GearLibrary::findGearItemById(const juce::String &itemId)
{
    // Search through all gear items to find the one with matching ID
    for (auto *item : gearItems)
    {
        if (item)
        {
            // Check if it's a template item
            if (item->unitId == itemId)
            {
                return item;
            }
            // Check if it's an instance
            if (item->isInstance && item->instanceId == itemId)
            {
                return item;
            }
        }
    }

    return nullptr;
}
