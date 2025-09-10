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
    setupTreeView();

    populateTree();
}

GearLibraryTree::~GearLibraryTree()
{
    if (treeView)
        treeView->setRootItem(nullptr);
}

void GearLibraryTree::paint(juce::Graphics &g)
{
    // Draw tree view info
    if (treeView)
    {
        g.drawText("TreeView exists - Root item: " + juce::String(rootItem ? "YES" : "NO"),
                   10, 35, getWidth() - 20, 20, juce::Justification::left);
    }
    else
    {
        g.drawText("TreeView is NULL!", 10, 35, getWidth() - 20, 20, juce::Justification::left);
    }
}

void GearLibraryTree::resized()
{

    if (treeView)
    {
        treeView->setBounds(getLocalBounds());
    }
    else
    {
    }
}

void GearLibraryTree::setupTreeView()
{
    // Create standard JUCE TreeView - drag and drop will be handled at TreeViewItem level
    treeView = std::make_unique<juce::TreeView>();
    treeView->setRootItemVisible(true);
    treeView->setColour(juce::TreeView::backgroundColourId, juce::Colours::darkgrey.darker(0.7f));
    treeView->setIndentSize(20);
    treeView->setDefaultOpenness(false);
    treeView->setMultiSelectEnabled(false);
    treeView->setOpenCloseButtonsVisible(true);

    addAndMakeVisible(treeView.get());
}

void GearLibraryTree::populateTree()
{

    rootItem = std::make_unique<GearTreeItem>(GearTreeItem::ItemType::Root, "Gear Library", gearLibrary, cacheManager);

    createRecentlyUsedSection();

    createFavoritesSection();

    createCategoriesSection();

    treeView->setRootItem(rootItem.get());

    treeView->repaint();

}

// Drag and drop is now handled at the TreeViewItem level in GearTreeItem::itemClicked()

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

    auto categoriesNode = new GearTreeItem(GearTreeItem::ItemType::Category, "Categories", gearLibrary, cacheManager);

    // Add categories node to root item
    rootItem->addSubItem(categoriesNode);

    // Get all gear items and group by category
    juce::Array<GearItem *> allItems;
    try
    {
        allItems = gearLibrary.getAllGearItems();
    }
    catch (...)
    {
        // If getting gear items fails, show an error message
        categoriesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "Error loading gear items", gearLibrary, cacheManager));
        return;
    }

    if (allItems.isEmpty())
    {
        categoriesNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Message, "No gear items available", gearLibrary, cacheManager));
    }
    else
    {
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

// Mouse event handling for drag and drop

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

    // For gear items, draw thumbnail if available
    if (itemType == ItemType::Gear && gearItem)
    {
        // Draw thumbnail if available
        if (!gearItem->thumbnailImage.isNull())
        {
            int thumbnailSize = height - 4; // Leave 2px margin
            auto thumbnailArea = juce::Rectangle<int>(2, 2, thumbnailSize, thumbnailSize);

            // Draw thumbnail with rounded corners
            g.setColour(juce::Colours::darkgrey);
            g.fillRoundedRectangle(thumbnailArea.toFloat(), 3.0f);

            g.drawImageWithin(gearItem->thumbnailImage,
                              thumbnailArea.getX(), thumbnailArea.getY(),
                              thumbnailArea.getWidth(), thumbnailArea.getHeight(),
                              juce::RectanglePlacement::centred);

            // Adjust text area to account for thumbnail
            area.removeFromLeft(thumbnailSize + 8); // thumbnail + margin
        }
    }

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
        else if (e.mods.isLeftButtonDown())
        {
            // Handle single left click - this is where we initiate drag and drop

            // Find the parent drag container
            juce::Component *comp = getOwnerView();
            if (comp == nullptr)
            {
                return;
            }

            juce::DragAndDropContainer *container = juce::DragAndDropContainer::findParentDragContainerFor(comp);
            if (container == nullptr)
            {
                return;
            }


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

            // Find the gear index in the library
            auto allItems = gearLibrary.getAllGearItems();
            int gearIndex = -1;
            for (int i = 0; i < allItems.size(); ++i)
            {
                if (allItems[i]->unitId == gearItem->unitId)
                {
                    gearIndex = i;
                    break;
                }
            }

            if (gearIndex >= 0)
            {
                // Create drag data in the format expected by the Rack (lowercase "gear:")
                juce::String dragDesc = "gear:" + gearItem->unitId;

                // Calculate the drag image offset from the mouse
                juce::Point<int> imageOffset(e.x - 10, e.y - itemHeight / 2);

                // Start the drag operation using JUCE's built-in system
                container->startDragging(dragDesc, comp, dragImage, true, &imageOffset, nullptr);
            }
            else
            {
            }
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
        return itemName;
    case ItemType::Category:
        // Category names are lowercase and singular, so we need to modify the display text
        if (itemName.toLowerCase() != "categories")
            return itemName.substring(0, 1).toUpperCase() + itemName.substring(1) + "s";
        else
            return itemName;
    case ItemType::Gear:
        return itemName;
    case ItemType::RecentlyUsed:
        return itemName;
    case ItemType::Favorites:
        return itemName;
    case ItemType::Message:
        return "NOTE: " + itemName;
    default:
        return itemName;
    }
}

juce::Colour GearTreeItem::getItemColour() const
{
    switch (itemType)
    {
    case ItemType::Root:
        return juce::Colours::lightgrey;
    case ItemType::Category:
        return juce::Colours::lightgrey;
    case ItemType::Gear:
        return juce::Colours::white;
    case ItemType::RecentlyUsed:
        return juce::Colours::lightgrey;
    case ItemType::Favorites:
        return juce::Colours::lightgrey;
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
         } });
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
                                                    "Category: " +
            gearItem->categoryString + "\n"
                                       "Type: " +
            gearItem->getTypeString() + "\n"
                                        "Version: " +
            gearItem->version + "\n"
                                "Controls: " +
            juce::String(gearItem->getNumControls()) + "\n"
                                                       "Description: " +
            gearItem->description + "\n"
                                    "Tags: " +
            gearItem->tags.joinIntoString(", "),
        "OK");
}
