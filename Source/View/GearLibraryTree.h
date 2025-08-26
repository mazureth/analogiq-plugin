/**
 * @file GearLibraryTree.h
 * @brief Header file for the GearLibraryTree class which displays the gear library in a tree view.
 *
 * This file defines the GearLibraryTree class which provides a hierarchical tree view
 * of the gear library, allowing users to browse categories, favorites, recently used items,
 * and drag gear items to the rack.
 */

#pragma once

#include <JuceHeader.h>
#include "../Shared/ICacheManager.h"
#include "../Model/GearLibrary.h"
#include "../Model/PresetManager.h"

// Forward declarations
class GearTreeItem;

/**
 * @class GearLibraryTree
 * @brief A component that displays the gear library in a hierarchical tree view.
 *
 * The GearLibraryTree class provides a visual interface for browsing the gear library,
 * including categories, favorites, recently used items, and individual gear items.
 * It supports drag and drop operations for moving gear items to the rack.
 */
class GearLibraryTree : public juce::Component,
                       public juce::DragAndDropContainer
{
public:
    /**
     * @brief Constructs a new GearLibraryTree instance.
     *
     * @param gearLibrary Reference to the gear library
     * @param cacheManager Reference to the cache manager
     * @param presetManager Reference to the preset manager
     */
    GearLibraryTree(GearLibrary &gearLibrary,
                    ICacheManager &cacheManager,
                    PresetManager &presetManager);

    /**
     * @brief Destructor for the GearLibraryTree class.
     *
     * Cleans up resources and ensures proper cleanup of tree items.
     */
    ~GearLibraryTree() override;

    /**
     * @brief Paints the gear library tree.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handles resizing of the gear library tree component.
     *
     * Adjusts the layout of the tree view based on the new dimensions.
     */
    void resized() override;

    /**
     * @brief Refreshes the tree view with current gear library data.
     */
    void refreshTree();

    /**
     * @brief Sets the search filter for the tree view.
     *
     * @param searchText The search text to filter by
     */
    void setSearchFilter(const juce::String &searchText);

private:
    // References to dependencies
    GearLibrary &gearLibrary;
    ICacheManager &cacheManager;
    PresetManager &presetManager;

    // Tree view components
    std::unique_ptr<juce::TreeView> treeView;
    std::unique_ptr<GearTreeItem> rootItem;

    // Search functionality
    juce::String currentSearchText;

    // Helper methods
    void setupTreeView();
    void populateTree();
    void createRecentlyUsedSection();
    void createFavoritesSection();
    void createCategoriesSection();
    void applySearchFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearLibraryTree)
};

/**
 * @class GearTreeItem
 * @brief A tree view item representing a node in the gear library tree.
 *
 * The GearTreeItem class represents individual nodes in the gear library tree,
 * including categories, gear items, and special sections like favorites and recently used.
 */
class GearTreeItem : public juce::TreeViewItem
{
public:
    /**
     * @brief Enumeration of item types in the tree.
     */
    enum class ItemType
    {
        Root,           ///< Root node of the tree
        Category,       ///< Category node
        Gear,           ///< Individual gear item
        RecentlyUsed,   ///< Recently used section
        Favorites,      ///< Favorites section
        Message         ///< Informational message
    };

    /**
     * @brief Constructs a new GearTreeItem.
     *
     * @param type The type of this tree item
     * @param name The display name of this item
     * @param gearLibrary Reference to the gear library
     * @param cacheManager Reference to the cache manager
     * @param gearItem Optional reference to a specific gear item
     * @param itemIndex Optional index of the gear item
     */
    GearTreeItem(ItemType type,
                 const juce::String &name,
                 GearLibrary &gearLibrary,
                 ICacheManager &cacheManager,
                 GearItem *gearItem = nullptr,
                 int itemIndex = -1);

    /**
     * @brief Destructor for the GearTreeItem class.
     */
    ~GearTreeItem() override;

    /**
     * @brief Paints this tree item.
     *
     * @param g The graphics context to paint with
     * @param width The width available for painting
     * @param height The height available for painting
     */
    void paintItem(juce::Graphics &g, int width, int height) override;

    /**
     * @brief Handles mouse clicks on this tree item.
     *
     * @param e The mouse event details
     */
    void itemClicked(const juce::MouseEvent &e) override;

    /**
     * @brief Checks if this item can be selected.
     *
     * @return true if this item can be selected
     */
    bool mightContainSubItems() override;

    /**
     * @brief Gets the item type.
     *
     * @return The type of this item
     */
    ItemType getItemType() const { return itemType; }

    /**
     * @brief Gets the associated gear item.
     *
     * @return Pointer to the associated gear item, or nullptr if none
     */
    GearItem *getGearItem() const { return gearItem; }

    /**
     * @brief Gets the item index.
     *
     * @return The index of this item
     */
    int getItemIndex() const { return itemIndex; }

private:
    ItemType itemType;
    juce::String itemName;
    GearLibrary &gearLibrary;
    ICacheManager &cacheManager;
    GearItem *gearItem;
    int itemIndex;

    // Helper methods
    void setupDragAndDrop();
    juce::String getDisplayText() const;
    juce::Colour getItemColour() const;
    void handleGearItemClick();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearTreeItem)
};
