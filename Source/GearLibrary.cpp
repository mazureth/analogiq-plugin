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
class GearListBoxModel : public juce::ListBoxModel
{
public:
    /**
     * @brief Constructs a new GearListBoxModel.
     *
     * @param ownerRef Reference to the owning GearLibrary instance
     */
    GearListBoxModel(GearLibrary &ownerRef) : owner(ownerRef) {}

    /**
     * @brief Gets the number of rows in the list.
     *
     * @return The number of gear items to display
     */
    int getNumRows() override { return owner.getNumRows(); }

    /**
     * @brief Paints a list box item.
     *
     * @param rowNumber The index of the row to paint
     * @param g The graphics context to paint with
     * @param width The width of the item
     * @param height The height of the item
     * @param rowIsSelected Whether the row is currently selected
     */
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        owner.paintListBoxItem(rowNumber, g, width, height, rowIsSelected);
    }

    /**
     * @brief Refreshes a component for a specific row.
     *
     * @param rowNumber The index of the row
     * @param isRowSelected Whether the row is selected
     * @param existingComponentToUpdate Component to update if it exists
     * @return The component to display for the row
     */
    juce::Component *refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate) override
    {
        return owner.refreshComponentForRow(rowNumber, isRowSelected, existingComponentToUpdate);
    }

    /**
     * @brief Handles a list box item click.
     *
     * @param row The index of the clicked row
     * @param e The mouse event details
     */
    void listBoxItemClicked(int row, const juce::MouseEvent &e) override
    {
        owner.listBoxItemClicked(row, e);
    }

    /**
     * @brief Handles a list box item double click.
     *
     * @param row The index of the double-clicked row
     * @param e The mouse event details
     */
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent &e) override
    {
        owner.listBoxItemDoubleClicked(row, e);
    }

private:
    GearLibrary &owner;
};

// DraggableListBox is now defined in DraggableListBox.h

/**
 * @brief Constructs a new GearLibrary.
 *
 * Initializes the UI components including the title label, search box,
 * refresh button, add user gear button, and both list and tree views.
 */
GearLibrary::GearLibrary(INetworkFetcher &networkFetcher, bool autoLoad)
    : fetcher(networkFetcher)
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

    // Set up list box (legacy)
    gearListModel = std::make_unique<GearListBoxModel>(*this);

    gearListBox = std::make_unique<DraggableListBox>("gearListBox", gearListModel.get());
    gearListBox->setRowHeight(40);
    gearListBox->setColour(juce::ListBox::backgroundColourId, juce::Colours::darkgrey.darker(0.7f));
    gearListBox->setMultipleSelectionEnabled(false);
    gearListBox->setVisible(false); // Hide legacy list box
    addAndMakeVisible(gearListBox.get());

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

    // Load initial data only if autoLoad is true
    if (autoLoad)
    {
        loadLibraryAsync();
    }
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

    // List area (legacy)
    if (gearListBox && gearListBox->isVisible())
    {
        gearListBox->setBounds(bounds);
    }

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
        juce::Logger::writeToLog("Search: '" + currentSearchText + "' -> '" + normalizedSearch + "' | Item: '" + item.name + "' -> '" + normalizedName + "' | Match: " + (normalizedName.contains(normalizedSearch) ? "YES" : "NO"));

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
    // First update the legacy list box
    if (gearListBox)
    {
        gearListBox->updateContent();
        gearListBox->repaint();
    }

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
                // Group matching items by category
                juce::HashMap<juce::String, juce::Array<GearItem *>> categoryGroups;

                for (auto *item : matchingItems)
                {
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

                // Create the "Categories" node
                auto categoriesNode = new GearTreeItem(GearTreeItem::ItemType::Category, "Categories", this);
                rootItem->addSubItem(categoriesNode);

                // Add each category that has matching items
                for (auto it = categoryGroups.begin(); it != categoryGroups.end(); ++it)
                {
                    juce::String categoryName = it.getKey();
                    juce::String displayName = categoryName.substring(0, 1).toUpperCase() + categoryName.substring(1);

                    auto categoryNode = new GearTreeItem(GearTreeItem::ItemType::Category, displayName, this);
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
                            categoryNode->addSubItem(new GearTreeItem(GearTreeItem::ItemType::Gear, item->name, this, item, itemIndex));
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
        }
        else
        {
            // When not searching, restore the normal hierarchical structure
            rootItem->refreshSubItems();
        }

        gearTreeView->repaint();
    }
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
 * @brief Paints a list box item.
 *
 * @param rowNumber The index of the row to paint
 * @param g The graphics context to paint with
 * @param width The width of the item
 * @param height The height of the item
 * @param rowIsSelected Whether the row is currently selected
 */
void GearLibrary::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int /*height*/, bool rowIsSelected)
{
    // This is still needed for legacy support or components that might use the list box
    if (rowNumber >= 0 && rowNumber < gearItems.size())
    {
        const auto &item = gearItems[rowNumber];

        // Background
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue.darker(0.2f));
        else
            g.fillAll(juce::Colours::darkgrey);

        // Text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText(item.name, 10, 5, width - 20, 20, juce::Justification::left);

        g.setFont(14.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(item.manufacturer, 10, 25, width - 20, 16, juce::Justification::left);
    }
}

/**
 * @brief Refreshes a component for a specific row in the list box.
 *
 * @param rowNumber The index of the row to refresh
 * @param isRowSelected Whether the row is currently selected
 * @param existingComponentToUpdate The existing component to update, if any
 * @return The component to display for the row, or nullptr if using default painting
 */
juce::Component *GearLibrary::refreshComponentForRow(int /*rowNumber*/, bool /*isRowSelected*/, juce::Component *existingComponentToUpdate)
{
    // We're using paintListBoxItem for now, so return nullptr
    return existingComponentToUpdate;
}

/**
 * @brief Handles a list box item click.
 *
 * @param row The index of the clicked row
 * @param e The mouse event details
 */
void GearLibrary::listBoxItemClicked(int row, const juce::MouseEvent & /*e*/)
{
    // Select the row
    if (gearListBox->isVisible())
        gearListBox->selectRow(row);
}

/**
 * @brief Handles a list box item double click.
 *
 * @param row The index of the double-clicked row
 * @param e The mouse event details
 */
void GearLibrary::listBoxItemDoubleClicked(int row, const juce::MouseEvent & /*e*/)
{
    // Not used with tree view
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
    juce::String jsonData = fetcher.fetchJsonBlocking(url, success);

    if (success && jsonData.isNotEmpty())
    {
        parseGearLibrary(jsonData);
    }
    else
    {
        // TODO: Handle error case
        juce::Logger::writeToLog("Failed to load gear items from: " + url.toString(false));
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
                              thumbnailImage, tags, fetcher, GearType::Other, GearCategory::Other,
                              slotSize, controls);

                // If thumbnailImage is provided, try to load the image
                if (thumbnailImage.isNotEmpty())
                {
                    // For now, we'll just use our placeholder image code in loadImage()
                    item.loadImage();
                }

                // Add to list
                gearItems.add(item);
            }
        }
    }
    // Fallback to the old "gear" array format for backwards compatibility
    else if (json.hasProperty("gear") && json["gear"].isArray())
    {
        auto gearArray = json["gear"].getArray();
        gearItems.clear();

        for (auto &gearJson : *gearArray)
        {
            if (gearJson.isObject())
            {
                auto obj = gearJson.getDynamicObject();

                // Extract properties using the old format
                juce::String name = obj->getProperty("name");
                juce::String manufacturer = obj->getProperty("manufacturer");

                // Determine type
                GearType type;
                juce::String typeStr = obj->getProperty("type");
                if (typeStr == "500Series")
                    type = GearType::Series500;
                else if (typeStr == "Rack19Inch")
                    type = GearType::Rack19Inch;
                else
                    type = GearType::UserCreated;

                // Determine category
                GearCategory category;
                juce::String catStr = obj->getProperty("category");
                if (catStr == "EQ")
                    category = GearCategory::EQ;
                else if (catStr == "Compressor")
                    category = GearCategory::Compressor;
                else if (catStr == "Preamp")
                    category = GearCategory::Preamp;
                else
                    category = GearCategory::Other;

                int slotSize = obj->getProperty("slotSize");
                juce::String thumbnailUrl = obj->getProperty("thumbnailUrl");

                // Create an empty array of controls
                juce::Array<GearControl> controls;

                // Create gear item using the legacy constructor
                GearItem item(name, manufacturer, type, category, slotSize, thumbnailUrl, controls, fetcher);

                // If thumbnailUrl is provided, try to load the image
                if (thumbnailUrl.isNotEmpty())
                {
                    item.loadImage();
                }

                // Add to list
                gearItems.add(item);
            }
        }
    }

    // Update the tree view if we have a root item
    if (rootItem != nullptr)
        rootItem->refreshSubItems();
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
    gearItems.add(GearItem(name, manufacturer, gearType, gearCategory, 1, "", controls, fetcher));

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