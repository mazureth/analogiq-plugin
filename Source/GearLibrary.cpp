/**
 * @file GearLibrary.cpp
 * @brief Implementation of the gear library management system.
 *
 * This file implements the GearLibrary class and its supporting components,
 * providing functionality for:
 * - Loading and caching gear items from remote or local sources
 * - Filtering and searching gear items
 * - Hierarchical display of gear items by category and type
 * - Drag and drop support for gear items
 * - User interface components for browsing and managing gear
 *
 * The implementation includes both a traditional list view and a modern
 * tree view for hierarchical organization of gear items.
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#include "GearLibrary.h"

/**
 * @brief Adapter class for ListBoxModel to support legacy list view.
 *
 * This class adapts the GearLibrary's list view functionality to work with
 * JUCE's ListBox component. It forwards all list box operations to the
 * owning GearLibrary instance.
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
     * @brief Gets the number of rows in the list box.
     *
     * @return The number of gear items currently displayed
     */
    int getNumRows() override { return owner.getNumRows(); }

    /**
     * @brief Paints a single row in the list box.
     *
     * @param rowNumber The index of the row to paint
     * @param g The graphics context to paint with
     * @param width The width of the row
     * @param height The height of the row
     * @param rowIsSelected Whether the row is currently selected
     */
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        owner.paintListBoxItem(rowNumber, g, width, height, rowIsSelected);
    }

    /**
     * @brief Creates or updates a component for a list box row.
     *
     * @param rowNumber The index of the row
     * @param isRowSelected Whether the row is selected
     * @param existingComponentToUpdate Existing component to update, if any
     * @return A component for the row
     */
    juce::Component *refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate) override
    {
        return owner.refreshComponentForRow(rowNumber, isRowSelected, existingComponentToUpdate);
    }

    /**
     * @brief Handles single-click events on list box items.
     *
     * @param row The index of the clicked row
     * @param e The mouse event details
     */
    void listBoxItemClicked(int row, const juce::MouseEvent &e) override
    {
        owner.listBoxItemClicked(row, e);
    }

    /**
     * @brief Handles double-click events on list box items.
     *
     * @param row The index of the clicked row
     * @param e The mouse event details
     */
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent &e) override
    {
        owner.listBoxItemDoubleClicked(row, e);
    }

private:
    GearLibrary &owner; ///< Reference to the owning GearLibrary instance
};

// DraggableListBox is now defined in DraggableListBox.h

/**
 * @brief Constructs a new GearLibrary instance.
 *
 * Initializes the library's UI components and data structures:
 * - Sets up the title label with appropriate styling
 * - Configures the search box with placeholder text and callback
 * - Creates and configures the refresh button with a Unicode arrow icon
 * - Creates and configures the add user gear button with a plus icon
 * - Initializes both the legacy list box and modern tree view
 * - Starts loading the library data asynchronously
 */
GearLibrary::GearLibrary()
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

    // Set up add user gear button with plus icon
    addUserGearButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::darkgrey);
    addUserGearButton.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::darkgrey.brighter(0.2f));
    addUserGearButton.addListener(this);
    addUserGearButton.setTooltip("Add Custom Gear");

    // Create plus icon using Unicode character
    auto normalPlusIcon = std::make_unique<juce::DrawableText>();
    normalPlusIcon->setText("+"); // Unicode Plus sign
    normalPlusIcon->setFont(juce::Font(24.0f, juce::Font::plain), true);
    normalPlusIcon->setColour(juce::Colours::white);
    normalPlusIcon->setJustification(juce::Justification::centred);

    // Use the same icon for normal and hover states
    addUserGearButton.setImages(normalPlusIcon.get(), nullptr, nullptr, nullptr,
                                nullptr, nullptr, nullptr, nullptr);
    addAndMakeVisible(addUserGearButton);

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

    rootItem = std::make_unique<GearTreeItem>(GearTreeItem::ItemType::Root, "Root", this);
    gearTreeView->setRootItem(rootItem.get());

    addAndMakeVisible(gearTreeView.get());

    // Load initial data
    loadLibraryAsync();
}

/**
 * @brief Destructor for GearLibrary.
 *
 * Ensures proper cleanup of resources:
 * - Sets the tree view's root item to null before deletion
 * - Allows the unique_ptr members to clean up automatically
 */
GearLibrary::~GearLibrary()
{
    // Important: set root item to null before the TreeView is deleted
    gearTreeView->setRootItem(nullptr);
}

/**
 * @brief Paints the library component.
 *
 * Fills the background with a dark grey color to provide contrast
 * with the gear items and controls.
 *
 * @param g The graphics context to paint with
 */
void GearLibrary::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
}

/**
 * @brief Handles component resizing.
 *
 * Updates the layout of all child components when the library
 * component is resized:
 * - Positions the title label at the top
 * - Arranges the control area (search box and buttons)
 * - Updates the bounds of either the list box or tree view
 *
 * @param bounds The new bounds of the component
 */
void GearLibrary::resized()
{
    auto bounds = getLocalBounds();

    // Title area
    titleLabel.setBounds(bounds.removeFromTop(30));

    // Control area
    auto controlArea = bounds.removeFromTop(40);
    refreshButton.setBounds(controlArea.removeFromRight(80).reduced(5));
    addUserGearButton.setBounds(controlArea.removeFromRight(120).reduced(5));
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
 * @brief Checks if an item should be shown based on current filters.
 *
 * Evaluates an item against:
 * - Current search text (matches against name, manufacturer, category, and tags)
 * - Current filter category (type or category)
 * - Current filter value
 *
 * @param item The gear item to check
 * @return true if the item should be displayed
 */
bool GearLibrary::shouldShowItem(const GearItem &item) const
{
    // First check the search text
    if (!currentSearchText.isEmpty())
    {
        bool matchesSearch = false;

        // Check name and manufacturer
        if (item.name.containsIgnoreCase(currentSearchText) ||
            item.manufacturer.containsIgnoreCase(currentSearchText))
        {
            matchesSearch = true;
        }

        // Check category string
        if (!matchesSearch && !item.categoryString.isEmpty() &&
            item.categoryString.containsIgnoreCase(currentSearchText))
        {
            matchesSearch = true;
        }

        // Check tags
        if (!matchesSearch && item.tags.size() > 0)
        {
            for (const auto &tag : item.tags)
            {
                if (tag.containsIgnoreCase(currentSearchText))
                {
                    matchesSearch = true;
                    break;
                }
            }
        }

        if (!matchesSearch)
            return false;
    }

    // Then check the filter
    if (currentFilter.first == FilterCategory::All)
        return true;

    if (currentFilter.first == FilterCategory::Type)
    {
        if (currentFilter.second == "500Series")
            return item.type == GearType::Series500;
        if (currentFilter.second == "Rack19Inch")
            return item.type == GearType::Rack19Inch;
        if (currentFilter.second == "UserCreated")
            return item.type == GearType::UserCreated;
    }
    else if (currentFilter.first == FilterCategory::Category)
    {
        // Check both the enum category and the string category for flexibility
        if (currentFilter.second == "EQ" || currentFilter.second == "equalizer")
            return (item.category == GearCategory::EQ ||
                    item.categoryString.equalsIgnoreCase("equalizer") ||
                    item.categoryString.equalsIgnoreCase("eq"));

        if (currentFilter.second == "Preamp" || currentFilter.second == "preamp")
            return (item.category == GearCategory::Preamp ||
                    item.categoryString.equalsIgnoreCase("preamp"));

        if (currentFilter.second == "Compressor" || currentFilter.second == "compressor")
            return (item.category == GearCategory::Compressor ||
                    item.categoryString.equalsIgnoreCase("compressor"));

        if (currentFilter.second == "Other" || currentFilter.second == "other")
            return (item.category == GearCategory::Other ||
                    item.categoryString.equalsIgnoreCase("other"));
    }

    return true;
}

/**
 * @brief Updates the filtered items based on current filters.
 *
 * Updates both the legacy list box and tree view to reflect
 * the current search and filter state:
 * - Updates list box content and repaints
 * - Refreshes tree view items
 * - Expands or collapses nodes based on search results
 * - Shows or hides items based on filter matches
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

        // If we're searching or the root item has no children yet, refresh from scratch
        if (isSearching || rootItem->getNumSubItems() == 0)
        {
            rootItem->clearSubItems();
            rootItem->refreshSubItems();
        }

        // Recursively expand and hide/show nodes based on search results
        if (isSearching)
        {
            // First see if any item in the entire tree matches the search
            bool anyMatches = false;
            for (int i = 0; i < gearItems.size(); ++i)
            {
                if (shouldShowItem(gearItems.getReference(i)))
                {
                    anyMatches = true;
                    break;
                }
            }

            // If we have matches, expand everything to show them
            if (anyMatches)
            {
                // Make sure the root categories node is expanded
                for (int i = 0; i < rootItem->getNumSubItems(); ++i)
                {
                    if (auto categoriesNode = dynamic_cast<GearTreeItem *>(rootItem->getSubItem(i)))
                    {
                        categoriesNode->setOpen(true);

                        // For each category, check if it contains matching items
                        for (int j = 0; j < categoriesNode->getNumSubItems(); ++j)
                        {
                            if (auto categoryNode = dynamic_cast<GearTreeItem *>(categoriesNode->getSubItem(j)))
                            {
                                bool categoryHasMatches = false;

                                // If the node hasn't loaded its items yet, refresh it
                                if (categoryNode->getNumSubItems() == 0)
                                {
                                    categoryNode->refreshSubItems();
                                }

                                // Check each gear item to see if it matches
                                for (int k = 0; k < categoryNode->getNumSubItems(); ++k)
                                {
                                    if (auto gearNode = dynamic_cast<GearTreeItem *>(categoryNode->getSubItem(k)))
                                    {
                                        if (GearItem *gearItem = gearNode->getGearItem())
                                        {
                                            // Check if this item matches the search
                                            bool matches = shouldShowItem(*gearItem);
                                            gearNode->setVisible(matches);

                                            if (matches)
                                                categoryHasMatches = true;
                                        }
                                    }
                                }

                                // Only show and expand categories that have matching items
                                categoryNode->setVisible(categoryHasMatches);
                                categoryNode->setOpen(categoryHasMatches);
                            }
                        }
                    }
                }
            }
            else
            {
                // If no matches, don't expand anything
                rootItem->setOpenness(false);
            }
        }
        else
        {
            // When not searching, make all nodes visible
            for (int i = 0; i < rootItem->getNumSubItems(); ++i)
            {
                if (auto categoriesNode = dynamic_cast<GearTreeItem *>(rootItem->getSubItem(i)))
                {
                    categoriesNode->setVisible(true);

                    for (int j = 0; j < categoriesNode->getNumSubItems(); ++j)
                    {
                        if (auto categoryNode = dynamic_cast<GearTreeItem *>(categoriesNode->getSubItem(j)))
                        {
                            categoryNode->setVisible(true);

                            for (int k = 0; k < categoryNode->getNumSubItems(); ++k)
                            {
                                if (auto gearNode = dynamic_cast<GearTreeItem *>(categoryNode->getSubItem(k)))
                                {
                                    gearNode->setVisible(true);
                                }
                            }
                        }
                    }
                }
            }
        }

        gearTreeView->repaint();
    }
}

/**
 * @brief Gets the number of rows in the list box.
 *
 * @return The number of gear items currently in the library
 */
int GearLibrary::getNumRows()
{
    return gearItems.size();
}

/**
 * @brief Paints a single row in the list box.
 *
 * Renders a gear item in the list box with:
 * - Background color based on selection state
 * - Item name in white with larger font
 * - Manufacturer name in light grey with smaller font
 *
 * @param rowNumber The index of the row to paint
 * @param g The graphics context to paint with
 * @param width The width of the row
 * @param height The height of the row
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
 * @brief Creates or updates a component for a list box row.
 *
 * Currently not used as we're using paintListBoxItem for rendering.
 * Returns the existing component unchanged.
 *
 * @param rowNumber The index of the row
 * @param isRowSelected Whether the row is selected
 * @param existingComponentToUpdate Existing component to update, if any
 * @return The existing component, unchanged
 */
juce::Component *GearLibrary::refreshComponentForRow(int /*rowNumber*/, bool /*isRowSelected*/, juce::Component *existingComponentToUpdate)
{
    // We're using paintListBoxItem for now, so return nullptr
    return existingComponentToUpdate;
}

/**
 * @brief Handles single-click events on list box items.
 *
 * Selects the clicked row in the list box if it's visible.
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
 * @brief Handles double-click events on list box items.
 *
 * Currently not used as we're using the tree view for interaction.
 *
 * @param row The index of the clicked row
 * @param e The mouse event details
 */
void GearLibrary::listBoxItemDoubleClicked(int row, const juce::MouseEvent & /*e*/)
{
    // Not used with tree view
}

/**
 * @brief Loads the gear library asynchronously.
 *
 * Initiates the loading of both filter options and gear items
 * in parallel using background threads.
 */
void GearLibrary::loadLibraryAsync()
{
    // Start both async loading operations
    loadFiltersAsync();
    loadGearItemsAsync();
}

/**
 * @brief Loads filter options asynchronously.
 *
 * Fetches filter options from a remote source or uses fallback data
 * if the connection fails. Updates the UI on the message thread
 * when complete.
 */
void GearLibrary::loadFiltersAsync()
{
    // Use a background thread to avoid blocking the message thread
    juce::Thread::launch([this]()
                         {
        // Simulate network delay
        juce::Thread::sleep(500); 
        
        // Create a copy of the JSON data to avoid any threading issues
        juce::String filterJson = R"({
            "filters": [
                {"displayName": "500 Series", "category": "Type", "value": "500Series"},
                {"displayName": "19\" Rack", "category": "Type", "value": "Rack19Inch"},
                {"displayName": "User Created", "category": "Type", "value": "UserCreated"},
                {"displayName": "EQ", "category": "Category", "value": "EQ"},
                {"displayName": "Preamp", "category": "Category", "value": "Preamp"},
                {"displayName": "Compressor", "category": "Category", "value": "Compressor"},
                {"displayName": "Other", "category": "Category", "value": "Other"}
            ]
        })";
        
        // Send the result back to the message thread
        juce::MessageManager::callAsync([this, filterJson]() {
            parseFilterOptions(filterJson);
        }); });
}

/**
 * @brief Loads gear items asynchronously.
 *
 * Fetches gear items from a remote source or uses fallback data
 * if the connection fails. Updates the UI and saves to local cache
 * on the message thread when complete.
 */
void GearLibrary::loadGearItemsAsync()
{
    // Use a background thread to avoid blocking the message thread
    juce::Thread::launch([this]()
                         {
        // Create URL for the remote endpoint using the helper method
        juce::URL url(getFullUrl(RemoteResources::LIBRARY_PATH));
        
        // Fetch the JSON data
        auto urlStream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                   .withConnectionTimeoutMs(10000)
                                                   .withNumRedirectsToFollow(5));
        
        juce::String jsonData;
        bool loadedSuccessfully = false;
        
        if (urlStream != nullptr)
        {
            // Read the data from the stream
            jsonData = urlStream->readEntireStreamAsString();
            loadedSuccessfully = true;
            DBG("Successfully fetched gear library from remote URL");
        }
        else
        {
            // Failed to connect, use fallback data
            DBG("Failed to connect to remote URL, using fallback data");
            jsonData = R"({
                "units": [
                    {
                        "unitId": "la2a-compressor",
                        "name": "LA-2A Compressor",
                        "manufacturer": "Teletronix/Universal Audio",
                        "category": "compressor",
                        "version": "1.0.0",
                        "schemaPath": "units/la2a-compressor-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                        "tags": [
                            "UA", "Universal Audio", "Teletronix", "optical", "tube", "vintage", "leveling amplifier"
                        ]
                    },
                    {
                        "unitId": "api-560-eq",
                        "name": "API 560 10-Band Graphic Equalizer",
                        "manufacturer": "Automated Processes Inc.",
                        "category": "equalizer",
                        "version": "1.0.0",
                        "schemaPath": "units/api-560-eq-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/api-560-eq-1.0.0.jpg",
                        "tags": [
                            "API", "500 series", "graphic EQ", "10-band", "hardware"
                        ]
                    }
                ]
            })";
        }
        
        // Send the result back to the message thread
        juce::MessageManager::callAsync([this, jsonData, loadedSuccessfully]() {
            // Clear existing items before adding new ones
            gearItems.clear();
            
            // Parse the JSON data into gear items
            parseGearLibrary(jsonData);
            
            // Update the UI to reflect the changes
            updateFilteredItems();
            
            // If loaded successfully, save the library to local cache
            if (loadedSuccessfully)
                saveLibraryAsync();
        }); });
}

/**
 * @brief Parses the gear library data from JSON.
 *
 * Processes JSON data in either the new "units" format or the legacy
 * "gear" format. For each item:
 * - Extracts basic properties (ID, name, manufacturer, etc.)
 * - Processes tags and categories
 * - Handles schema and thumbnail image paths
 * - Creates GearItem instances with appropriate defaults
 * - Attempts to load thumbnail images
 *
 * @param jsonData The JSON string containing the library data
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
                              thumbnailImage, tags, GearType::Other, GearCategory::Other,
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
                GearItem item(name, manufacturer, type, category, slotSize, thumbnailUrl, controls);

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
 * @brief Gets a gear item by its index.
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
 * Currently not used as we're using the tree view for interaction.
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
 * Currently not used as we're using the tree view for interaction.
 *
 * @param e The mouse event details
 */
void GearLibrary::mouseDrag(const juce::MouseEvent &e)
{
    // For now, we're not forwarding mouse events to the list box
    // since we're using the tree view
}

/**
 * @brief Adds a new user-created gear item to the library.
 *
 * Creates a new gear item with:
 * - Specified name, category, description, and manufacturer
 * - Appropriate gear type based on name (500 series or rack mount)
 * - Default slot size of 1
 * - Empty controls array
 *
 * Updates the UI to reflect the new item.
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
    gearItems.add(GearItem(name, manufacturer, gearType, gearCategory, 1, "", controls));

    // Update the UI
    if (rootItem != nullptr)
        rootItem->refreshSubItems();
}

/**
 * @brief Saves the gear library asynchronously.
 *
 * Simulates saving the library to local storage using a background thread.
 * In a real implementation, this would write to a file or server.
 */
void GearLibrary::saveLibraryAsync()
{
    // In a real implementation, this would save to a file or server
    // We'll use a background thread to simulate the save operation
    juce::Thread::launch([this]()
                         {
        // Simulate saving to a file or server
        juce::Thread::sleep(500);
        
        // Log the save operation completion
        DBG("Library saved successfully"); });
}

/**
 * @brief Handles button click events.
 *
 * Processes clicks on the refresh and add user gear buttons:
 * - Refresh button: Reloads the library data
 * - Add user gear button: Opens the user gear creation dialog (to be implemented)
 *
 * @param button The button that was clicked
 */
void GearLibrary::buttonClicked(juce::Button *button)
{
    if (button == &refreshButton)
    {
        loadLibraryAsync();
    }
    else if (button == &addUserGearButton)
    {
        // Open user gear creation dialog (to be implemented)
    }
}

/**
 * @brief Parses filter options from JSON.
 *
 * Processes JSON data containing filter options:
 * - Adds an "All Items" option as the first filter
 * - Extracts display names, categories, and values
 * - Stores options for use in filtering
 *
 * @param jsonData The JSON string containing filter options
 */
void GearLibrary::parseFilterOptions(const juce::String &jsonData)
{
    // This method is retained for backwards compatibility but simplified
    // since we no longer use the filter box
    auto json = juce::JSON::parse(jsonData);

    if (json.hasProperty("filters") && json["filters"].isArray())
    {
        auto filterArray = json["filters"].getArray();
        filterOptions.clear();

        // Always add "All Items" as the first option
        filterOptions.add({"All Items", FilterCategory::All, ""});

        for (auto &filterJson : *filterArray)
        {
            if (filterJson.isObject())
            {
                auto obj = filterJson.getDynamicObject();

                juce::String displayName = obj->getProperty("displayName");
                juce::String category = obj->getProperty("category");
                juce::String value = obj->getProperty("value");

                FilterCategory filterCategory;
                if (category == "Type")
                    filterCategory = FilterCategory::Type;
                else if (category == "Category")
                    filterCategory = FilterCategory::Category;
                else
                    continue; // Skip invalid categories

                filterOptions.add({displayName, filterCategory, value});
            }
        }
    }
}

void GearLibrary::updateFilterBox()
{
    // This method is now a no-op since we removed the filter box
    // but we keep it for API compatibility
    currentFilter = {FilterCategory::All, ""};
    updateFilteredItems();
}