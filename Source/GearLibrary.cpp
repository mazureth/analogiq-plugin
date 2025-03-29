#include "GearLibrary.h"

// Create a ListBoxModel adapter
class GearListBoxModel : public juce::ListBoxModel
{
public:
    GearListBoxModel(GearLibrary& ownerRef) : owner(ownerRef) {}
    
    int getNumRows() override { return owner.getNumRows(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        owner.paintListBoxItem(rowNumber, g, width, height, rowIsSelected);
    }
    
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override
    {
        return owner.refreshComponentForRow(rowNumber, isRowSelected, existingComponentToUpdate);
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override
    {
        owner.listBoxItemClicked(row, e);
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override
    {
        owner.listBoxItemDoubleClicked(row, e);
    }
    
private:
    GearLibrary& owner;
};

// DraggableListBox is now defined in DraggableListBox.h

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
    searchBox.onTextChange = [this] {
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
    juce::String unicodeArrow (CharPointer_UTF8 ("\xE2\x86\xBB"));
    normalIcon->setText(unicodeArrow);  // Unicode U+27F2 (ANTICLOCKWISE GAPPED CIRCLE ARROW)
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
    normalPlusIcon->setText("+");  // Unicode Plus sign
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

GearLibrary::~GearLibrary()
{
    // Important: set root item to null before the TreeView is deleted
    gearTreeView->setRootItem(nullptr);
}

void GearLibrary::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
}

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

bool GearLibrary::shouldShowItem(const GearItem& item) const
{
    // First check the search text
    if (!currentSearchText.isEmpty())
    {
        if (!item.name.containsIgnoreCase(currentSearchText) &&
            !item.manufacturer.containsIgnoreCase(currentSearchText))
        {
            return false;
        }
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
        if (currentFilter.second == "EQ")
            return item.category == GearCategory::EQ;
        if (currentFilter.second == "Preamp")
            return item.category == GearCategory::Preamp;
        if (currentFilter.second == "Compressor")
            return item.category == GearCategory::Compressor;
        if (currentFilter.second == "Other")
            return item.category == GearCategory::Other;
    }

    return true;
}

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
        // Only refresh the root item's children if they exist
        if (rootItem->getNumSubItems() > 0)
        {
            // Check if we're searching
            bool shouldExpand = !currentSearchText.isEmpty();
                               
            // Remember current expanded state of items
            juce::Array<juce::String> expandedItems;
            for (int i = 0; i < rootItem->getNumSubItems(); ++i)
            {
                if (auto item = dynamic_cast<GearTreeItem*>(rootItem->getSubItem(i)))
                {
                    if (item->isOpen())
                        expandedItems.add(item->getUniqueName());
                        
                    // Also check second-level items
                    for (int j = 0; j < item->getNumSubItems(); ++j)
                    {
                        if (auto subItem = dynamic_cast<GearTreeItem*>(item->getSubItem(j)))
                        {
                            if (subItem->isOpen())
                                expandedItems.add(subItem->getUniqueName());
                        }
                    }
                }
            }
            
            // Refresh all items
            rootItem->clearSubItems();
            rootItem->refreshSubItems();
            
            // Restore expanded state or expand based on search
            for (int i = 0; i < rootItem->getNumSubItems(); ++i)
            {
                if (auto item = dynamic_cast<GearTreeItem*>(rootItem->getSubItem(i)))
                {
                    if (shouldExpand || expandedItems.contains(item->getUniqueName()))
                    {
                        item->setOpen(true);
                        
                        // Also expand relevant second-level items
                        for (int j = 0; j < item->getNumSubItems(); ++j)
                        {
                            if (auto subItem = dynamic_cast<GearTreeItem*>(item->getSubItem(j)))
                            {
                                bool matchesSearch = false;
                                
                                // Check for search text matches
                                if (!currentSearchText.isEmpty() && 
                                    subItem->getItemText().toLowerCase().contains(currentSearchText))
                                {
                                    matchesSearch = true;
                                }
                                
                                if (shouldExpand || matchesSearch || 
                                    expandedItems.contains(subItem->getUniqueName()))
                                {
                                    subItem->setOpen(true);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            rootItem->refreshSubItems();
        }
        
        gearTreeView->repaint();
    }
}

int GearLibrary::getNumRows()
{
    return gearItems.size();
}

void GearLibrary::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int /*height*/, bool rowIsSelected)
{
    // This is still needed for legacy support or components that might use the list box
    if (rowNumber >= 0 && rowNumber < gearItems.size())
    {
        const auto& item = gearItems[rowNumber];
        
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

juce::Component* GearLibrary::refreshComponentForRow(int /*rowNumber*/, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate)
{
    // We're using paintListBoxItem for now, so return nullptr
    return existingComponentToUpdate;
}

void GearLibrary::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    // Select the row
    if (gearListBox->isVisible())
        gearListBox->selectRow(row);
}

void GearLibrary::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    // Not used with tree view
}

void GearLibrary::loadLibraryAsync()
{
    // Start both async loading operations
    loadFiltersAsync();
    loadGearItemsAsync();
}

void GearLibrary::loadFiltersAsync()
{
    // Use a background thread to avoid blocking the message thread
    juce::Thread::launch([this]() {
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
        });
    });
}

void GearLibrary::loadGearItemsAsync()
{
    // Use a background thread to avoid blocking the message thread
    juce::Thread::launch([this]() {
        // Simulate network delay
        juce::Thread::sleep(1000);
        
        // Create a copy of the JSON data to avoid any threading issues
        juce::String gearJson = R"({
            "gear": [
                {
                    "name": "API 550A",
                    "manufacturer": "API",
                    "type": "500Series",
                    "category": "EQ",
                    "slotSize": 1,
                    "thumbnailUrl": "https://www.rspeaudio.com/media/iopt/catalog/product/cache/c92d27ddc910f51cbf84cfdc5a8e45c0/image/1394d61/api-550b-discrete-4-band-eq.webp"
                },
                {
                    "name": "Trident 80B",
                    "manufacturer": "Trident",
                    "type": "Rack19Inch",
                    "category": "EQ",
                    "slotSize": 1,
                    "thumbnailUrl": "https://tridentaudiodevelopments.com/wp-content/uploads/2016/06/80BEQ_front.jpg"
                },
                {
                    "name": "DBX 560A",
                    "manufacturer": "DBX",
                    "type": "500Series",
                    "category": "Compressor",
                    "slotSize": 1,
                    "thumbnailUrl": "https://vintageking.com/media/catalog/product/d/b/dbx560a-500s-vca-com_43992_1.jpg"
                },
                {
                    "name": "Neve 1073",
                    "manufacturer": "Neve",
                    "type": "Rack19Inch",
                    "category": "Preamp",
                    "slotSize": 2,
                    "thumbnailUrl": ""
                },
                {
                    "name": "SSL G Series",
                    "manufacturer": "SSL",
                    "type": "Rack19Inch",
                    "category": "Compressor",
                    "slotSize": 1,
                    "thumbnailUrl": ""
                },
                {
                    "name": "Pultec EQP-1A",
                    "manufacturer": "Pultec",
                    "type": "Rack19Inch",
                    "category": "EQ",
                    "slotSize": 2,
                    "thumbnailUrl": ""
                }
            ]
        })";
        
        // Send the result back to the message thread
        juce::MessageManager::callAsync([this, gearJson]() {
            // Clear existing items before adding new ones
            gearItems.clear();
            
            // Parse the JSON data into gear items
            parseGearLibrary(gearJson);
            
            // Save the updated library
            saveLibraryAsync();
        });
    });
}

void GearLibrary::parseGearLibrary(const juce::String& jsonData)
{
    // Parse JSON data and populate gear items
    auto json = juce::JSON::parse(jsonData);
    
    if (json.hasProperty("gear") && json["gear"].isArray())
    {
        auto gearArray = json["gear"].getArray();
        gearItems.clear();
        
        for (auto& gearJson : *gearArray)
        {
            if (gearJson.isObject())
            {
                auto obj = gearJson.getDynamicObject();
                
                // Extract properties
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
                
                // Create gear item
                GearItem item(name, manufacturer, type, category, slotSize, thumbnailUrl, controls);
                
                // If thumbnailUrl is provided, try to load the image
                if (thumbnailUrl.isNotEmpty())
                {
                    // In a real implementation, you'd load from network or disk
                    // For now, we'll setup a placeholder image
                    
                    // Create a placeholder colored image based on category
                    juce::Image thumbnail(juce::Image::ARGB, 24, 24, true);
                    juce::Graphics g(thumbnail);
                    
                    // Use different colors for different categories
                    switch (category)
                    {
                        case GearCategory::EQ:
                            g.setColour(juce::Colours::orange);
                            break;
                        case GearCategory::Preamp:
                            g.setColour(juce::Colours::red);
                            break;
                        case GearCategory::Compressor:
                            g.setColour(juce::Colours::blue);
                            break;
                        default:
                            g.setColour(juce::Colours::green);
                            break;
                    }
                    
                    // Draw a rounded rectangle for the thumbnail
                    g.fillRoundedRectangle(0.0f, 0.0f, 24.0f, 24.0f, 4.0f);
                    
                    // Draw the first letter of the gear name
                    g.setColour(juce::Colours::white);
                    g.setFont(16.0f);
                    g.drawText(name.substring(0, 1).toUpperCase(), 
                              0, 0, 24, 24, juce::Justification::centred);
                    
                    // Set the image in the item
                    item.image = thumbnail;
                }
                
                // Add to list
                gearItems.add(item);
            }
        }
        
        // Update the tree view
        if (rootItem != nullptr)
            rootItem->refreshSubItems();
    }
}

GearItem* GearLibrary::getGearItem(int index)
{
    if (index >= 0 && index < gearItems.size())
        return &(gearItems.getReference(index));
    
    return nullptr;
}

void GearLibrary::mouseDown(const juce::MouseEvent& e)
{
    // For now, we're not forwarding mouse events to the list box
    // since we're using the tree view
}

void GearLibrary::mouseDrag(const juce::MouseEvent& e)
{
    // For now, we're not forwarding mouse events to the list box
    // since we're using the tree view
}

void GearLibrary::addItem(const juce::String& name, const juce::String& category, const juce::String& description, const juce::String& manufacturer)
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

void GearLibrary::saveLibraryAsync()
{
    // In a real implementation, this would save to a file or server
    // We'll use a background thread to simulate the save operation
    juce::Thread::launch([this]() {
        // Simulate saving to a file or server
        juce::Thread::sleep(500);
        
        // Log the save operation completion
        DBG("Library saved successfully");
    });
}

void GearLibrary::buttonClicked(juce::Button* button)
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

void GearLibrary::parseFilterOptions(const juce::String& jsonData)
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
        
        for (auto& filterJson : *filterArray)
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