/**
 * @file GearLibrary.cpp
 * @brief Implementation of the GearLibrary class for managing audio gear items.
 *
 * This file contains the implementation of the GearLibrary class, which provides
 * a user interface for browsing, searching, and managing audio gear items.
 * It includes both a legacy list view and a new hierarchical tree view.
 */

#include "GearLibrary.h"

/**
 * @brief ListBoxModel adapter for the GearLibrary.
 *
 * This class adapts the GearLibrary to work with JUCE's ListBox component,
 * providing the necessary interface for displaying gear items in a list.
 */

// DraggableListBox is now defined in DraggableListBox.h

/**
 * @brief Constructs a new GearLibrary.
 *
 * Initializes the UI components including the title label, search box,
 * refresh button, add user gear button, and both list and tree views.
 */
GearLibrary::GearLibrary(INetworkFetcher &networkFetcher, IFileSystem &fileSystem, CacheManager &cacheManager, PresetManager &presetManager)
    : networkFetcher(networkFetcher), fileSystem(fileSystem), cacheManager(cacheManager), presetManager(presetManager)
{
    // Set up title label
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Set up search box
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colours::grey);
    searchBox.setJustification(juce::Justification::centredLeft);
    searchBox.onTextChange = [this]
    {
        currentSearchText = searchBox.getText().trim().toLowerCase();
        updateFilteredItems();
    };
    addAndMakeVisible(searchBox);

    // Set up refresh button with Unicode character (adjusted settings)
    refreshButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::darkgrey);
    refreshButton.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::darkgrey.brighter(0.2f));
    refreshButton.addListener(this);
    refreshButton.setTooltip("Refresh List");

    // Create refresh icon using Unicode character
    auto normalIcon = std::make_unique<juce::DrawableText>();
    juce::String unicodeArrow(CharPointer_UTF8("\xE2\x86\xBB"));
    normalIcon->setText(unicodeArrow); // Unicode U+27F2 (ANTICLOCKWISE GAPPED CIRCLE ARROW)
    normalIcon->setFont(juce::Font(24.0f, juce::Font::plain), true);
    normalIcon->setColour(juce::Colours::white);
    normalIcon->setJustification(juce::Justification::centred);

    // Use the same icon for normal and hover states
    refreshButton.setImages(normalIcon.get(), nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr, nullptr);
    addAndMakeVisible(refreshButton);

    // Set up the tree view (new hierarchical view)
    gearTreeView = std::make_unique<juce::TreeView>();
    gearTreeView->setRootItemVisible(false);
    gearTreeView->setColour(juce::TreeView::backgroundColourId, juce::Colours::darkgrey.darker(0.7f));
    gearTreeView->setIndentSize(20);
    gearTreeView->setDefaultOpenness(false); // Collapsed by default
    gearTreeView->setMultiSelectEnabled(false);
    gearTreeView->setOpenCloseButtonsVisible(true);

    rootItem = std::make_unique<GearTreeItem>(GearTreeItem::ItemType::Root, "Gear Library", this);
    gearTreeView->setRootItem(rootItem.get());

    addAndMakeVisible(gearTreeView.get());

    // Don't load initial data automatically - let the plugin load it when ready
    // loadLibraryAsync();
}

/**
 * @brief Destructor for GearLibrary.
 *
 * Ensures proper cleanup of the tree view root item.
 */
GearLibrary::~GearLibrary()
{
    // Important: set root item to null before the TreeView is deleted
    gearTreeView->setRootItem(nullptr);
}

/**
 * @brief Paints the GearLibrary component.
 *
 * @param g The graphics context to paint with
 */
void GearLibrary::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
}

/**
 * @brief Handles resizing of the GearLibrary component.
 *
 * Updates the bounds of all child components based on the new size.
 */
void GearLibrary::resized()
{
    auto bounds = getLocalBounds();

    // Title area
    titleLabel.setBounds(bounds.removeFromTop(30));

    // Control area
    auto controlArea = bounds.removeFromTop(40);
    refreshButton.setBounds(controlArea.removeFromRight(80).reduced(5));
    searchBox.setBounds(controlArea.reduced(5));

    // Tree view (new hierarchical view)
    if (gearTreeView)
    {
        gearTreeView->setBounds(bounds);
    }
}

/**
 * @brief Gets the list of characters to ignore during search.
 *
 * @return Array of characters that should be ignored during fuzzy matching
 */
juce::Array<juce::juce_wchar> GearLibrary::getIgnoredCharacters()
{
    return {
        '-',  // Hyphens (e.g., "LA-2A" matches "la2a")
        ' ',  // Spaces (e.g., "Tube Compressor" matches "tubecompressor")
        '_',  // Underscores
        '.',  // Dots
        '(',  // Parentheses
        ')',  // Parentheses
        '[',  // Brackets
        ']',  // Brackets
        '/',  // Forward slashes
        '\\', // Backward slashes
        '&',  // Ampersands
        '+',  // Plus signs
        '=',  // Equals signs
        '#'   // Hash symbols
    };
}

/**
 * @brief Normalizes text for fuzzy search by removing ignored characters.
 *
 * @param text The text to normalize
 * @return The normalized text (lowercase with ignored characters removed)
 */
juce::String GearLibrary::normalizeForSearch(const juce::String &text) const
{
    juce::String normalized = text.toLowerCase();
    auto ignoredChars = getIgnoredCharacters();

    // Build a string of characters to remove
    juce::String charsToRemove;
    for (auto ignoredChar : ignoredChars)
    {
        charsToRemove += ignoredChar;
    }

    // Remove all ignored characters at once
    normalized = normalized.removeCharacters(charsToRemove);

    return normalized;
}

/**
 * @brief Determines if a gear item should be shown based on current search.
 *
 * Checks search text to determine if an item should be visible in the current view.
 * Uses fuzzy matching to ignore common separators and special characters.
 *
 * @param item The gear item to check
 * @return true if the item should be shown
 */
bool GearLibrary::shouldShowItem(const GearItem &item) const
{
    // Check the search text
    if (!currentSearchText.isEmpty())
    {
        // Normalize the search term
        juce::String normalizedSearch = normalizeForSearch(currentSearchText);

        // Debug output
        juce::String normalizedName = normalizeForSearch(item.name);
        juce::String normalizedManufacturer = normalizeForSearch(item.manufacturer);

        // Uncomment the next line for debug output
        // juce::Logger::writeToLog("Search: '" + currentSearchText + "' -> '" + normalizedSearch + "' | Item: '" + item.name + "' -> '" + normalizedName + "' | Match: " + (normalizedName.contains(normalizedSearch) ? "YES" : "NO"));

        bool matchesSearch = false;

        // Check normalized name and manufacturer
        if (normalizedName.contains(normalizedSearch) ||
            normalizedManufacturer.contains(normalizedSearch))
        {
            matchesSearch = true;
        }

        // Check normalized category string
        if (!matchesSearch && !item.categoryString.isEmpty() &&
            normalizeForSearch(item.categoryString).contains(normalizedSearch))
        {
            matchesSearch = true;
        }

        // Check normalized tags
        if (!matchesSearch && item.tags.size() > 0)
        {
            for (const auto &tag : item.tags)
            {
                if (normalizeForSearch(tag).contains(normalizedSearch))
                {
                    matchesSearch = true;
                    break;
                }
            }
        }

        return matchesSearch;
    }

    // If no search text, show all items
    return true;
}

/**
 * @brief Updates the filtered items in both list and tree views.
 *
 * Refreshes the display of items based on current search criteria.
 */
void GearLibrary::updateFilteredItems()
{

    // Then update the tree view
    if (rootItem && gearTreeView)
    {
        bool isSearching = !currentSearchText.isEmpty();

        // Clear and rebuild the tree structure
        rootItem->clearSubItems();

        if (isSearching)
        {
            // Find all matching items
            juce::Array<GearItem *> matchingItems;
            for (int i = 0; i < gearItems.size(); ++i)
            {
                if (shouldShowItem(gearItems.getReference(i)))
                {
                    matchingItems.add(&gearItems.getReference(i));
                }
            }

            if (matchingItems.size() > 0)
            {
                // Get favorites and recently used items
                juce::StringArray favorites = cacheManager.getFavorites();
                juce::StringArray recentlyUsed = cacheManager.getRecentlyUsed();

                // Group matching items by category
                juce::HashMap<juce::String, juce::Array<GearItem *>> categoryGroups;
                juce::Array<GearItem *> matchingFavorites;
                juce::Array<GearItem *> matchingRecentlyUsed;

                for (auto *item : matchingItems)
                {
                    // Check if item is in favorites
                    if (favorites.contains(item->unitId))
                    {
                        matchingFavorites.add(item);
                    }

                    // Check if item is in recently used
                    if (recentlyUsed.contains(item->unitId))
                    {
                        matchingRecentlyUsed.add(item);
                    }

                    // Group by category for the main categories section
                    juce::String category = item->categoryString;
                    if (category.isEmpty())
                    {
                        // Fallback to enum-based category
                        switch (item->category)
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

                    categoryGroups.getReference(category).add(item);
                }

                // Add Recently Used section if there are matching recently used items
                if (matchingRecentlyUsed.size() > 0)
                {
                    auto recentlyUsedNode = new GearTreeItem(GearTreeItem::ItemType::RecentlyUsed, "Recently Used", this, &cacheManager);
                    rootItem->addSubItem(recentlyUsedNode);

                    for (auto *item : matchingRecentlyUsed)
                    {
                        // Find the index of this item in the original array
                        int itemIndex = -1;
                        for (int i = 0; i < gearItems.size(); ++i)
                        {
                            if (&gearItems.getReference(i) == item)
                            {
                                itemIndex = i;
                                break;
                            }
                        }

                        if (itemIndex >= 0)
                        {
                            recentlyUsedNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                        }
                    }
                    recentlyUsedNode->setOpen(true);
                }

                // Add My Gear section if there are matching favorites
                if (matchingFavorites.size() > 0)
                {
                    auto myGearNode = new GearTreeItem(GearTreeItem::ItemType::Favorites, "My Gear", this, &cacheManager);
                    rootItem->addSubItem(myGearNode);

                    // Group matching favorites by category (same approach as Categories tree)
                    juce::HashMap<juce::String, juce::Array<GearItem *>> favoriteCategoryGroups;

                    for (auto *item : matchingFavorites)
                    {
                        // Get the category string or derive it from the enum
                        juce::String category;
                        if (!item->categoryString.isEmpty())
                        {
                            category = item->categoryString.toLowerCase();
                        }
                        else
                        {
                            // Fallback to enum if string is empty
                            switch (item->category)
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

                        if (!favoriteCategoryGroups.contains(category))
                            favoriteCategoryGroups.set(category, juce::Array<GearItem *>());

                        favoriteCategoryGroups.getReference(category).add(item);
                    }

                    // Add category groups as sub-items
                    for (auto it = favoriteCategoryGroups.begin(); it != favoriteCategoryGroups.end(); ++it)
                    {
                        juce::String categoryName = it.getKey();
                        juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                        auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, displayName, this, &cacheManager);
                        myGearNode->addSubItem(categoryNode);

                        // Add gear items to this category group
                        for (auto *item : it.getValue())
                        {
                            // Find the index of this item in the original array
                            int itemIndex = -1;
                            for (int i = 0; i < gearItems.size(); ++i)
                            {
                                if (&gearItems.getReference(i) == item)
                                {
                                    itemIndex = i;
                                    break;
                                }
                            }

                            if (itemIndex >= 0)
                            {
                                categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                            }
                        }
                    }
                    myGearNode->setOpen(true);

                    // Expand all category nodes that contain matching items (same as Categories tree)
                    for (int i = 0; i < myGearNode->getNumSubItems(); ++i)
                    {
                        if (auto categoryNode = dynamic_cast<GearTreeItem *>(myGearNode->getSubItem(i)))
                        {
                            categoryNode->setOpen(true);
                        }
                    }
                }

                // Create the "Categories" node
                auto categoriesNode = new GearTreeItem(GearTreeItem::ItemType::Category, "Categories", this, &cacheManager);
                rootItem->addSubItem(categoriesNode);

                // Add each category that has matching items
                for (auto it = categoryGroups.begin(); it != categoryGroups.end(); ++it)
                {
                    juce::String categoryName = it.getKey();
                    juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                    auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, displayName, this, &cacheManager);
                    categoriesNode->addSubItem(categoryNode);

                    // Add matching items to this category
                    for (auto *item : it.getValue())
                    {
                        // Find the index of this item in the original array
                        int itemIndex = -1;
                        for (int i = 0; i < gearItems.size(); ++i)
                        {
                            if (&gearItems.getReference(i) == item)
                            {
                                itemIndex = i;
                                break;
                            }
                        }

                        if (itemIndex >= 0)
                        {
                            categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                        }
                    }
                }

                // Expand the tree to show all matching items
                rootItem->setOpen(true);
                categoriesNode->setOpen(true);

                // Expand all category nodes that contain matching items
                for (int i = 0; i < categoriesNode->getNumSubItems(); ++i)
                {
                    if (auto categoryNode = dynamic_cast<GearTreeItem *>(categoriesNode->getSubItem(i)))
                    {
                        categoryNode->setOpen(true);
                    }
                }
            }
            else
            {
                // No matching items found - create a simple message
                auto messageNode = new GearTreeItem(GearTreeItem::ItemType::Message, "No units match your search", this, &cacheManager);
                rootItem->addSubItem(messageNode);

                // Expand the root to show the message
                rootItem->setOpen(true);
            }
        }
        else
        {
            // When not searching, restore the normal hierarchical structure
            rootItem->refreshSubItems();

            // Explicitly refresh the Recently Used and Favorites sections to ensure they're populated
            refreshRecentlyUsedSection();
            refreshFavoritesSection();
        }

        gearTreeView->repaint();
    }
}

/**
 * @brief Refreshes the tree view to update recently used items.
 *
 * This method should be called when recently used items change
 * to update the tree view display.
 */
void GearLibrary::refreshTreeView()
{
    if (rootItem && gearTreeView)
    {
        // Refresh the entire tree structure
        rootItem->refreshSubItems();
        gearTreeView->repaint();
    }
}

/**
 * @brief Refreshes only the recently used section of the tree.
 *
 * This method updates only the recently used items without
 * affecting the expansion state of other tree nodes.
 */
void GearLibrary::refreshRecentlyUsedSection()
{
    if (rootItem && gearTreeView)
    {
        // Debug: Check what's in the cache
        juce::StringArray recentlyUsed = cacheManager.getRecentlyUsed();
        // juce::Logger::writeToLog("refreshRecentlyUsedSection: Found " + juce::String(recentlyUsed.size()) + " recently used items");
        // for (const auto &unitId : recentlyUsed)
        // {
        //     juce::Logger::writeToLog("  - " + unitId);
        // }

        // Find the Recently Used item in the tree
        GearTreeItem *recentlyUsedItem = nullptr;
        for (int i = 0; i < rootItem->getNumSubItems(); ++i)
        {
            if (auto item = dynamic_cast<GearTreeItem *>(rootItem->getSubItem(i)))
            {
                if (item->getItemText() == "Recently Used")
                {
                    recentlyUsedItem = item;
                    // juce::Logger::writeToLog("Found existing Recently Used section");
                    break;
                }
            }
        }

        // If Recently Used section doesn't exist, create it
        if (recentlyUsedItem == nullptr)
        {
            // juce::Logger::writeToLog("Recently Used section doesn't exist, creating it");

            // Get recently used items from cache
            juce::Array<GearItem *> matchingRecentlyUsed;

            // Find matching items in our gear library
            for (auto &item : gearItems)
            {
                if (recentlyUsed.contains(item.unitId))
                {
                    matchingRecentlyUsed.add(&item);
                    // juce::Logger::writeToLog("  - Found matching item: " + item.name);
                }
            }

            // Only create the section if there are recently used items
            if (matchingRecentlyUsed.size() > 0)
            {
                recentlyUsedItem = new GearTreeItem(GearTreeItem::ItemType::RecentlyUsed, "Recently Used", this, &cacheManager);
                rootItem->addSubItem(recentlyUsedItem);

                // Add the recently used items to the section
                for (auto *item : matchingRecentlyUsed)
                {
                    // Find the index of this item in the original array
                    int itemIndex = -1;
                    for (int i = 0; i < gearItems.size(); ++i)
                    {
                        if (&gearItems.getReference(i) == item)
                        {
                            itemIndex = i;
                            break;
                        }
                    }

                    if (itemIndex >= 0)
                    {
                        recentlyUsedItem->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                        // juce::Logger::writeToLog("  - Added item to tree: " + item->name);
                    }
                }
                recentlyUsedItem->setOpen(true);
            }
            else
            {
                // juce::Logger::writeToLog("No matching items found in gear library");
            }
        }
        else
        {
            // juce::Logger::writeToLog("Recently Used section exists, refreshing it");

            // Clear existing sub-items and rebuild the Recently Used section
            recentlyUsedItem->clearSubItems();

            // Add each recently used item
            for (const auto &unitId : recentlyUsed)
            {
                // Find the gear item in the library
                for (int i = 0; i < gearItems.size(); ++i)
                {
                    const auto &item = gearItems.getReference(i);
                    if (item.unitId == unitId)
                    {
                        recentlyUsedItem->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item.name, this, &cacheManager,
                                                                      const_cast<GearItem *>(&item), i));
                        // juce::Logger::writeToLog("  - Added item to existing section: " + item.name);
                        break;
                    }
                }
            }
        }

        gearTreeView->repaint();
    }
}

/**
 * @brief Clears the recently used items and refreshes the tree view.
 *
 * This method clears all recently used items from the cache
 * and updates the tree view display.
 */
void GearLibrary::clearRecentlyUsed()
{
    cacheManager.clearRecentlyUsed();
    refreshTreeView();
}

/**
 * @brief Refreshes only the favorites section of the tree.
 *
 * This method updates only the favorite items without
 * affecting the expansion state of other tree nodes.
 */
void GearLibrary::refreshFavoritesSection()
{
    if (rootItem && gearTreeView)
    {
        // Get favorites from cache
        juce::StringArray favorites = cacheManager.getFavorites();
        // juce::Logger::writeToLog("refreshFavoritesSection: Found " + juce::String(favorites.size()) + " favorite items");
        // for (const auto &unitId : favorites)
        // {
        //     juce::Logger::writeToLog("  - " + unitId);
        // }

        // Find the My Gear item in the tree
        GearTreeItem *favoritesItem = nullptr;
        for (int i = 0; i < rootItem->getNumSubItems(); ++i)
        {
            if (auto item = dynamic_cast<GearTreeItem *>(rootItem->getSubItem(i)))
            {
                if (item->getItemText() == "My Gear")
                {
                    favoritesItem = item;
                    // juce::Logger::writeToLog("Found existing My Gear section");
                    break;
                }
            }
        }

        // If My Gear section doesn't exist, create it
        if (favoritesItem == nullptr)
        {
            // juce::Logger::writeToLog("My Gear section doesn't exist, creating it");

            // Find matching items in our gear library
            juce::Array<GearItem *> matchingFavorites;

            for (auto &item : gearItems)
            {
                if (favorites.contains(item.unitId))
                {
                    matchingFavorites.add(&item);
                    // juce::Logger::writeToLog("  - Found matching item: " + item.name);
                }
            }

            // Create the section even if it's empty (like Recently Used)
            favoritesItem = new GearTreeItem(GearTreeItem::ItemType::Favorites, "My Gear", this, &cacheManager);
            rootItem->addSubItem(favoritesItem);

            // Add items if we have any
            if (matchingFavorites.size() > 0)
            {
                // Group matching favorites by category
                juce::HashMap<juce::String, juce::Array<GearItem *>> favoriteCategoryGroups;

                for (auto *item : matchingFavorites)
                {
                    // Get the category string or derive it from the enum
                    juce::String category;
                    if (!item->categoryString.isEmpty())
                    {
                        category = item->categoryString.toLowerCase();
                    }
                    else
                    {
                        // Fallback to enum if string is empty
                        switch (item->category)
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

                    if (!favoriteCategoryGroups.contains(category))
                        favoriteCategoryGroups.set(category, juce::Array<GearItem *>());

                    favoriteCategoryGroups.getReference(category).add(item);
                }

                // Sort categories alphabetically
                juce::StringArray sortedCategoryNames;
                for (auto it = favoriteCategoryGroups.begin(); it != favoriteCategoryGroups.end(); ++it)
                {
                    sortedCategoryNames.add(it.getKey());
                }
                sortedCategoryNames.sort(true); // true = ignore case

                // Add category groups as sub-items
                for (const auto &categoryName : sortedCategoryNames)
                {
                    juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                    auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, displayName, this, &cacheManager);
                    favoritesItem->addSubItem(categoryNode);

                    // Sort items within this category alphabetically
                    auto &categoryItems = favoriteCategoryGroups.getReference(categoryName);
                    std::vector<GearItem *> sortedItems(categoryItems.begin(), categoryItems.end());
                    std::sort(sortedItems.begin(), sortedItems.end(),
                              [](const GearItem *a, const GearItem *b)
                              {
                                  return a->name.compareIgnoreCase(b->name) < 0;
                              });

                    // Add gear items to this category group
                    for (auto *item : sortedItems)
                    {
                        // Find the index of this item in the original array
                        int itemIndex = -1;
                        for (int i = 0; i < gearItems.size(); ++i)
                        {
                            if (&gearItems.getReference(i) == item)
                            {
                                itemIndex = i;
                                break;
                            }
                        }

                        if (itemIndex >= 0)
                        {
                            categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                            // juce::Logger::writeToLog("  - Added item to tree: " + item->name);
                        }
                    }
                }
            }
            else
            {
                // juce::Logger::writeToLog("No matching items found in gear library");
            }
            favoritesItem->setOpen(true);
        }
        else
        {
            // juce::Logger::writeToLog("My Gear section exists, refreshing it");

            // Capture the current tree state before clearing
            juce::HashMap<juce::String, bool> categoryExpansionState;

            for (int i = 0; i < favoritesItem->getNumSubItems(); ++i)
            {
                if (auto categoryItem = dynamic_cast<GearTreeItem *>(favoritesItem->getSubItem(i)))
                {
                    juce::String categoryName = categoryItem->getItemText();
                    categoryExpansionState.set(categoryName, categoryItem->getOpenness());
                    // juce::Logger::writeToLog("Captured expansion state for category: " + categoryName + " = " + (categoryItem->getOpenness() ? "open" : "closed"));
                }
            }

            // Clear existing sub-items and rebuild the My Gear section
            favoritesItem->clearSubItems();

            // Group favorites by category
            juce::HashMap<juce::String, juce::Array<GearItem *>> favoriteCategoryGroups;

            for (const auto &unitId : favorites)
            {
                // Find the gear item in the library
                for (int i = 0; i < gearItems.size(); ++i)
                {
                    const auto &item = gearItems.getReference(i);
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

                        if (!favoriteCategoryGroups.contains(category))
                            favoriteCategoryGroups.set(category, juce::Array<GearItem *>());

                        favoriteCategoryGroups.getReference(category).add(const_cast<GearItem *>(&item));
                        // juce::Logger::writeToLog("  - Added item to existing section: " + item.name);
                        break;
                    }
                }
            }

            // Sort categories alphabetically
            juce::StringArray sortedCategoryNames;
            for (auto it = favoriteCategoryGroups.begin(); it != favoriteCategoryGroups.end(); ++it)
            {
                sortedCategoryNames.add(it.getKey());
            }
            sortedCategoryNames.sort(true); // true = ignore case

            // Add category groups as sub-items and restore their expansion state
            for (const auto &categoryName : sortedCategoryNames)
            {
                juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, displayName, this, &cacheManager);
                favoritesItem->addSubItem(categoryNode);

                // Sort items within this category alphabetically
                auto &categoryItems = favoriteCategoryGroups.getReference(categoryName);
                std::vector<GearItem *> sortedItems(categoryItems.begin(), categoryItems.end());
                std::sort(sortedItems.begin(), sortedItems.end(),
                          [](const GearItem *a, const GearItem *b)
                          {
                              return a->name.compareIgnoreCase(b->name) < 0;
                          });

                // Add gear items to this category group
                for (auto *item : sortedItems)
                {
                    // Find the index of this item in the original array
                    int itemIndex = -1;
                    for (int i = 0; i < gearItems.size(); ++i)
                    {
                        if (&gearItems.getReference(i) == item)
                        {
                            itemIndex = i;
                            break;
                        }
                    }

                    if (itemIndex >= 0)
                    {
                        categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, &cacheManager, item, itemIndex));
                    }
                }

                // Restore the expansion state for this category
                if (categoryExpansionState.contains(displayName))
                {
                    // juce::Logger::writeToLog("Restoring expansion state for category: " + displayName + " = " + (categoryExpansionState[displayName] ? "open" : "closed"));
                    categoryNode->setOpenness(categoryExpansionState[displayName]);
                }
            }
        }

        gearTreeView->repaint();
    }
}

/**
 * @brief Clears the favorites items and refreshes the tree view.
 *
 * This method clears all favorite items from the cache
 * and updates the tree view display.
 */
void GearLibrary::clearFavorites()
{
    cacheManager.clearFavorites();
    refreshTreeView();
}

/**
 * @brief Gets the number of rows in the gear list.
 *
 * @return The total number of gear items in the library
 */
int GearLibrary::getNumRows()
{
    return gearItems.size();
}

/**
 * @brief Loads the gear library data asynchronously.
 *
 * Initiates gear item loading operations.
 */
void GearLibrary::loadLibraryAsync()
{
    // Start async loading operation
    loadGearItemsAsync();
}

/**
 * @brief Loads gear items asynchronously.
 *
 * Fetches gear items from a remote source or uses fallback data
 * if the remote source is unavailable.
 */
void GearLibrary::loadGearItemsAsync()
{
    // Create URL for the remote endpoint using the helper method
    juce::URL url(getFullUrl(RemoteResources::LIBRARY_PATH));

    // Use the injected network fetcher to get the data
    bool success = false;
    juce::String jsonData = networkFetcher.fetchJsonBlocking(url, success);

    if (success && jsonData.isNotEmpty())
    {
        parseGearLibrary(jsonData);
    }
    else
    {
        // TODO: Handle error case
        // juce::Logger::writeToLog("Failed to load gear items from: " + url.toString(false));
    }
}

/**
 * @brief Parses the gear library JSON data.
 *
 * Processes JSON data containing gear items and populates the library.
 * Supports both new and legacy JSON formats.
 *
 * @param jsonData The JSON string containing gear library data
 */
void GearLibrary::parseGearLibrary(const juce::String &jsonData)
{
    // Parse JSON data and populate gear items
    auto json = juce::JSON::parse(jsonData);

    // Check if we have a "units" array in the new format
    if (json.hasProperty("units") && json["units"].isArray())
    {
        auto unitsArray = json["units"].getArray();
        gearItems.clear();

        for (auto &unitJson : *unitsArray)
        {
            if (unitJson.isObject())
            {
                auto obj = unitJson.getDynamicObject();

                // Extract properties using the new format
                juce::String unitId = obj->getProperty("unitId");
                juce::String name = obj->getProperty("name");
                juce::String manufacturer = obj->getProperty("manufacturer");
                juce::String category = obj->getProperty("category");
                juce::String version = obj->getProperty("version");
                juce::String schemaPath = obj->getProperty("schemaPath");
                juce::String thumbnailImage = obj->getProperty("thumbnailImage");

                // Process tags
                juce::StringArray tags;
                if (obj->hasProperty("tags") && obj->getProperty("tags").isArray())
                {
                    auto tagsArray = obj->getProperty("tags").getArray();
                    for (auto &tag : *tagsArray)
                    {
                        tags.add(tag.toString());
                    }
                }

                // Determine slotSize (default to 1)
                int slotSize = obj->hasProperty("slotSize") ? static_cast<int>(obj->getProperty("slotSize")) : 1;

                // Create empty controls array (we'll populate this later when loading the full schema)
                juce::Array<GearControl> controls;

                // Ensure schemaPath is properly formatted using our constants
                if (!schemaPath.startsWith("http") && !schemaPath.isEmpty())
                {
                    // If it's a relative path, ensure it's relative to SCHEMAS_PATH
                    if (!schemaPath.startsWith(RemoteResources::SCHEMAS_PATH) &&
                        !schemaPath.startsWith("/"))
                    {
                        schemaPath = RemoteResources::SCHEMAS_PATH + schemaPath;
                    }
                }

                // Do the same for thumbnail images
                if (!thumbnailImage.startsWith("http") && !thumbnailImage.isEmpty())
                {
                    // If it's a relative path and doesn't start with assets/, add the ASSETS_PATH
                    if (!thumbnailImage.startsWith(RemoteResources::ASSETS_PATH) &&
                        !thumbnailImage.startsWith("/"))
                    {
                        thumbnailImage = RemoteResources::ASSETS_PATH + thumbnailImage;
                    }
                }

                GearItem item(unitId, name, manufacturer, category, version, schemaPath,
                              thumbnailImage, tags, networkFetcher, fileSystem, cacheManager, GearType::Other, GearCategory::Other,
                              slotSize, controls);

                // Add to list
                gearItems.add(item);
            }
        }
    }

    // Update the tree view if we have a root item
    if (rootItem != nullptr)
    {
        rootItem->refreshSubItems();

        // Refresh the recently used section to ensure it's populated on startup
        refreshRecentlyUsedSection();

        // Refresh the favorites section to ensure it's populated on startup
        refreshFavoritesSection();
    }
}

/**
 * @brief Gets a gear item by index.
 *
 * @param index The index of the gear item to retrieve
 * @return Pointer to the gear item, or nullptr if index is invalid
 */
GearItem *GearLibrary::getGearItem(int index)
{
    if (index >= 0 && index < gearItems.size())
        return &(gearItems.getReference(index));

    return nullptr;
}

/**
 * @brief Gets a gear item by unit ID.
 *
 * @param unitId The unit ID of the gear item to retrieve
 * @return Pointer to the gear item, or nullptr if not found
 */
GearItem *GearLibrary::getGearItemByUnitId(const juce::String &unitId)
{
    for (auto &item : gearItems)
    {
        if (item.unitId == unitId)
            return &item;
    }

    return nullptr;
}

/**
 * @brief Handles mouse down events.
 *
 * @param e The mouse event details
 */
void GearLibrary::mouseDown(const juce::MouseEvent &e)
{
    // For now, we're not forwarding mouse events to the list box
    // since we're using the tree view
}

/**
 * @brief Handles mouse drag events.
 *
 * @param e The mouse event details
 */
void GearLibrary::mouseDrag(const juce::MouseEvent &e)
{
    // For now, we're not forwarding mouse events to the list box
    // since we're using the tree view
}

/**
 * @brief Adds a new gear item to the library.
 *
 * Creates and adds a new gear item with the specified properties.
 *
 * @param name The name of the gear item
 * @param category The category of the gear item
 * @param description The description of the gear item
 * @param manufacturer The manufacturer of the gear item
 */
void GearLibrary::addItem(const juce::String &name, const juce::String &category, const juce::String &description, const juce::String &manufacturer)
{
    // Create a new GearItem with default controls
    GearCategory gearCategory;
    if (category == "EQ")
        gearCategory = GearCategory::EQ;
    else if (category == "Preamp")
        gearCategory = GearCategory::Preamp;
    else if (category == "Compressor")
        gearCategory = GearCategory::Compressor;
    else
        gearCategory = GearCategory::Other;

    // Determine a reasonable default type based on common industry standards
    GearType gearType;
    if (name.containsIgnoreCase("500") || name.containsIgnoreCase("lunchbox"))
        gearType = GearType::Series500;
    else
        gearType = GearType::Rack19Inch;

    // Create an empty array of controls
    juce::Array<GearControl> controls;

    // Add the new item to the list
    gearItems.add(GearItem(name, manufacturer, gearType, gearCategory, 1, "", controls, networkFetcher, fileSystem, cacheManager));

    // Update the UI
    if (rootItem != nullptr)
        rootItem->refreshSubItems();
}

/**
 * @brief Saves the gear library data asynchronously.
 *
 * Initiates an asynchronous save operation for the current library state.
 */
void GearLibrary::saveLibraryAsync()
{
    // In a real implementation, this would save to a file or server
    // We'll use a background thread to simulate the save operation
    juce::Thread::launch([this]()
                         {
                             // Simulate saving to a file or server
                             juce::Thread::sleep(500); });
}

/**
 * @brief Handles button click events.
 *
 * Processes clicks on the refresh button.
 *
 * @param button Pointer to the clicked button
 */
void GearLibrary::buttonClicked(juce::Button *button)
{
    if (button == &refreshButton)
    {
        loadLibraryAsync();
    }
}