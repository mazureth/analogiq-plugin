#pragma once

#include <JuceHeader.h>
#include "DraggableListBox.h"
#include "GearItem.h"

// FilterCategory enum
enum class FilterCategory
{
    All,
    Type,
    Category
};

// Struct to represent a filter option
struct FilterOption
{
    juce::String displayName;
    FilterCategory category;
    juce::String value;
};

// Forward declarations
class GearListBoxModel;
class GearTreeItem;

class GearLibrary : public juce::Component,
                    public juce::Button::Listener
{
public:
    GearLibrary();
    ~GearLibrary() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // ListBox methods
    int getNumRows();
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected);
    juce::Component *refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate);
    void listBoxItemClicked(int row, const juce::MouseEvent &e);
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent &e);

    // Mouse event handlers
    void mouseDown(const juce::MouseEvent &e) override;
    void mouseDrag(const juce::MouseEvent &e) override;

    // Button listener
    void buttonClicked(juce::Button *button) override;

    // Get a gear item by index
    GearItem *getGearItem(int index);

    // Get the full array of items
    const juce::Array<GearItem> &getItems() const { return gearItems; }

    // Load gear library from remote or local cache
    void loadLibraryAsync();
    void loadFiltersAsync();
    void loadGearItemsAsync();
    void saveLibraryAsync();
    void addItem(const juce::String &name, const juce::String &category, const juce::String &description, const juce::String &manufacturer);

private:
    // JSON parsing
    void parseGearLibrary(const juce::String &jsonData);
    void parseFilterOptions(const juce::String &jsonData);
    void updateFilterBox();

    // UI components
    juce::Label titleLabel{"titleLabel", "Gear Library"};
    juce::TextEditor searchBox;
    juce::DrawableButton refreshButton{"RefreshButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};
    juce::DrawableButton addUserGearButton{"AddUserGearButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};

    // List box components (for legacy support)
    std::unique_ptr<GearListBoxModel> gearListModel;
    std::unique_ptr<DraggableListBox> gearListBox;

    // TreeView components (new hierarchical view)
    std::unique_ptr<juce::TreeView> gearTreeView;
    std::unique_ptr<GearTreeItem> rootItem;

    // Data
    juce::Array<GearItem> gearItems;
    juce::Array<FilterOption> filterOptions;

    // Filter state
    juce::String currentSearchText;
    std::pair<FilterCategory, juce::String> currentFilter{FilterCategory::All, ""};

    // Filtering
    bool shouldShowItem(const GearItem &item) const;
    void updateFilteredItems();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearLibrary)
};

//==============================================================================
// Custom TreeViewItem for hierarchical display
class GearTreeItem : public juce::TreeViewItem
{
public:
    enum class ItemType
    {
        Root,
        Category,
        Type,
        Gear
    };

    GearTreeItem(ItemType typeIn, const juce::String &nameIn, GearLibrary *ownerIn = nullptr,
                 GearItem *gearItemIn = nullptr, int gearIndexIn = -1)
        : itemType(typeIn), name(nameIn), owner(ownerIn), gearItem(gearItemIn), gearIndex(gearIndexIn)
    {
    }

    bool mightContainSubItems() override
    {
        return itemType != ItemType::Gear;
    }

    juce::String getUniqueName() const override
    {
        if (itemType == ItemType::Gear && gearItem != nullptr)
            return "gear_" + gearItem->unitId + "_" + juce::String(gearIndex);

        return "category_" + name + "_" + juce::String((int)itemType);
    }

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

    void itemOpennessChanged(bool isNowOpen) override
    {
        if (isNowOpen && getNumSubItems() == 0)
            refreshSubItems();
    }

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

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/) override
    {
        return false; // We're a source, not a target
    }

    void itemDropped(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/,
                     int /*insertIndex*/) override
    {
        // Not a drop target
    }

    // Override the itemClicked method to start drag operations for gear items
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

    GearItem *getGearItem() const { return gearItem; }
    int getGearIndex() const { return gearIndex; }

    // Get the item's display text
    juce::String getItemText() const { return name; }

private:
    ItemType itemType;
    juce::String name;
    GearLibrary *owner;
    GearItem *gearItem = nullptr;
    int gearIndex = -1;
};