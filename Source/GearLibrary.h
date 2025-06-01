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

/**
 * @brief Enumeration of filter categories for gear items.
 */
enum class FilterCategory
{
    All,     ///< Show all items
    Type,    ///< Filter by gear type
    Category ///< Filter by gear category
};

/**
 * @brief Structure representing a filter option in the gear library.
 */
struct FilterOption
{
    juce::String displayName; ///< User-friendly name for the filter
    FilterCategory category;  ///< Category of the filter
    juce::String value;       ///< Value to filter by
};

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
     * @brief Constructs a new GearLibrary.
     */
    GearLibrary();

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
     * @brief Loads filter options asynchronously.
     */
    void loadFiltersAsync();

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
        DBG("GearLibrary::getFullUrl called with path: " + relativePath);

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

        DBG("Full URL constructed: " + result);
        return result;
    }

private:
    /**
     * @brief Parses the gear library JSON data.
     *
     * @param jsonData The JSON string containing gear library data
     */
    void parseGearLibrary(const juce::String &jsonData);

    /**
     * @brief Parses filter options from JSON data.
     *
     * @param jsonData The JSON string containing filter options
     */
    void parseFilterOptions(const juce::String &jsonData);

    /**
     * @brief Updates the filter box state.
     */
    void updateFilterBox();

    // UI components
    juce::Label titleLabel{"titleLabel", "Gear Library"};                                                                    ///< Title label for the library
    juce::TextEditor searchBox;                                                                                              ///< Search box for filtering items
    juce::DrawableButton refreshButton{"RefreshButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};         ///< Button to refresh the library
    juce::DrawableButton addUserGearButton{"AddUserGearButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground}; ///< Button to add custom gear

    // List box components (for legacy support)
    std::unique_ptr<GearListBoxModel> gearListModel; ///< Model for the legacy list box
    std::unique_ptr<DraggableListBox> gearListBox;   ///< Legacy list box component

    // TreeView components (new hierarchical view)
    std::unique_ptr<juce::TreeView> gearTreeView; ///< Tree view for hierarchical display
    std::unique_ptr<GearTreeItem> rootItem;       ///< Root item of the tree view

    // Data
    juce::Array<GearItem> gearItems;         ///< Array of all gear items
    juce::Array<FilterOption> filterOptions; ///< Available filter options

    // Filter state
    juce::String currentSearchText;                                                 ///< Current search text
    std::pair<FilterCategory, juce::String> currentFilter{FilterCategory::All, ""}; ///< Current filter selection

    /**
     * @brief Determines if a gear item should be shown based on current filters.
     *
     * @param item The gear item to check
     * @return true if the item should be shown
     */
    bool shouldShowItem(const GearItem &item) const;

    /**
     * @brief Updates the filtered items in both list and tree views.
     */
    void updateFilteredItems();

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
        Root,     ///< Root node
        Category, ///< Category node
        Type,     ///< Type node
        Gear      ///< Individual gear item
    };

    /**
     * @brief Constructs a new GearTreeItem.
     *
     * @param typeIn The type of the item
     * @param nameIn The name of the item
     * @param ownerIn Pointer to the owning GearLibrary
     * @param gearItemIn Pointer to the associated gear item (for gear items)
     * @param gearIndexIn Index of the gear item (for gear items)
     */
    GearTreeItem(ItemType typeIn, const juce::String &nameIn, GearLibrary *ownerIn = nullptr,
                 GearItem *gearItemIn = nullptr, int gearIndexIn = -1)
        : itemType(typeIn), name(nameIn), owner(ownerIn), gearItem(gearItemIn), gearIndex(gearIndexIn)
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
     * @brief Gets a unique name for the item.
     *
     * @return A unique string identifier for the item
     */
    juce::String getUniqueName() const override
    {
        if (itemType == ItemType::Gear && gearItem != nullptr)
            return "gear_" + gearItem->unitId + "_" + juce::String(gearIndex);

        return "category_" + name + "_" + juce::String((int)itemType);
    }

    /**
     * @brief Paints the item in the tree view.
     *
     * @param g The graphics context to paint with
     * @param width The width of the item
     * @param height The height of the item
     */
    void paintItem(juce::Graphics &g, int width, int height) override
    {
        if (isSelected())
            g.fillAll(juce::Colours::lightblue.darker(0.2f));

        g.setColour(juce::Colours::white);
        g.setFont(itemType == ItemType::Gear ? 14.0f : 16.0f);

        // Draw icon based on type
        int textX = 4;
        juce::String itemText = name;

        // Only draw icons for actual gear items (leaf nodes)
        if (itemType == ItemType::Gear)
        {
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
            // Add Categories group
            addSubItem(new GearTreeItem(ItemType::Category, "Categories", owner));
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
                addSubItem(new GearTreeItem(ItemType::Category, displayName, owner));
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
                    addSubItem(new GearTreeItem(ItemType::Gear, item.name, owner,
                                                const_cast<GearItem *>(&item), i));
                    hasItems = true;
                }
            }

            // If no items were found for this category, don't expand it
            if (!hasItems)
                setOpen(false);
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
        // First handle the default behavior
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

            DBG("Started dragging item: " + dragDesc);
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
    ItemType itemType;    ///< Type of this tree item
    juce::String name;    ///< Name of this tree item
    GearLibrary *owner;   ///< Pointer to the owning GearLibrary
    GearItem *gearItem;   ///< Associated gear item (for gear items)
    int gearIndex;        ///< Index of the gear item (for gear items)
    bool isVisible{true}; ///< Whether this item is visible
    bool isOpen{false};   ///< Whether this item is open
};