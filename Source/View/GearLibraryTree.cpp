/**
 * @file GearLibraryTree.cpp
 * @brief Implementation of the GearLibraryTree class.
 *
 * This file implements the gear library tree view component, providing a hierarchical
 * display of gear items with drag and drop functionality for moving items to the rack.
 */

#include "GearLibraryTree.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

// GearLibraryTree implementation
GearLibraryTree::GearLibraryTree(GearLibrary &gl, ICacheManager &cm, PresetManager &pm)
    : gearLibrary(gl), cacheManager(cm), presetManager(pm)
{
    juce::Logger::writeToLog("GearLibraryTree: Constructor starting");
    juce::Logger::writeToLog("GearLibraryTree: Calling setupTreeView");
    setupTreeView();
    juce::Logger::writeToLog("GearLibraryTree: setupTreeView completed");

    juce::Logger::writeToLog("GearLibraryTree: Calling populateTree");
    populateTree();
    juce::Logger::writeToLog("GearLibraryTree: populateTree completed");
    juce::Logger::writeToLog("GearLibraryTree: Constructor completed successfully");
}

GearLibraryTree::~GearLibraryTree()
{
    if (treeView)
        treeView->setRootItem(nullptr);
}

void GearLibraryTree::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
}

void GearLibraryTree::resized()
{
    if (treeView)
        treeView->setBounds(getLocalBounds());
}

void GearLibraryTree::setupTreeView()
{
    treeView = std::make_unique<juce::TreeView>();
    treeView->setRootItemVisible(false);
    treeView->setColour(juce::TreeView::backgroundColourId, juce::Colours::darkgrey.darker(0.7f));
    treeView->setIndentSize(20);
    treeView->setDefaultOpenness(false);
    treeView->setMultiSelectEnabled(false);
    treeView->setOpenCloseButtonsVisible(true);

    addAndMakeVisible(treeView.get());
}

void GearLibraryTree::populateTree()
{
    juce::Logger::writeToLog("GearLibraryTree: populateTree starting");

    juce::Logger::writeToLog("GearLibraryTree: Creating root item");
    rootItem = std::make_unique<GearTreeItem>(GearTreeItem::ItemType::Root, "Gear Library", gearLibrary, cacheManager);
    juce::Logger::writeToLog("GearLibraryTree: Root item created successfully");

    juce::Logger::writeToLog("GearLibraryTree: Calling createRecentlyUsedSection");
    createRecentlyUsedSection();
    juce::Logger::writeToLog("GearLibraryTree: createRecentlyUsedSection completed");

    juce::Logger::writeToLog("GearLibraryTree: Calling createFavoritesSection");
    createFavoritesSection();
    juce::Logger::writeToLog("GearLibraryTree: createFavoritesSection completed");

    juce::Logger::writeToLog("GearLibraryTree: Calling createCategoriesSection");
    createCategoriesSection();
    juce::Logger::writeToLog("GearLibraryTree: createCategoriesSection completed");

    juce::Logger::writeToLog("GearLibraryTree: Setting root item in tree view");
    treeView->setRootItem(rootItem.get());
    juce::Logger::writeToLog("GearLibraryTree: Root item set successfully");

    juce::Logger::writeToLog("GearLibraryTree: Calling repaint on tree view");
    treeView->repaint();
    juce::Logger::writeToLog("GearLibraryTree: Repaint completed");

    juce::Logger::writeToLog("GearLibraryTree: populateTree completed successfully");
}

void GearLibraryTree::createRecentlyUsedSection()
{
    auto recentlyUsedNode = new GearTreeItem(GearTreeItem::ItemType::RecentlyUsed, "Recently Used", gearLibrary, cacheManager);
    rootItem->addSubItem(recentlyUsedNode);

    // Get recently used items from cache manager
    auto recentlyUsedIds = cacheManager.getRecentlyUsed(ICacheManager::MAX_RECENTLY_USED);
    
    if (recentlyUsedIds.isEmpty())
    {
        recentlyUsedNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "No recently used items", gearLibrary, cacheManager));
    }
    else
    {
        // Get gear items for recently used IDs
        for (const auto &unitId : recentlyUsedIds)
        {
            auto gearItem = gearLibrary.getGearItem(unitId);
            if (gearItem)
            {
                recentlyUsedNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, gearItem->name, gearLibrary, cacheManager, gearItem, -1));
            }
        }
    }
}

void GearLibraryTree::createFavoritesSection()
{
    auto favoritesNode = new GearTreeItem(GearTreeItem::ItemType::Favorites, "My Gear", gearLibrary, cacheManager);
    rootItem->addSubItem(favoritesNode);

    // Get favorites from cache manager
    auto favoriteIds = cacheManager.getFavorites();
    
    if (favoriteIds.isEmpty())
    {
        favoritesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "No favorites yet", gearLibrary, cacheManager));
    }
    else
    {
        // Get gear items for favorite IDs
        for (const auto &unitId : favoriteIds)
        {
            auto gearItem = gearLibrary.getGearItem(unitId);
            if (gearItem)
            {
                favoritesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, gearItem->name, gearLibrary, cacheManager, gearItem, -1));
            }
        }
    }
}

void GearLibraryTree::createCategoriesSection()
{
    juce::Logger::writeToLog("GearLibraryTree: createCategoriesSection starting");

    juce::Logger::writeToLog("GearLibraryTree: Creating categories node");
    auto categoriesNode = new GearTreeItem(GearTreeItem::ItemType::Category, "Categories", gearLibrary, cacheManager);
    juce::Logger::writeToLog("GearLibraryTree: Categories node created successfully");

    // Add categories node to root item
    rootItem->addSubItem(categoriesNode);
    juce::Logger::writeToLog("GearLibraryTree: Categories node added to root item");

    // Get all gear items and group by category
    juce::Array<GearItem *> allItems;
    try
    {
        juce::Logger::writeToLog("GearLibraryTree: Calling gearLibrary.getAllGearItems()");
        allItems = gearLibrary.getAllGearItems();
        juce::Logger::writeToLog("GearLibraryTree: getAllGearItems() returned " + juce::String(allItems.size()) + " items");
    }
    catch (...)
    {
        juce::Logger::writeToLog("GearLibraryTree: Exception caught in getAllGearItems(), showing error message");
        // If getting gear items fails, show an error message
        categoriesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "Error loading gear items", gearLibrary, cacheManager));
        return;
    }

    if (allItems.isEmpty())
    {
        juce::Logger::writeToLog("GearLibraryTree: No gear items found, showing placeholder message");
        categoriesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "No gear items available", gearLibrary, cacheManager));
    }
    else
    {
        juce::Logger::writeToLog("GearLibraryTree: Processing " + juce::String(allItems.size()) + " gear items");
        std::map<juce::String, juce::Array<const GearItem *>> categorizedItems;

        for (const auto &item : allItems)
        {
            juce::String category = item->categoryString;
            if (category.isEmpty())
                category = "Uncategorized";

            categorizedItems[category].add(item);
        }

        // Create category nodes and add gear items
        for (const auto &[category, items] : categorizedItems)
        {
            auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, category, gearLibrary, cacheManager);
            categoriesNode->addSubItem(categoryNode);

            // Sort items alphabetically
            auto sortedItems = items;
            std::sort(sortedItems.begin(), sortedItems.end(), [](const GearItem *a, const GearItem *b)
                      { return a->name.compareIgnoreCase(b->name) < 0; });

            for (const auto &item : sortedItems)
            {
                categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, gearLibrary, cacheManager, const_cast<GearItem *>(item), -1));
            }
        }
    }
}

void GearLibraryTree::refreshTree()
{
    if (rootItem)
    {
        rootItem->clearSubItems();
        populateTree();
    }
}

void GearLibraryTree::setSearchFilter(const juce::String &searchText)
{
    currentSearchText = searchText;
    applySearchFilter();
}

void GearLibraryTree::setAdvancedFilters(const juce::String &categoryFilter, 
                                        const juce::String &manufacturerFilter, 
                                        const juce::String &gearTypeFilter)
{
    currentCategoryFilter = categoryFilter;
    currentManufacturerFilter = manufacturerFilter;
    currentGearTypeFilter = gearTypeFilter;
    applySearchFilter();
}

void GearLibraryTree::applySearchFilter()
{
    if (currentSearchText.isEmpty())
    {
        refreshTree();
        return;
    }

    // Clear existing tree
    if (rootItem)
        rootItem->clearSubItems();

    // Create search results section
    auto searchNode = new GearTreeItem(GearTreeItem::ItemType::Category, "Search Results", gearLibrary, cacheManager);
    rootItem->addSubItem(searchNode);

    // Search through all gear items
    juce::Array<GearItem *> allItems;
    try
    {
        allItems = gearLibrary.getAllGearItems();
    }
    catch (...)
    {
        // If getting gear items fails, show an error message
        searchNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "Error loading gear items", gearLibrary, cacheManager));
        return;
    }

    juce::Array<const GearItem *> searchResults;

    for (const auto &item : allItems)
    {
        // Apply text search filter
        bool matchesSearch = currentSearchText.isEmpty() ||
                            item->name.containsIgnoreCase(currentSearchText) ||
                            item->manufacturer.containsIgnoreCase(currentSearchText) ||
                            item->categoryString.containsIgnoreCase(currentSearchText);
        
        // Apply category filter
        bool matchesCategory = currentCategoryFilter.isEmpty() ||
                              item->categoryString.containsIgnoreCase(currentCategoryFilter);
        
        // Apply manufacturer filter
        bool matchesManufacturer = currentManufacturerFilter.isEmpty() ||
                                   item->manufacturer.containsIgnoreCase(currentManufacturerFilter);
        
        // Apply gear type filter
        bool matchesGearType = currentGearTypeFilter.isEmpty() ||
                               item->getTypeString().containsIgnoreCase(currentGearTypeFilter);
        
        if (matchesSearch && matchesCategory && matchesManufacturer && matchesGearType)
        {
            searchResults.add(item);
        }
    }

    if (searchResults.isEmpty())
    {
        searchNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "No items found", gearLibrary, cacheManager));
    }
    else
    {
        // Sort results alphabetically
        std::sort(searchResults.begin(), searchResults.end(), [](const GearItem *a, const GearItem *b)
                  { return a->name.compareIgnoreCase(b->name) < 0; });

        for (const auto &item : searchResults)
        {
            searchNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, gearLibrary, cacheManager, const_cast<GearItem *>(item), -1));
        }
    }

    treeView->repaint();
}

// GearTreeItem implementation
GearTreeItem::GearTreeItem(ItemType type, const juce::String &name, GearLibrary &gl, ICacheManager &cm, GearItem *item, int index)
    : itemType(type), itemName(name), gearLibrary(gl), cacheManager(cm), gearItem(item), itemIndex(index)
{
    setupDragAndDrop();
}

GearTreeItem::~GearTreeItem()
{
    // TreeViewItem handles cleanup automatically
}

void GearTreeItem::paintItem(juce::Graphics &g, int width, int height)
{
    auto area = juce::Rectangle<int>(0, 0, width, height);

    // Set text color based on item type
    g.setColour(getItemColour());
    g.setFont(14.0f);

    // Draw item text
    g.drawText(getDisplayText(), area.removeFromLeft(width - 20), juce::Justification::centredLeft);
}

void GearTreeItem::itemClicked(const juce::MouseEvent &e)
{
    if (itemType == ItemType::Gear && gearItem)
    {
        if (e.mods.isRightButtonDown())
        {
            showContextMenu(e);
        }
        else if (e.getNumberOfClicks() >= 2)
        {
            showGearDetails();
        }
        else
        {
            handleGearItemClick();
        }
    }

    // Toggle expansion for category items
    if (itemType == ItemType::Category || itemType == ItemType::RecentlyUsed || itemType == ItemType::Favorites)
    {
        setOpen(!isOpen());
    }
}

bool GearTreeItem::mightContainSubItems()
{
    return itemType == ItemType::Root ||
           itemType == ItemType::Category ||
           itemType == ItemType::RecentlyUsed ||
           itemType == ItemType::Favorites;
}

void GearTreeItem::setupDragAndDrop()
{
    if (itemType == ItemType::Gear && gearItem)
    {
        // Enable drag and drop for gear items
        // Note: TreeViewItem doesn't have setDragEnabled, so we'll handle this differently
    }
}

juce::String GearTreeItem::getDisplayText() const
{
    switch (itemType)
    {
    case ItemType::Root:
        return "[ROOT] " + itemName;
    case ItemType::Category:
        return "[DIR] " + itemName;
    case ItemType::Gear:
        if (gearItem && cacheManager.isInFavorites(gearItem->unitId))
            return "[GEAR] " + itemName + " ★";
        return "[GEAR] " + itemName;
    case ItemType::RecentlyUsed:
        return "[RECENT] " + itemName;
    case ItemType::Favorites:
        return "[FAV] " + itemName;
    case ItemType::Message:
        return "[INFO] " + itemName;
    default:
        return itemName;
    }
}

juce::Colour GearTreeItem::getItemColour() const
{
    switch (itemType)
    {
    case ItemType::Root:
        return juce::Colours::lightblue;
    case ItemType::Category:
        return juce::Colours::lightgreen;
    case ItemType::Gear:
        return juce::Colours::white;
    case ItemType::RecentlyUsed:
        return juce::Colours::orange;
    case ItemType::Favorites:
        return juce::Colours::yellow;
    case ItemType::Message:
        return juce::Colours::lightgrey;
    default:
        return juce::Colours::white;
    }
}

void GearTreeItem::handleGearItemClick()
{
    if (gearItem)
    {
        // Add to recently used
        cacheManager.addToRecentlyUsed(gearItem->unitId);

        // Repaint the tree to reflect changes
        if (auto treeView = dynamic_cast<juce::TreeView *>(getOwnerView()))
            treeView->repaint();
    }
}

void GearTreeItem::showContextMenu(const juce::MouseEvent &e)
{
    if (!gearItem)
        return;

    juce::PopupMenu menu;
    
    // Check if item is in favorites
    bool isFavorite = cacheManager.isInFavorites(gearItem->unitId);
    
    if (isFavorite)
    {
        menu.addItem(1, "Remove from Favorites");
    }
    else
    {
        menu.addItem(2, "Add to Favorites");
    }
    
    menu.addSeparator();
    menu.addItem(3, "Remove from Recently Used");
    
    // Show menu and handle selection
    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
    {
        switch (result)
        {
            case 1: // Remove from favorites
                cacheManager.removeFromFavorites(gearItem->unitId);
                break;
            case 2: // Add to favorites
                cacheManager.addToFavorites(gearItem->unitId);
                break;
            case 3: // Remove from recently used
                cacheManager.removeFromRecentlyUsed(gearItem->unitId);
                break;
        }
        
                 // Refresh the tree to show changes
         if (auto treeView = dynamic_cast<juce::TreeView *>(getOwnerView()))
         {
             if (auto tree = dynamic_cast<GearLibraryTree *>(treeView->getParentComponent()))
             {
                 tree->refreshTree();
             }
         }
     });
 }

void GearTreeItem::showGearDetails()
{
    if (!gearItem)
        return;

    // Create a simple details dialog
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Gear Details: " + gearItem->name,
        "Manufacturer: " + gearItem->manufacturer + "\n"
        "Category: " + gearItem->categoryString + "\n"
        "Type: " + gearItem->getTypeString() + "\n"
        "Version: " + gearItem->version + "\n"
        "Controls: " + juce::String(gearItem->getNumControls()) + "\n"
        "Description: " + gearItem->description + "\n"
        "Tags: " + gearItem->tags.joinIntoString(", "),
        "OK"
    );
}
