/**
 * @file GearLibrary.h
 * @brief Header file for the GearLibrary class and related components.
 *
 * This file defines the GearLibrary class, which provides a user interface for
 * browsing, searching, and managing audio gear items. It includes both a legacy
 * list view and a new hierarchical tree view.
 */

#pragma once

#include <JuceHeader.h>
#include "DraggableListBox.h"
#include "GearItem.h"
#include "INetworkFetcher.h"
#include "CacheManager.h"
#include "IFileSystem.h"

/**
 * @brief Namespace containing remote resource URLs and paths.
 */
namespace RemoteResources
{
    const juce::String BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/";
    // for use when making changes to the schemas locally
    // const juce::String BASE_URL = "http://localhost:8000/";
    const juce::String LIBRARY_PATH = "units/index.json";
    const juce::String ASSETS_PATH = "assets/";
    const juce::String SCHEMAS_PATH = "units/";
}

// Forward declarations
class GearListBoxModel;
class GearTreeItem;

/**
 * @brief Main class for managing and displaying audio gear items.
 *
 * The GearLibrary class provides a user interface for browsing, searching,
 * and managing audio gear items. It supports both a legacy list view and
 * a new hierarchical tree view.
 */
class GearLibrary : public juce::Component,
                    public juce::Button::Listener
{
public:
    /**
     * @brief Constructor for GearLibrary.
     *
     * @param networkFetcher The network fetcher to use for loading resources
     * @param cacheManager The cache manager to use for file operations
     * @param fileSystem The file system to use for file operations
     * @param autoLoad Whether to automatically load the library data (default: true)
     */
    explicit GearLibrary(INetworkFetcher &networkFetcher, CacheManager &cacheManager, IFileSystem &fileSystem, bool autoLoad = true);

    /**
     * @brief Destructor for GearLibrary.
     */
    ~GearLibrary() override;

    /**
     * @brief Paints the GearLibrary component.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handles resizing of the GearLibrary component.
     */
    void resized() override;

    /**
     * @brief Gets the number of rows in the gear list.
     *
     * @return The total number of gear items in the library
     */
    int getNumRows();

    /**
     * @brief Paints a list box item.
     *
     * @param rowNumber The index of the row to paint
     * @param g The graphics context to paint with
     * @param width The width of the item
     * @param height The height of the item
     * @param rowIsSelected Whether the row is currently selected
     */
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected);

    /**
     * @brief Refreshes a component for a specific row in the list box.
     *
     * @param rowNumber The index of the row to refresh
     * @param isRowSelected Whether the row is currently selected
     * @param existingComponentToUpdate The existing component to update, if any
     * @return The component to display for the row, or nullptr if using default painting
     */
    juce::Component *refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate);

    /**
     * @brief Handles a list box item click.
     *
     * @param row The index of the clicked row
     * @param e The mouse event details
     */
    void listBoxItemClicked(int row, const juce::MouseEvent &e);

    /**
     * @brief Handles a list box item double click.
     *
     * @param row The index of the double-clicked row
     * @param e The mouse event details
     */
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent &e);

    /**
     * @brief Handles mouse down events.
     *
     * @param e The mouse event details
     */
    void mouseDown(const juce::MouseEvent &e) override;

    /**
     * @brief Handles mouse drag events.
     *
     * @param e The mouse event details
     */
    void mouseDrag(const juce::MouseEvent &e) override;

    /**
     * @brief Handles button click events.
     *
     * @param button Pointer to the clicked button
     */
    void buttonClicked(juce::Button *button) override;

    /**
     * @brief Gets a gear item by index.
     *
     * @param index The index of the gear item to retrieve
     * @return Pointer to the gear item, or nullptr if index is invalid
     */
    GearItem *getGearItem(int index);

    /**
     * @brief Gets the full array of gear items.
     *
     * @return Constant reference to the array of gear items
     */
    const juce::Array<GearItem> &getItems() const { return gearItems; }

    /**
     * @brief Loads the gear library data asynchronously.
     */
    void loadLibraryAsync();

    /**
     * @brief Loads gear items asynchronously.
     */
    void loadGearItemsAsync();

    /**
     * @brief Saves the gear library data asynchronously.
     */
    void saveLibraryAsync();

    /**
     * @brief Adds a new gear item to the library.
     *
     * @param name The name of the gear item
     * @param category The category of the gear item
     * @param description The description of the gear item
     * @param manufacturer The manufacturer of the gear item
     */
    void addItem(const juce::String &name, const juce::String &category, const juce::String &description, const juce::String &manufacturer);

    /**
     * @brief Constructs a full URL from a relative path.
     *
     * @param relativePath The relative path to convert
     * @return The full URL
     */
    static juce::String getFullUrl(const juce::String &relativePath)
    {
        // If already a full URL, return as is
        if (relativePath.startsWith("http"))
            return relativePath;

        // If this is an absolute path on the filesystem, return as is
        if (relativePath.startsWith("/"))
            return relativePath;

        // Handle the case where we might need to add assets/ or units/ prefix
        juce::String result;

        if (relativePath.startsWith("assets/") || relativePath.startsWith("units/"))
        {
            // Path already has the correct folder prefix
            result = RemoteResources::BASE_URL + relativePath;
        }
        else if (relativePath.endsWith(".json"))
        {
            // Likely a schema file - add units/ prefix if needed
            result = RemoteResources::BASE_URL + RemoteResources::SCHEMAS_PATH + relativePath;
        }
        else if (relativePath.endsWith(".jpg") || relativePath.endsWith(".png") ||
                 relativePath.endsWith(".jpeg") || relativePath.endsWith(".gif"))
        {
            // Likely an image file - add assets/ prefix if needed
            result = RemoteResources::BASE_URL + RemoteResources::ASSETS_PATH + relativePath;
        }
        else
        {
            // Default case - just append to base URL
            result = RemoteResources::BASE_URL + relativePath;
        }

        return result;
    }

    /**
     * @brief Updates the filtered items in both list and tree views.
     *
     * Refreshes the display of items based on current search criteria.
     */
    void updateFilteredItems();

    /**
     * @brief Refreshes the tree view to update recently used items.
     *
     * This method should be called when recently used items change
     * to update the tree view display.
     */
    void refreshTreeView();

    /**
     * @brief Refreshes only the recently used section of the tree.
     *
     * This method updates only the recently used items without
     * affecting the expansion state of other tree nodes.
     */
    void refreshRecentlyUsedSection();

    /**
     * @brief Clears the recently used items and refreshes the tree view.
     *
     * This method clears all recently used items from the cache
     * and updates the tree view display.
     */
    void clearRecentlyUsed();

    /**
     * @brief Refreshes only the favorites section of the tree.
     *
     * This method updates only the favorite items without
     * affecting the expansion state of other tree nodes.
     */
    void refreshFavoritesSection();

    /**
     * @brief Clears the favorites items and refreshes the tree view.
     *
     * This method clears all favorite items from the cache
     * and updates the tree view display.
     */
    void clearFavorites();

private:
    /**
     * @brief Parses the gear library data from JSON format.
     *
     * @param jsonData The JSON string containing gear library data
     */
    void parseGearLibrary(const juce::String &jsonData);

    /**
     * @brief Determines if a gear item should be shown based on current search.
     *
     * @param item The gear item to check
     * @return true if the item should be shown
     */
    bool shouldShowItem(const GearItem &item) const;

    // UI components
    juce::Label titleLabel{"titleLabel", "Gear Library"};                                                            ///< Title label for the library
    juce::DrawableButton refreshButton{"RefreshButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground}; ///< Button to refresh the gear list
    juce::TextEditor searchBox;                                                                                      ///< Text box for searching gear items

    // List box components (for legacy support)
    std::unique_ptr<GearListBoxModel> gearListModel; ///< Model for the legacy list box
    std::unique_ptr<DraggableListBox> gearListBox;   ///< Legacy list box component

    // TreeView components (new hierarchical view)
    std::unique_ptr<juce::TreeView> gearTreeView; ///< Tree view for hierarchical display
    std::unique_ptr<GearTreeItem> rootItem;       ///< Root item of the tree view

    // Data
    juce::Array<GearItem> gearItems; ///< Array of all gear items

    // Search state
    juce::String currentSearchText; ///< Current search text

    /**
     * @brief Normalizes text for fuzzy search by removing ignored characters.
     *
     * @param text The text to normalize
     * @return The normalized text (lowercase with ignored characters removed)
     */
    juce::String normalizeForSearch(const juce::String &text) const;

    /**
     * @brief Gets the list of characters to ignore during search.
     *
     * @return Array of characters that should be ignored during fuzzy matching
     */
    static juce::Array<juce::juce_wchar> getIgnoredCharacters();

    INetworkFetcher &fetcher;   ///< Reference to the network fetcher
    CacheManager &cacheManager; ///< Reference to the cache manager
    IFileSystem &fileSystem;    ///< Reference to the file system

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearLibrary)
};

/**
 * @brief Custom TreeViewItem for hierarchical display of gear items.
 *
 * This class provides a hierarchical view of gear items, organizing them
 * by categories and types in a tree structure.
 */
class GearTreeItem : public juce::TreeViewItem
{
public:
    /**
     * @brief Enumeration of item types in the tree view.
     */
    enum class ItemType
    {
        Root,         ///< Root item
        Category,     ///< Category items group
        Gear,         ///< Individual gear items
        RecentlyUsed, ///< Recently used items group
        Favorites,    ///< Favorites items group
        Message       ///< Simple text message (for no results, etc.)
    };

    /**
     * @brief Constructs a new GearTreeItem.
     *
     * @param typeIn The type of the item
     * @param nameIn The name of the item
     * @param ownerIn Pointer to the owning GearLibrary
     * @param cacheManagerIn Reference to the cache manager
     * @param gearItemIn Pointer to the associated gear item (for gear items)
     * @param gearIndexIn Index of the gear item (for gear items)
     */
    GearTreeItem(ItemType typeIn, const juce::String &nameIn, GearLibrary *ownerIn = nullptr,
                 CacheManager *cacheManagerIn = nullptr, GearItem *gearItemIn = nullptr, int gearIndexIn = -1)
        : itemType(typeIn), name(nameIn), owner(ownerIn), cacheManager(cacheManagerIn), gearItem(gearItemIn), gearIndex(gearIndexIn)
    {
    }

    /**
     * @brief Checks if the item might contain sub-items.
     *
     * @return true if the item type is not Gear
     */
    bool mightContainSubItems() override
    {
        return itemType != ItemType::Gear;
    }

    /**
     * @brief Paints the item.
     *
     * @param g The graphics context to paint with
     * @param width The width of the item
     * @param height The height of the item
     */
    void paintItem(juce::Graphics &g, int width, int height) override
    {
        // Suppress selection highlighting for all items for cleaner UI
        // if (isSelected() && itemType != ItemType::Gear)
        //     g.fillAll(juce::Colours::lightblue.darker(0.2f));

        g.setColour(juce::Colours::white);
        g.setFont(itemType == ItemType::Gear ? 14.0f : 16.0f);

        // Handle message items (simple text without tree styling)
        if (itemType == ItemType::Message)
        {
            g.setColour(juce::Colours::lightgrey);
            g.setFont(14.0f);
            g.drawText(name, 4, 0, width - 8, height, juce::Justification::centredLeft);
            return;
        }

        // Draw icon based on type
        int textX = 4;
        juce::String itemText = name;

        // Only draw icons for actual gear items (leaf nodes)
        if (itemType == ItemType::Gear)
        {
            // Draw star icon for favorites first (far left)
            if (gearItem != nullptr && cacheManager != nullptr)
            {
                bool isFavorite = cacheManager->isFavorite(gearItem->unitId);

                const int starSize = 16;
                const int starX = 4; // Far left position
                const int starY = (height - starSize) / 2;

                g.setColour(isFavorite ? juce::Colours::yellow : juce::Colours::lightgrey);

                // Draw a simple star shape rotated 180 degrees
                juce::Path starPath;
                starPath.addStar(juce::Point<float>(starX + starSize / 2, starY + starSize / 2),
                                 5, starSize / 2, starSize / 4);

                // Apply 180 degree rotation around the star center
                juce::AffineTransform rotation = juce::AffineTransform::rotation(juce::MathConstants<float>::pi,
                                                                                 starX + starSize / 2,
                                                                                 starY + starSize / 2);
                starPath.applyTransform(rotation);

                g.fillPath(starPath);
            }

            // Move text position to account for star
            textX += 24; // Space for star + padding

            const int iconSize = 24;
            const int iconY = (height - iconSize) / 2;

            if (gearItem != nullptr && gearItem->image.isValid())
            {
                // Use the gear item's thumbnail image if available
                g.drawImageWithin(gearItem->image,
                                  textX, iconY,
                                  iconSize, iconSize,
                                  juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            }
            else
            {
                // Fallback to green circle
                g.setColour(juce::Colours::greenyellow);
                g.fillEllipse(textX + 4, height / 2 - 6, 12, 12);
            }

            g.setColour(juce::Colours::white);
            textX += 30; // Wider space for gear items with images
        }

        g.drawText(itemText, textX, 0, width - textX, height, juce::Justification::centredLeft);
    }

    /**
     * @brief Handles changes in item openness.
     *
     * @param isNowOpen Whether the item is now open
     */
    void itemOpennessChanged(bool isNowOpen) override
    {
        if (isNowOpen && getNumSubItems() == 0)
            refreshSubItems();
    }

    /**
     * @brief Refreshes the sub-items of this item.
     */
    void refreshSubItems()
    {
        clearSubItems();

        // No owner means we can't properly populate
        if (owner == nullptr)
            return;

        if (itemType == ItemType::Root)
        {
            // Add Recently Used group
            addSubItem(new GearTreeItem(ItemType::RecentlyUsed, "Recently Used", owner, cacheManager));

            // Add Favorites group
            addSubItem(new GearTreeItem(ItemType::Favorites, "My Gear", owner, cacheManager));

            // Add Categories group
            addSubItem(new GearTreeItem(ItemType::Category, "Categories", owner, cacheManager));
        }
        else if (itemType == ItemType::RecentlyUsed)
        {
            // Get recently used items from cache
            if (cacheManager != nullptr)
            {
                juce::StringArray recentlyUsed = cacheManager->getRecentlyUsed(CacheManager::MAX_RECENTLY_USED); // Show up to MAX_RECENTLY_USED items

                if (!recentlyUsed.isEmpty())
                {
                    // Get all items from the library
                    const auto &items = owner->getItems();

                    // Add each recently used item
                    for (const auto &unitId : recentlyUsed)
                    {
                        // Find the gear item in the library
                        for (int i = 0; i < items.size(); ++i)
                        {
                            const auto &item = items.getReference(i);
                            if (item.unitId == unitId)
                            {
                                addSubItem(new GearTreeItem(ItemType::Gear, item.name, owner, cacheManager,
                                                            const_cast<GearItem *>(&item), i));
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (itemType == ItemType::Category && name == "Categories")
        {
            // Get all items from the library
            const auto &items = owner->getItems();

            // Collect unique categories from the items
            juce::StringArray categories;

            // First pass - gather all unique categories
            for (int i = 0; i < items.size(); ++i)
            {
                const auto &item = items.getReference(i);

                // Get the category string or derive it from the enum
                juce::String category;

                if (!item.categoryString.isEmpty())
                {
                    category = item.categoryString.toLowerCase();
                }
                else
                {
                    // Fallback to enum if string is empty
                    switch (item.category)
                    {
                    case GearCategory::EQ:
                        category = "equalizer";
                        break;
                    case GearCategory::Compressor:
                        category = "compressor";
                        break;
                    case GearCategory::Preamp:
                        category = "preamp";
                        break;
                    case GearCategory::Other:
                        category = "other";
                        break;
                    }
                }

                // Add to our unique list if not already present
                if (!category.isEmpty() && !categories.contains(category))
                    categories.add(category);
            }

            // Sort categories alphabetically
            categories.sort(true);

            // Add a tree item for each category
            for (const auto &category : categories)
            {
                // Format category name for display (capitalize first letter)
                juce::String displayName = category.substring(0, 1).toUpperCase() + category.substring(1);
                addSubItem(new GearTreeItem(ItemType::Category, displayName, owner, cacheManager));
            }
        }
        else if (itemType == ItemType::Category)
        {
            // Find items matching this category
            const auto &items = owner->getItems();
            bool hasItems = false;

            for (int i = 0; i < items.size(); ++i)
            {
                const auto &item = items.getReference(i);

                // Check if this item matches the category
                bool matches = false;

                if (name.equalsIgnoreCase("EQ") || name.equalsIgnoreCase("Equalizer"))
                {
                    matches = (item.category == GearCategory::EQ ||
                               item.categoryString.equalsIgnoreCase("equalizer") ||
                               item.categoryString.equalsIgnoreCase("eq"));
                }
                else if (name.equalsIgnoreCase("Compressor"))
                {
                    matches = (item.category == GearCategory::Compressor ||
                               item.categoryString.equalsIgnoreCase("compressor"));
                }
                else if (name.equalsIgnoreCase("Preamp"))
                {
                    matches = (item.category == GearCategory::Preamp ||
                               item.categoryString.equalsIgnoreCase("preamp"));
                }
                else if (name.equalsIgnoreCase("Other"))
                {
                    matches = (item.category == GearCategory::Other ||
                               item.categoryString.equalsIgnoreCase("other"));
                }
                else
                {
                    // For any other category, match directly with the item's categoryString
                    matches = item.categoryString.equalsIgnoreCase(name);
                }

                if (matches)
                {
                    addSubItem(new GearTreeItem(ItemType::Gear, item.name, owner, cacheManager,
                                                const_cast<GearItem *>(&item), i));
                    hasItems = true;
                }
            }

            // If no items were found for this category, don't expand it
            if (!hasItems)
                setOpen(false);
        }
        else if (itemType == ItemType::Favorites)
        {
            // Get favorite items from cache
            if (cacheManager != nullptr)
            {
                juce::StringArray favorites = cacheManager->getFavorites();

                if (!favorites.isEmpty())
                {
                    // Get all items from the library
                    const auto &items = owner->getItems();

                    // Group favorites by category (same approach as Categories tree)
                    juce::HashMap<juce::String, juce::Array<GearItem *>> categoryGroups;

                    // Collect favorite items and group them by category
                    for (const auto &unitId : favorites)
                    {
                        // Find the gear item in the library
                        for (int i = 0; i < items.size(); ++i)
                        {
                            const auto &item = items.getReference(i);
                            if (item.unitId == unitId)
                            {
                                // Get the category string or derive it from the enum
                                juce::String category;
                                if (!item.categoryString.isEmpty())
                                {
                                    category = item.categoryString.toLowerCase();
                                }
                                else
                                {
                                    // Fallback to enum if string is empty
                                    switch (item.category)
                                    {
                                    case GearCategory::EQ:
                                        category = "equalizer";
                                        break;
                                    case GearCategory::Compressor:
                                        category = "compressor";
                                        break;
                                    case GearCategory::Preamp:
                                        category = "preamp";
                                        break;
                                    case GearCategory::Other:
                                        category = "other";
                                        break;
                                    }
                                }

                                if (!categoryGroups.contains(category))
                                    categoryGroups.set(category, juce::Array<GearItem *>());

                                categoryGroups.getReference(category).add(const_cast<GearItem *>(&item));
                                break;
                            }
                        }
                    }

                    // Add category groups as sub-items
                    for (auto it = categoryGroups.begin(); it != categoryGroups.end(); ++it)
                    {
                        juce::String categoryName = it.getKey();
                        juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                        auto categoryNode = new GearTreeItem(ItemType::Category, displayName, owner, cacheManager);
                        addSubItem(categoryNode);

                        // Add gear items to this category group
                        for (auto *item : it.getValue())
                        {
                            // Find the index of this item in the original array
                            int itemIndex = -1;
                            for (int i = 0; i < items.size(); ++i)
                            {
                                if (&items.getReference(i) == item)
                                {
                                    itemIndex = i;
                                    break;
                                }
                            }

                            if (itemIndex >= 0)
                            {
                                categoryNode->addSubItem(new GearTreeItem(ItemType::Gear, item->name, owner, cacheManager,
                                                                          const_cast<GearItem *>(item), itemIndex));
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Checks if the item is interested in a drag source.
     *
     * @param dragSourceDetails Details about the drag source
     * @return true if the item is interested in the drag source
     */
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/) override
    {
        return false; // We're a source, not a target
    }

    /**
     * @brief Handles an item being dropped.
     *
     * @param dragSourceDetails Details about the drag source
     * @param insertIndex The index where the item was dropped
     */
    void itemDropped(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/,
                     int /*insertIndex*/) override
    {
        // Not a drop target
    }

    /**
     * @brief Handles item click events.
     *
     * @param e The mouse event details
     */
    void itemClicked(const juce::MouseEvent &e) override
    {
        // Handle right-click on Recently Used item
        if (itemType == ItemType::RecentlyUsed && e.mods.isRightButtonDown())
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Clear Recently Used");

            menu.showMenuAsync(juce::PopupMenu::Options(),
                               [this](int result)
                               {
                                   if (result == 1 && owner != nullptr)
                                   {
                                       owner->clearRecentlyUsed();
                                   }
                               });
            return;
        }

        // Handle right-click on Favorites item
        if (itemType == ItemType::Favorites && e.mods.isRightButtonDown())
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Clear My Gear");

            menu.showMenuAsync(juce::PopupMenu::Options(),
                               [this](int result)
                               {
                                   if (result == 1 && owner != nullptr)
                                   {
                                       owner->clearFavorites();
                                   }
                               });
            return;
        }

        // Handle star icon clicks for gear items
        if (itemType == ItemType::Gear && gearItem != nullptr && e.mods.isLeftButtonDown())
        {
            // Check if click is in the left portion of the item where the star would be
            // The star is drawn at the left edge, so check if click is in the left 20 pixels
            const int starAreaWidth = 20;

            if (e.x <= starAreaWidth)
            {
                // Toggle favorite status
                if (cacheManager != nullptr)
                {
                    bool isFavorite = cacheManager->isFavorite(gearItem->unitId);

                    if (isFavorite)
                    {
                        cacheManager->removeFromFavorites(gearItem->unitId);
                    }
                    else
                    {
                        cacheManager->addToFavorites(gearItem->unitId);
                    }
                }

                // Refresh only the favorites section to preserve tree expansion state
                if (owner != nullptr)
                {
                    owner->refreshFavoritesSection();
                }

                return; // Don't proceed with other operations
            }
        }

        // Handle default behavior (expand/collapse) for all other clicks
        TreeViewItem::itemClicked(e);

        // If this is a gear item, make it draggable
        if (itemType == ItemType::Gear && gearItem != nullptr && e.mods.isLeftButtonDown())
        {
            // Find the parent drag container
            juce::Component *comp = getOwnerView();
            if (comp == nullptr)
                return;

            juce::DragAndDropContainer *container = juce::DragAndDropContainer::findParentDragContainerFor(comp);
            if (container == nullptr)
                return;

            // Create a custom drag image
            int itemWidth = 150;
            int itemHeight = 40;

            juce::Image dragImage(juce::Image::ARGB, itemWidth, itemHeight, true);
            juce::Graphics g(dragImage);

            g.setColour(juce::Colours::darkgrey);
            g.fillRoundedRectangle(0.0f, 0.0f, (float)itemWidth, (float)itemHeight, 8.0f);

            // Add a visual indicator
            g.setColour(juce::Colours::greenyellow);
            g.fillEllipse(10, itemHeight / 2 - 6, 12, 12);

            g.setColour(juce::Colours::white);
            g.setFont(14.0f);
            g.drawText(gearItem->name, 30, 5, itemWidth - 40, 30, juce::Justification::centredLeft);

            // Create a structured drag description that the rack can recognize
            juce::String dragDesc = "GEAR:" + juce::String(gearIndex) + ":" + gearItem->name;

            // Calculate the drag image offset from the mouse
            juce::Point<int> imageOffset(e.x - 10, e.y - itemHeight / 2);

            // Use the newer API to start the drag operation
            container->startDragging(dragDesc, comp, dragImage, true, &imageOffset, nullptr);
        }
    }

    /**
     * @brief Sets the visibility of the item.
     *
     * @param shouldBeVisible Whether the item should be visible
     */
    void setVisible(bool shouldBeVisible)
    {
        // In a TreeViewItem, visibility is managed by the TreeView,
        // so we need to implement this manually
        if (getOwnerView() != nullptr)
        {
            if (shouldBeVisible)
            {
                // Make parent items visible and expanded to show this item
                TreeViewItem *parent = getParentItem();
                while (parent != nullptr)
                {
                    parent->setOpen(true);
                    parent = parent->getParentItem();
                }

                // Ensure this item is visible in the tree
                getOwnerView()->scrollToKeepItemVisible(this);
            }
            else
            {
                // If not visible, close this item
                setOpen(false);
            }
        }
    }

    /**
     * @brief Sets the openness of the item.
     *
     * @param shouldBeOpen Whether the item should be open
     */
    void setOpenness(bool shouldBeOpen)
    {
        setOpen(shouldBeOpen);
    }

    juce::String getItemText() const
    {
        return name;
    }

    GearItem *getGearItem() const
    {
        return gearItem;
    }

private:
    ItemType itemType;          ///< Type of this tree item
    juce::String name;          ///< Name of this tree item
    GearLibrary *owner;         ///< Pointer to the owning GearLibrary
    CacheManager *cacheManager; ///< Reference to the cache manager
    GearItem *gearItem;         ///< Associated gear item (for gear items)
    int gearIndex;              ///< Index of the gear item (for gear items)
    bool isVisible{true};       ///< Whether this item is visible
    bool isOpen{false};         ///< Whether this item is open
};
