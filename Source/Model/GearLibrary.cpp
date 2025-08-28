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

    juce::Logger::writeToLog("GearLibrary: Lazy initialization starting");
    try
    {
        juce::Logger::writeToLog("GearLibrary: Calling initializeLibraryDirectory");
        initializeLibraryDirectory();
        juce::Logger::writeToLog("GearLibrary: initializeLibraryDirectory completed");

        juce::Logger::writeToLog("GearLibrary: Calling loadGearMetadata");
        loadGearMetadata();
        juce::Logger::writeToLog("GearLibrary: loadGearMetadata completed");

        juce::Logger::writeToLog("GearLibrary: Calling loadCategories");
        loadCategories();
        juce::Logger::writeToLog("GearLibrary: loadCategories completed");

        juce::Logger::writeToLog("GearLibrary: Calling loadRemoteGearLibrary");
        loadRemoteGearLibrary(); // Load gear from remote source (no fallback items)
        juce::Logger::writeToLog("GearLibrary: loadRemoteGearLibrary completed");

        juce::Logger::writeToLog("GearLibrary: Lazy initialization completed successfully");
        initialized = true;
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibrary: Lazy initialization caught exception - continuing with empty library");
        // If initialization fails, continue with empty library
        // This prevents crashes when file system is not available
    }
}

void GearLibrary::initializeLibraryDirectory()
{
    juce::Logger::writeToLog("GearLibrary: initializeLibraryDirectory starting");
    try
    {
        juce::Logger::writeToLog("GearLibrary: Getting cache root directory");
        libraryRootDir = fileSystem.joinPath(fileSystem.getCacheRootDirectory(), "GearLibrary");
        juce::Logger::writeToLog("GearLibrary: Library root dir set to: " + libraryRootDir);

        juce::Logger::writeToLog("GearLibrary: Checking if directory exists");
        if (!fileSystem.directoryExists(libraryRootDir))
        {
            juce::Logger::writeToLog("GearLibrary: Directory doesn't exist, creating it");
            if (!fileSystem.createDirectory(libraryRootDir))
            {
                juce::Logger::writeToLog("GearLibrary: Failed to create directory, using fallback path");
                // If we can't create the directory, use a fallback path
                libraryRootDir = "/tmp/analogiq_gear_library";
            }
            else
            {
                juce::Logger::writeToLog("GearLibrary: Directory created successfully");
            }
        }
        else
        {
            juce::Logger::writeToLog("GearLibrary: Directory already exists");
        }
        juce::Logger::writeToLog("GearLibrary: initializeLibraryDirectory completed successfully");
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibrary: initializeLibraryDirectory caught exception, using fallback path");
        // Use fallback path if anything goes wrong
        libraryRootDir = "/tmp/analogiq_gear_library";
    }
}

void GearLibrary::loadRemoteGearLibrary()
{
    juce::Logger::writeToLog("GearLibrary: loadRemoteGearLibrary starting");

    // Load gear from remote GitHub repository using configuration
    juce::String remoteUrl = RemoteConfig::getGearLibraryIndexUrl();
    juce::Logger::writeToLog("GearLibrary: Remote URL: " + remoteUrl);

    // Step 1: Try to load from local cache first
    juce::Logger::writeToLog("GearLibrary: Checking local cache for remote_gear_library");
    auto cachedData = cacheManager.getCachedPath("remote_gear_library");
    if (!cachedData.isEmpty())
    {
        juce::Logger::writeToLog("GearLibrary: Found cached data, loading from cache");
        if (loadGearFromCache(cachedData))
        {
            juce::Logger::writeToLog("GearLibrary: Successfully loaded " + juce::String(gearItems.size()) + " items from cache");
        }
        else
        {
            juce::Logger::writeToLog("GearLibrary: Failed to load from cache, clearing items");
            gearItems.clear();
        }
    }
    else
    {
        juce::Logger::writeToLog("GearLibrary: No cached data found, library will be empty until remote fetch completes");
        gearItems.clear();
    }

    // Step 2: Async remote fetch to update cache and add new/changed gear
    juce::Logger::writeToLog("GearLibrary: Starting async remote fetch");
    fetchRemoteGearAsync(remoteUrl);

    juce::Logger::writeToLog("GearLibrary: loadRemoteGearLibrary completed");
}

void GearLibrary::createSampleGearItems()
{
    juce::Logger::writeToLog("GearLibrary: createSampleGearItems starting");
    // Create sample gear items for development and testing
    // These will be replaced by real remote data when NetworkFetcher is integrated

    // Sample EQ
    juce::Logger::writeToLog("GearLibrary: Creating sample EQ item");
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

    juce::Logger::writeToLog("GearLibrary: Adding EQ item to gearItems array");
    gearItems.add(eq.release());
    juce::Logger::writeToLog("GearLibrary: EQ item added successfully");

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
        juce::Logger::writeToLog("GearLibrary: Creating default categories");

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

        juce::Logger::writeToLog("GearLibrary: Default categories created");
    }
}

// Helper methods for remote gear loading
bool GearLibrary::loadGearFromCache(const juce::String &cachePath)
{
    juce::Logger::writeToLog("GearLibrary: loadGearFromCache starting with path: " + cachePath);

    try
    {
        juce::File cacheFile(cachePath);
        if (!cacheFile.existsAsFile())
        {
            juce::Logger::writeToLog("GearLibrary: Cache file does not exist");
            return false;
        }

        juce::String jsonContent = cacheFile.loadFileAsString();
        if (jsonContent.isEmpty())
        {
            juce::Logger::writeToLog("GearLibrary: Cache file is empty");
            return false;
        }

        auto json = juce::JSON::parse(jsonContent);
        if (!json.isObject())
        {
            juce::Logger::writeToLog("GearLibrary: Cache file does not contain valid JSON object");
            return false;
        }

        // Check if we have a "units" array in the new format (matching legacy system)
        if (!json.hasProperty("units") || !json["units"].isArray())
        {
            juce::Logger::writeToLog("GearLibrary: Cache file does not contain a 'units' array");
            return false;
        }

        // Clear existing items and load from cache
        juce::Logger::writeToLog("GearLibrary: Clearing existing gear items");
        gearItems.clear();
        juce::Logger::writeToLog("GearLibrary: Getting units array from JSON");
        auto gearArray = json["units"].getArray();
        juce::Logger::writeToLog("GearLibrary: Units array size: " + juce::String(gearArray->size()));

        for (auto &gearObject : *gearArray)
        {
            juce::Logger::writeToLog("GearLibrary: Processing gear item, isObject: " + juce::String(gearObject.isObject() ? "YES" : "NO") +
                                     ", isArray: " + juce::String(gearObject.isArray() ? "YES" : "NO") +
                                     ", isString: " + juce::String(gearObject.isString() ? "YES" : "NO"));

            if (gearObject.isObject())
            {
                juce::Logger::writeToLog("GearLibrary: About to call parseGearFromJson");
                auto gearItem = parseGearFromJson(gearObject);
                juce::Logger::writeToLog("GearLibrary: parseGearFromJson returned");

                if (gearItem)
                {
                    juce::Logger::writeToLog("GearLibrary: Successfully parsed gear item: " + gearItem->unitId);
                    juce::Logger::writeToLog("GearLibrary: Adding gear item to array");
                    gearItems.add(gearItem.release());
                    juce::Logger::writeToLog("GearLibrary: Gear item added successfully");
                }
                else
                {
                    juce::Logger::writeToLog("GearLibrary: Failed to parse gear item - parseGearFromJson returned nullptr");
                }
            }
            else
            {
                juce::Logger::writeToLog("GearLibrary: Gear object is not a valid object");
            }
        }

        juce::Logger::writeToLog("GearLibrary: Successfully loaded " + juce::String(gearItems.size()) + " items from cache");

        // Refresh all thumbnails after loading from cache
        refreshAllThumbnails();

        return true;
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibrary: Exception while loading from cache");
        return false;
    }
}

void GearLibrary::fetchRemoteGearAsync(const juce::String &remoteUrl)
{
    juce::Logger::writeToLog("GearLibrary: fetchRemoteGearAsync starting with URL: " + remoteUrl);

    // Use the injected NetworkFetcher to fetch remote data
    try
    {
        bool success = false;
        auto jsonData = networkFetcher.fetchRemoteGearLibrary(juce::URL(remoteUrl), success);
        if (!jsonData.isEmpty())
        {
            juce::Logger::writeToLog("GearLibrary: Successfully fetched remote data, size: " + juce::String(jsonData.length()));

            // Parse the remote data
            auto json = juce::JSON::parse(jsonData);
            if (!json.isObject())
            {
                juce::Logger::writeToLog("GearLibrary: Remote data is not a valid JSON object");
                return;
            }

            // Check if we have a "units" array in the new format (matching legacy system)
            if (!json.hasProperty("units") || !json["units"].isArray())
            {
                juce::Logger::writeToLog("GearLibrary: Remote data does not contain a 'units' array");
                return;
            }

            auto gearArray = json["units"].getArray();
            juce::Logger::writeToLog("GearLibrary: Remote data contains " + juce::String(gearArray->size()) + " gear items");

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

            juce::Logger::writeToLog("GearLibrary: Added " + juce::String(newItemsCount) + " new gear items from remote");

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
            juce::Logger::writeToLog("GearLibrary: Failed to fetch remote data - empty response");
        }
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibrary: Exception while fetching remote gear data");
    }
}

std::unique_ptr<GearItem> GearLibrary::parseGearFromJson(const juce::var &gearObject)
{
    juce::Logger::writeToLog("GearLibrary: parseGearFromJson called");

    if (!gearObject.isObject())
    {
        juce::Logger::writeToLog("GearLibrary: gearObject is not an object, returning nullptr");
        return nullptr;
    }

    auto obj = gearObject.getDynamicObject();
    if (!obj)
    {
        juce::Logger::writeToLog("GearLibrary: getDynamicObject() returned nullptr");
        return nullptr;
    }
    juce::Logger::writeToLog("GearLibrary: getDynamicObject() succeeded, object has " + juce::String(obj->getProperties().size()) + " properties");

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
    juce::Logger::writeToLog("GearLibrary: Extracted properties - unitId: '" + unitId + "', name: '" + name + "', manufacturer: '" + manufacturer + "', category: '" + category + "'");

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
        juce::Logger::writeToLog("GearLibrary: Controls found but parsing not yet implemented");
    }

    // Load thumbnail if imageUrl is available
    if (!gearItem->imageUrl.isEmpty())
    {
        loadGearThumbnail(gearItem.get());
    }

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
    for (auto &item : gearItems)
    {
        if (item->unitId == gearId)
            return item;
    }
    return nullptr;
}

const GearItem *GearLibrary::getGearItem(const juce::String &gearId) const
{
    for (auto &item : gearItems)
    {
        if (item->unitId == gearId)
            return item;
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
    juce::Logger::writeToLog("GearLibrary: Initialization flag reset to false");
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
    juce::Logger::writeToLog("GearLibrary::loadGearThumbnail called for " + gearItem->unitId);

    if (!gearItem || gearItem->imageUrl.isEmpty())
    {
        juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - gearItem is null or imageUrl is empty");
        return false;
    }

    juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - imageUrl: '" + gearItem->imageUrl + "'");

    // Check if thumbnail is already cached
    if (cacheManager.isCached("thumb_" + gearItem->unitId))
    {
        juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Found cached thumbnail for " + gearItem->unitId);
        gearItem->thumbnailImage = cacheManager.getCachedImage("thumb_" + gearItem->unitId);
        bool success = !gearItem->thumbnailImage.isNull();
        juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Cached thumbnail load " + juce::String(success ? "SUCCESS" : "FAILED"));

        // If cached thumbnail load failed, clear the cache entry and fetch again
        if (!success)
        {
            juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Cached thumbnail is corrupted, clearing cache entry");
            cacheManager.clearCache("thumb_" + gearItem->unitId);
        }
        else
        {
            return true;
        }
    }

    juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - No cached thumbnail, fetching from remote");

    // Convert relative thumbnail path to absolute URL
    juce::String absoluteThumbnailUrl = RemoteConfig::getThumbnailUrl(gearItem->imageUrl);
    juce::Logger::writeToLog("GearLibrary: Loading thumbnail from: " + absoluteThumbnailUrl);

    // Try to fetch thumbnail from remote
    try
    {
        juce::URL imageUrl(absoluteThumbnailUrl);
        bool success = false;
        auto imageData = networkFetcher.fetchRemoteGearLibrary(imageUrl, success);

        juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Remote fetch " + juce::String(success ? "SUCCESS" : "FAILED") +
                                 " - Data size: " + juce::String(imageData.getNumBytesAsUTF8()));

        if (success && !imageData.isEmpty())
        {
            // Convert to MemoryBlock and cache
            juce::MemoryBlock imageBlock;
            imageBlock.append(imageData.toRawUTF8(), imageData.getNumBytesAsUTF8());

            if (cacheManager.cacheBinaryData("thumb_" + gearItem->unitId, imageBlock))
            {
                juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Successfully cached binary data");
                // Load the cached image
                gearItem->thumbnailImage = cacheManager.getCachedImage("thumb_" + gearItem->unitId);
                bool finalSuccess = !gearItem->thumbnailImage.isNull();
                juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Final thumbnail load " + juce::String(finalSuccess ? "SUCCESS" : "FAILED"));
                return finalSuccess;
            }
            else
            {
                juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Failed to cache binary data");
            }
        }
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibrary: Exception while loading thumbnail for " + gearItem->unitId);
    }

    // Create placeholder if loading fails
    juce::Logger::writeToLog("GearLibrary::loadGearThumbnail - Creating placeholder image for " + gearItem->unitId);
    gearItem->createPlaceholderImage();
    return false;
}

void GearLibrary::refreshAllThumbnails()
{
    juce::Logger::writeToLog("GearLibrary::refreshAllThumbnails - Starting thumbnail refresh for " + juce::String(gearItems.size()) + " items");

    for (auto *gearItem : gearItems)
    {
        if (gearItem && !gearItem->imageUrl.isEmpty())
        {
            juce::Logger::writeToLog("GearLibrary::refreshAllThumbnails - Refreshing thumbnail for " + gearItem->unitId);

            // Clear any existing cached thumbnail to force a fresh fetch
            cacheManager.clearCache("thumb_" + gearItem->unitId);

            // Load the thumbnail (this will fetch from remote if not cached)
            loadGearThumbnail(gearItem);
        }
    }

    juce::Logger::writeToLog("GearLibrary::refreshAllThumbnails - Completed thumbnail refresh");

    // Notify listeners that the library has been updated
    if (onLibraryUpdated)
    {
        onLibraryUpdated();
    }
}
