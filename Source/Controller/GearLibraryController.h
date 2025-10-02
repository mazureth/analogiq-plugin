/**
 * @file GearLibraryController.h
 * @brief Header file for the GearLibraryController class.
 *
 * This file defines the gear library controller that coordinates between
 * the GearLibrary model and UI components, handling search, filtering,
 * and user interactions.
 */

#pragma once

#include <JuceHeader.h>
#include "../Model/GearLibrary.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/INetworkFetcher.h"

// Forward declarations
class GearItem;

/**
 * @brief Controller for gear library operations and coordination.
 *
 * The GearLibraryController is responsible for:
 * - Coordinating between GearLibrary (Model) and UI components
 * - Handling gear search and filtering operations
 * - Managing gear loading and caching operations
 * - Handling user interactions (selection, favorites, recently used)
 * - Coordinating drag-and-drop operations
 * - Managing tree view state and updates
 * - Handling refresh operations and external data loading
 */
class GearLibraryController
{
public:
    /**
     * @brief Constructs a new GearLibraryController.
     *
     * @param gearLibrary Reference to the gear library model
     * @param fileSystem Reference to the file system service
     * @param cacheManager Reference to the cache manager
     * @param networkFetcher Reference to the network fetcher
     */
    GearLibraryController(GearLibrary &gearLibrary,
                          IFileSystem &fileSystem,
                          ICacheManager &cacheManager,
                          INetworkFetcher &networkFetcher);

    /**
     * @brief Destructor for GearLibraryController.
     */
    ~GearLibraryController() = default;

    // Search and Filter Coordination
    /**
     * @brief Performs a search for gear items.
     *
     * @param searchTerm The search term to use
     * @return Array of matching gear items
     */
    juce::Array<GearItem *> searchGear(const juce::String &searchTerm);

    /**
     * @brief Filters gear items by type.
     *
     * @param gearType The gear type to filter by
     * @return Array of matching gear items
     */
    juce::Array<GearItem *> filterGearByType(GearItem::GearType gearType);

    /**
     * @brief Filters gear items by category.
     *
     * @param category The category to filter by
     * @return Array of matching gear items
     */
    juce::Array<GearItem *> filterGearByCategory(GearItem::GearCategory category);

    /**
     * @brief Gets all gear items in a category.
     *
     * @param categoryName The name of the category
     * @return Array of gear items in the category
     */
    juce::Array<GearItem *> getGearItemsInCategory(const juce::String &categoryName);

    /**
     * @brief Gets all available gear types.
     *
     * @return Array of gear type strings
     */
    juce::StringArray getAvailableGearTypes() const;

    /**
     * @brief Gets all available gear categories.
     *
     * @return Array of gear category strings
     */
    juce::StringArray getAvailableGearCategories() const;

    // Gear Loading and Caching
    /**
     * @brief Loads gear data from the local library.
     *
     * @return true if loading was successful, false otherwise
     */
    bool loadLocalGearLibrary();

    /**
     * @brief Refreshes the gear library from external sources.
     *
     * @return true if refresh was successful, false otherwise
     */
    bool refreshGearLibrary();

    /**
     * @brief Downloads a specific gear item.
     *
     * @param gearId The ID of the gear item to download
     * @return true if download was successful, false otherwise
     */
    bool downloadGearItem(const juce::String &gearId);

    /**
     * @brief Uploads a gear item to external sources.
     *
     * @param gearId The ID of the gear item to upload
     * @return true if upload was successful, false otherwise
     */
    bool uploadGearItem(const juce::String &gearId);

    // User Interaction Management
    /**
     * @brief Gets the currently selected gear item.
     *
     * @return Pointer to the selected gear item, or nullptr if none
     */
    GearItem *getSelectedGearItem() const;

    /**
     * @brief Sets the selected gear item.
     *
     * @param gearItem The gear item to select
     */
    void setSelectedGearItem(GearItem *gearItem);

    /**
     * @brief Gets the list of favorite gear items.
     *
     * @return Array of favorite gear items
     */
    juce::Array<GearItem *> getFavoriteGearItems() const;

    /**
     * @brief Adds a gear item to favorites.
     *
     * @param gearItem The gear item to add to favorites
     * @return true if successful, false otherwise
     */
    bool addToFavorites(GearItem *gearItem);

    /**
     * @brief Removes a gear item from favorites.
     *
     * @param gearItem The gear item to remove from favorites
     * @return true if successful, false otherwise
     */
    bool removeFromFavorites(GearItem *gearItem);

    // Drag and Drop Coordination
    /**
     * @brief Handles the start of a drag operation.
     *
     * @param gearItem The gear item being dragged
     * @param dragSourceDetails Details about the drag source
     */
    void handleDragStart(GearItem *gearItem, const juce::DragAndDropTarget::SourceDetails &dragSourceDetails);

    /**
     * @brief Handles the end of a drag operation.
     *
     * @param dragSourceDetails Details about the drag source
     */
    void handleDragEnd(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails);

    /**
     * @brief Checks if a drop operation is valid.
     *
     * @param dragSourceDetails Details about the drag source
     * @param dropPosition The position where the item would be dropped
     * @return true if the drop is valid, false otherwise
     */
    bool isValidDrop(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails,
                     const juce::Point<int> &dropPosition);

    // Tree View Management
    /**
     * @brief Gets the tree view structure for the gear library.
     *
     * @return Tree view structure as a ValueTree
     */
    juce::ValueTree getTreeViewStructure() const;

    /**
     * @brief Updates the tree view when the library changes.
     */
    void updateTreeView();

    /**
     * @brief Expands a specific category in the tree view.
     *
     * @param categoryName The name of the category to expand
     */
    void expandCategory(const juce::String &categoryName);

    /**
     * @brief Collapses a specific category in the tree view.
     *
     * @param categoryName The name of the category to collapse
     */
    void collapseCategory(const juce::String &categoryName);

    // Library Management
    /**
     * @brief Gets the total number of gear items in the library.
     *
     * @return Total number of gear items
     */
    int getTotalGearItemCount() const;

    /**
     * @brief Gets the number of gear items in a specific category.
     *
     * @param categoryName The name of the category
     * @return Number of gear items in the category
     */
    int getGearItemCountInCategory(const juce::String &categoryName) const;

    /**
     * @brief Checks if the library is currently loading.
     *
     * @return true if loading, false otherwise
     */
    bool isLoading() const;

    /**
     * @brief Gets the loading progress.
     *
     * @return Loading progress as a percentage (0-100)
     */
    int getLoadingProgress() const;

private:
    // Core dependencies
    GearLibrary &gearLibrary;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    INetworkFetcher &networkFetcher;

    // State tracking
    GearItem *selectedGearItem{nullptr};
    juce::Array<GearItem *> favoriteGearItems;
    bool isLoadingLibrary{false};
    int loadingProgress{0};

    // Private helper methods
    /**
     * @brief Updates the favorites list from persistent storage.
     */
    void updateFavoritesFromStorage();

    /**
     * @brief Saves the favorites list to persistent storage.
     */
    void saveFavoritesToStorage();

    /**
     * @brief Normalizes a search term for better matching.
     *
     * @param searchTerm The search term to normalize
     * @return Normalized search term
     */
    juce::String normalizeSearchTerm(const juce::String &searchTerm) const;

    /**
     * @brief Performs fuzzy matching on gear items.
     *
     * @param searchTerm The search term
     * @param gearItem The gear item to match against
     * @return Match score (higher is better)
     */
    int calculateFuzzyMatchScore(const juce::String &searchTerm, const GearItem &gearItem) const;
};
