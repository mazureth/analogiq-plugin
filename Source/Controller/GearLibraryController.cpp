/**
 * @file GearLibraryController.cpp
 * @brief Implementation of the GearLibraryController class.
 *
 * This file implements the gear library controller that coordinates between
 * the GearLibrary model and UI components, handling search, filtering,
 * and user interactions.
 */

#include "GearLibraryController.h"
#include "../Model/GearItem.h"

/**
 * @brief Constructs a new GearLibraryController.
 *
 * @param gearLibrary Reference to the gear library model
 * @param fileSystem Reference to the file system service
 * @param cacheManager Reference to the cache manager
 * @param networkFetcher Reference to the network fetcher
 */
GearLibraryController::GearLibraryController(GearLibrary &gearLibrary,
                                             IFileSystem &fileSystem,
                                             ICacheManager &cacheManager,
                                             INetworkFetcher &networkFetcher)
    : gearLibrary(gearLibrary),
      fileSystem(fileSystem),
      cacheManager(cacheManager),
      networkFetcher(networkFetcher)
{
    // Load favorites from storage
    updateFavoritesFromStorage();
}

// Search and Filter Coordination

juce::Array<GearItem *> GearLibraryController::searchGear(const juce::String &searchTerm)
{
    if (searchTerm.trim().isEmpty())
        return juce::Array<GearItem *>();

    juce::String normalizedTerm = normalizeSearchTerm(searchTerm);
    juce::Array<GearItem *> results;

    // Get all gear items and perform fuzzy matching
    auto allGearItems = gearLibrary.getAllGearItems();

    for (auto &gearItem : allGearItems)
    {
        int score = calculateFuzzyMatchScore(normalizedTerm, *gearItem);
        if (score > 0)
        {
            results.add(gearItem);
        }
    }

    // For now, skip sorting to avoid compilation issues
    // TODO: Implement proper sorting when the sorting mechanism is resolved

    return results;
}

juce::Array<GearItem *> GearLibraryController::filterGearByType(GearItem::GearType gearType)
{
    auto items = gearLibrary.filterGearByType(gearType);
    juce::Array<GearItem *> result;
    for (auto item : items)
    {
        result.add(const_cast<GearItem *>(item));
    }
    return result;
}

juce::Array<GearItem *> GearLibraryController::filterGearByCategory(GearItem::GearCategory category)
{
    auto items = gearLibrary.filterGearByCategory(category);
    juce::Array<GearItem *> result;
    for (auto item : items)
    {
        result.add(const_cast<GearItem *>(item));
    }
    return result;
}

juce::Array<GearItem *> GearLibraryController::getGearItemsInCategory(const juce::String &categoryName)
{
    auto items = gearLibrary.getGearItemsInCategory(categoryName);
    juce::Array<GearItem *> result;
    for (auto item : items)
    {
        result.add(const_cast<GearItem *>(item));
    }
    return result;
}

juce::StringArray GearLibraryController::getAvailableGearTypes() const
{
    juce::StringArray types;
    types.add("Series500");
    types.add("Rack19Inch");
    types.add("UserCreated");
    types.add("Other");
    return types;
}

juce::StringArray GearLibraryController::getAvailableGearCategories() const
{
    juce::StringArray categories;
    categories.add("EQ");
    categories.add("Compressor");
    categories.add("Preamp");
    categories.add("Other");
    return categories;
}

// Gear Loading and Caching

bool GearLibraryController::loadLocalGearLibrary()
{
    isLoadingLibrary = true;
    loadingProgress = 0;

    try
    {
        // Load the local gear library from the default location
        bool success = gearLibrary.importGearLibrary("");

        isLoadingLibrary = false;
        loadingProgress = 100;

        return success;
    }
    catch (...)
    {
        isLoadingLibrary = false;
        loadingProgress = 0;
        return false;
    }
}

bool GearLibraryController::refreshGearLibrary()
{
    isLoadingLibrary = true;
    loadingProgress = 0;

    try
    {
        // Refresh the gear library by checking for updates
        bool success = gearLibrary.checkForUpdates("");

        isLoadingLibrary = false;
        loadingProgress = 100;

        return success;
    }
    catch (...)
    {
        isLoadingLibrary = false;
        loadingProgress = 0;
        return false;
    }
}

bool GearLibraryController::downloadGearItem(const juce::String &gearId)
{
    try
    {
        // Download the gear item
        return gearLibrary.downloadGearItem(gearId, "");
    }
    catch (...)
    {
        return false;
    }
}

bool GearLibraryController::uploadGearItem(const juce::String &gearId)
{
    try
    {
        // Upload the gear item
        return gearLibrary.uploadGearItem(gearId, "");
    }
    catch (...)
    {
        return false;
    }
}

// User Interaction Management

GearItem *GearLibraryController::getSelectedGearItem() const
{
    return selectedGearItem;
}

void GearLibraryController::setSelectedGearItem(GearItem *gearItem)
{
    selectedGearItem = gearItem;
}

juce::Array<GearItem *> GearLibraryController::getFavoriteGearItems() const
{
    return favoriteGearItems;
}

bool GearLibraryController::addToFavorites(GearItem *gearItem)
{
    if (!gearItem || favoriteGearItems.contains(gearItem))
        return false;

    favoriteGearItems.add(gearItem);
    saveFavoritesToStorage();
    return true;
}

bool GearLibraryController::removeFromFavorites(GearItem *gearItem)
{
    if (!gearItem)
        return false;

    bool removed = favoriteGearItems.removeFirstMatchingValue(gearItem);
    if (removed)
    {
        saveFavoritesToStorage();
    }
    return removed;
}

// Drag and Drop Coordination

void GearLibraryController::handleDragStart(GearItem *gearItem, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // For now, just log the drag start
    // This will be enhanced when we implement the full drag and drop system
    if (gearItem)
    {
        std::cout << "[GearLibraryController] Started dragging gear item: " << gearItem->name << std::endl;
    }
}

void GearLibraryController::handleDragEnd(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // For now, just log the drag end
    // This will be enhanced when we implement the full drag and drop system
    std::cout << "[GearLibraryController] Drag operation ended" << std::endl;
}

bool GearLibraryController::isValidDrop(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails,
                                        const juce::Point<int> &dropPosition)
{
    // For now, accept all drops
    // This will be enhanced when we implement the full drag and drop system
    return true;
}

// Tree View Management

juce::ValueTree GearLibraryController::getTreeViewStructure() const
{
    juce::ValueTree tree("GearLibrary");

    // Add categories
    auto categories = getAvailableGearCategories();
    for (const auto &category : categories)
    {
        auto categoryNode = juce::ValueTree("Category");
        categoryNode.setProperty("name", category, nullptr);
        categoryNode.setProperty("expanded", true, nullptr);

        // Add gear items in this category
        auto gearItems = gearLibrary.getGearItemsInCategory(category);
        for (const auto &gearItem : gearItems)
        {
            auto gearNode = juce::ValueTree("GearItem");
            gearNode.setProperty("id", gearItem->unitId, nullptr);
            gearNode.setProperty("name", gearItem->name, nullptr);
            gearNode.setProperty("manufacturer", gearItem->manufacturer, nullptr);
            categoryNode.appendChild(gearNode, nullptr);
        }

        tree.appendChild(categoryNode, nullptr);
    }

    return tree;
}

void GearLibraryController::updateTreeView()
{
    // For now, just log the update
    // This will be enhanced when we implement the full tree view system
    std::cout << "[GearLibraryController] Tree view updated" << std::endl;
}

void GearLibraryController::expandCategory(const juce::String &categoryName)
{
    // For now, just log the expansion
    // This will be enhanced when we implement the full tree view system
    std::cout << "[GearLibraryController] Expanded category: " << categoryName << std::endl;
}

void GearLibraryController::collapseCategory(const juce::String &categoryName)
{
    // For now, just log the collapse
    // This will be enhanced when we implement the full tree view system
    std::cout << "[GearLibraryController] Collapsed category: " << categoryName << std::endl;
}

// Library Management

int GearLibraryController::getTotalGearItemCount() const
{
    return gearLibrary.getTotalGearItemCount();
}

int GearLibraryController::getGearItemCountInCategory(const juce::String &categoryName) const
{
    auto items = gearLibrary.getGearItemsInCategory(categoryName);
    return items.size();
}

bool GearLibraryController::isLoading() const
{
    return isLoadingLibrary;
}

int GearLibraryController::getLoadingProgress() const
{
    return loadingProgress;
}

// Private helper methods

void GearLibraryController::updateFavoritesFromStorage()
{
    // For now, just initialize with an empty list
    // This will be enhanced when we implement persistent storage
    favoriteGearItems.clear();
}

void GearLibraryController::saveFavoritesToStorage()
{
    // For now, just log the save
    // This will be enhanced when we implement persistent storage
    std::cout << "[GearLibraryController] Favorites saved to storage" << std::endl;
}

juce::String GearLibraryController::normalizeSearchTerm(const juce::String &searchTerm) const
{
    // Convert to lowercase and remove extra whitespace
    return searchTerm.toLowerCase().trim();
}

int GearLibraryController::calculateFuzzyMatchScore(const juce::String &searchTerm, const GearItem &gearItem) const
{
    if (searchTerm.isEmpty())
        return 0;

    int score = 0;
    juce::String normalizedSearch = searchTerm.toLowerCase();

    // Check name match
    if (gearItem.name.toLowerCase().contains(normalizedSearch))
    {
        score += 100;

        // Bonus for exact match
        if (gearItem.name.toLowerCase() == normalizedSearch)
            score += 50;

        // Bonus for starts with
        if (gearItem.name.toLowerCase().startsWith(normalizedSearch))
            score += 25;
    }

    // Check manufacturer match
    if (gearItem.manufacturer.toLowerCase().contains(normalizedSearch))
    {
        score += 75;
    }

    // Check description match
    if (gearItem.description.toLowerCase().contains(normalizedSearch))
    {
        score += 25;
    }

    // Check tags match
    for (const auto &tag : gearItem.tags)
    {
        if (tag.toLowerCase().contains(normalizedSearch))
        {
            score += 30;
            break;
        }
    }

    return score;
}
