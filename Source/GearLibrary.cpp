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
    : gearListModel(new GearListBoxModel(*this)),
      gearListBox(new DraggableListBox("Gear Library", gearListModel.get()))
{
    // Set component ID for debugging
    setComponentID("GearLibrary");
    gearListBox->setComponentID("GearListBox");
    
    // Set up title label
    titleLabel.setText("Gear Library", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // CRITICAL: Make the parent GearLibrary component forward all mouse events to the list box
    // This is often the root cause of drag and drop not working
    setInterceptsMouseClicks(false, true);
    
    // Set up search box
    searchBox.setTextToShowWhenEmpty("Search gear...", juce::Colours::grey);
    searchBox.onTextChange = [this] { 
        currentSearchText = searchBox.getText();
        updateFilteredItems();
    };
    addAndMakeVisible(searchBox);

    // Set up filter box
    filterBox.clear();
    filterBox.onChange = [this] {
        int selectedId = filterBox.getSelectedId();
        DBG("Filter box selection changed: ID=" + juce::String(selectedId));
        
        if (selectedId > 0 && selectedId <= filterOptions.size())
        {
            // Adjust index (0-based array, 1-based combobox IDs)
            const auto& option = filterOptions[selectedId - 1];
            currentFilter = {option.category, option.value};
            DBG("Setting filter to: category=" + 
                (option.category == FilterCategory::All ? "All" : 
                 option.category == FilterCategory::Type ? "Type" : "Category") + 
                ", value=" + option.value);
            updateFilteredItems();
        }
    };
    addAndMakeVisible(filterBox);

    // Set up refresh button
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { loadLibraryAsync(); };
    addAndMakeVisible(refreshButton);

    // Set up add user gear button
    addUserGearButton.setButtonText("Add Custom Gear");
    addUserGearButton.onClick = [] { 
        // Open user gear creation dialog (to be implemented)
    };
    addAndMakeVisible(addUserGearButton);

    // Set up list box
    gearListBox->setRowHeight(60);
    gearListBox->setMultipleSelectionEnabled(false);
    addAndMakeVisible(*gearListBox);

    // Load initial filter options
    loadLibraryAsync();
}

GearLibrary::~GearLibrary()
{
}

void GearLibrary::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
}

void GearLibrary::resized()
{
    auto area = getLocalBounds();
    
    // Title area
    auto titleArea = area.removeFromTop(30);
    titleArea.removeFromLeft(10); // Add some padding
    titleLabel.setBounds(titleArea);
    
    // Search and filter controls
    auto topControls = area.removeFromTop(30);
    searchBox.setBounds(topControls.removeFromLeft(static_cast<int>(topControls.getWidth() * 0.7f)).reduced(2));
    filterBox.setBounds(topControls.reduced(2));
    
    // Bottom buttons - centered
    auto bottomButtons = area.removeFromBottom(30);
    int totalButtonWidth = 150 + 80 + 4; // Add Custom Gear + Refresh + spacing
    int startX = (bottomButtons.getWidth() - totalButtonWidth) / 2;
    
    bottomButtons.removeFromLeft(startX); // Remove space from left to center
    addUserGearButton.setBounds(bottomButtons.removeFromLeft(150).reduced(2));
    refreshButton.setBounds(bottomButtons.removeFromLeft(80).reduced(2));
    
    // List box gets remaining space
    gearListBox->setBounds(area.reduced(0, 5));
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
    // Log current filter state for debugging
    DBG("Updating filtered items:");
    DBG(" - Current filter: " + 
        (currentFilter.first == FilterCategory::All ? "All" : 
         currentFilter.first == FilterCategory::Type ? "Type" : "Category") + 
        ", value: " + currentFilter.second);
    DBG(" - Search text: " + currentSearchText);
    
    // Update count of visible items for debugging
    int visibleCount = 0;
    for (const auto& item : gearItems)
    {
        if (shouldShowItem(item))
            visibleCount++;
    }
    DBG(" - Visible items: " + juce::String(visibleCount) + " / " + juce::String(gearItems.size()));
    
    // Refresh the list box
    gearListBox->updateContent();
    gearListBox->repaint();
}

int GearLibrary::getNumRows()
{
    int count = 0;
    for (const auto& item : gearItems)
    {
        if (shouldShowItem(item))
        {
            count++;
        }
    }
    return count;
}

void GearLibrary::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int /*height*/, bool rowIsSelected)
{
    // Find the actual item index that corresponds to this filtered row
    int actualIndex = -1;
    int filteredCount = 0;
    for (int i = 0; i < gearItems.size(); ++i)
    {
        if (shouldShowItem(gearItems[i]))
        {
            if (filteredCount == rowNumber)
            {
                actualIndex = i;
                break;
            }
            filteredCount++;
        }
    }

    if (actualIndex >= 0 && actualIndex < gearItems.size())
    {
        const auto& item = gearItems[actualIndex];
        
        // Background
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue.darker(0.2f));
        else
            g.fillAll(juce::Colours::darkgrey);
        
        // Debug outline to clearly see the bounds
        g.setColour(juce::Colours::green);
        g.drawRect(0, 0, width, 60, 1); // Draw outline around item
        
        // Text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText(item.name, 10, 5, width - 20, 20, juce::Justification::left);
        
        g.setFont(14.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(item.manufacturer, 10, 25, width - 20, 16, juce::Justification::left);
        
        // Type/category info
        juce::String typeStr;
        if (item.type == GearType::Series500)
            typeStr = "500 Series";
        else if (item.type == GearType::Rack19Inch)
            typeStr = "19\" Rack";
        else
            typeStr = "Custom";
            
        juce::String catStr;
        if (item.category == GearCategory::EQ)
            catStr = "EQ";
        else if (item.category == GearCategory::Compressor)
            catStr = "Compressor";
        else if (item.category == GearCategory::Preamp)
            catStr = "Preamp";
        else
            catStr = "Other";
            
        g.drawText(typeStr + " | " + catStr + " | Drag Me!", 10, 41, width - 20, 14, juce::Justification::left);
    }
}

juce::Component* GearLibrary::refreshComponentForRow(int /*rowNumber*/, bool /*isRowSelected*/, juce::Component* /*existingComponentToUpdate*/)
{
    // We're using paintListBoxItem for now, so return nullptr
    return nullptr;
}

void GearLibrary::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    // Select the row
    gearListBox->selectRow(row);
}

void GearLibrary::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    // Show details or edit the gear item
    if (row >= 0 && row < gearItems.size())
    {
        // In the future, show a details dialog
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                           "Gear Details",
                                           "Item: " + gearItems[row].name);
    }
}

void GearLibrary::loadLibraryAsync()
{
    // Start both async loading operations
    loadFiltersAsync();
    loadGearItemsAsync();
}

void GearLibrary::loadFiltersAsync()
{
    // Simulate async loading of filters
    juce::Thread::sleep(500); // Simulate network delay
    
    juce::MessageManager::callAsync([this]() {
        // Simulate fetching filter options from remote
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
        
        parseFilterOptions(filterJson);
    });
}

void GearLibrary::loadGearItemsAsync()
{
    // Simulate async loading of gear items
    juce::Thread::sleep(1000); // Simulate network delay
    
    juce::MessageManager::callAsync([this]() {
        // Clear existing items before adding new ones
        gearItems.clear();
        
        // Simulate fetching gear items from remote as JSON
        juce::String gearJson = R"({
            "gear": [
                {
                    "name": "API 550A",
                    "manufacturer": "API",
                    "type": "500Series",
                    "category": "EQ",
                    "slotSize": 1,
                    "imageUrl": ""
                },
                {
                    "name": "Trident 80B",
                    "manufacturer": "Trident",
                    "type": "Rack19Inch",
                    "category": "EQ",
                    "slotSize": 1,
                    "imageUrl": ""
                },
                {
                    "name": "DBX 560A",
                    "manufacturer": "DBX",
                    "type": "500Series",
                    "category": "Compressor",
                    "slotSize": 1,
                    "imageUrl": ""
                },
                {
                    "name": "Neve 1073",
                    "manufacturer": "Neve",
                    "type": "Rack19Inch",
                    "category": "Preamp",
                    "slotSize": 2,
                    "imageUrl": ""
                },
                {
                    "name": "SSL G Series",
                    "manufacturer": "SSL",
                    "type": "Rack19Inch",
                    "category": "Compressor",
                    "slotSize": 1,
                    "imageUrl": ""
                },
                {
                    "name": "Pultec EQP-1A",
                    "manufacturer": "Pultec",
                    "type": "Rack19Inch",
                    "category": "EQ",
                    "slotSize": 2,
                    "imageUrl": ""
                }
            ]
        })";
        
        // Parse the JSON data into gear items
        parseGearLibrary(gearJson);
        
        // Save the updated library
        saveLibraryAsync();
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
                juce::String imageUrl = obj->getProperty("imageUrl");
                
                // Add to list
                gearItems.add(GearItem(name, manufacturer, type, category, slotSize, imageUrl, {}));
            }
        }
        
        // Update list box
        gearListBox->updateContent();
        gearListBox->repaint();
    }
}

GearItem* GearLibrary::getGearItem(int index)
{
    // Find the actual item index that corresponds to this filtered row
    int actualIndex = -1;
    int filteredCount = 0;
    for (int i = 0; i < gearItems.size(); ++i)
    {
        if (shouldShowItem(gearItems[i]))
        {
            if (filteredCount == index)
            {
                actualIndex = i;
                break;
            }
            filteredCount++;
        }
    }

    if (actualIndex >= 0 && actualIndex < gearItems.size())
    {
        return &(gearItems.getReference(actualIndex));
    }
    return nullptr;
}

void GearLibrary::mouseDown(const juce::MouseEvent& e)
{
    // Forward mouse down events to the list box
    DBG("GearLibrary: mouseDown event - forwarding to list box");
    
    // Convert to list box coordinates
    juce::Point<int> pointInListBox = e.position.toInt() - gearListBox->getPosition();
    juce::MouseEvent listBoxEvent = e.withNewPosition(pointInListBox.toFloat());
    
    // Forward to list box
    gearListBox->mouseDown(listBoxEvent);
}

void GearLibrary::mouseDrag(const juce::MouseEvent& e)
{
    // Forward mouse drag events to the list box
    DBG("GearLibrary: mouseDrag event - forwarding to list box");
    
    // Convert to list box coordinates
    juce::Point<int> pointInListBox = e.position.toInt() - gearListBox->getPosition();
    juce::MouseEvent listBoxEvent = e.withNewPosition(pointInListBox.toFloat());
    
    // Forward to list box
    gearListBox->mouseDrag(listBoxEvent);
}

void GearLibrary::addItem(const juce::String& name, const juce::String& category, const juce::String& description, const juce::String& manufacturer)
{
    GearItem item;
    item.name = name;
    item.manufacturer = manufacturer;
    
    // Properly set the category based on the string value
    if (category == "EQ")
        item.category = GearCategory::EQ;
    else if (category == "Preamp")
        item.category = GearCategory::Preamp;
    else if (category == "Compressor")
        item.category = GearCategory::Compressor;
    else
        item.category = GearCategory::Other;
    
    // Determine a reasonable default type based on common industry standards
    if (name.containsIgnoreCase("500") || name.containsIgnoreCase("lunchbox"))
        item.type = GearType::Series500;
    else
        item.type = GearType::Rack19Inch;
    
    item.slotSize = 1;  // Default slot size
    
    gearItems.add(item);
    gearListBox->updateContent();
}

void GearLibrary::saveLibraryAsync()
{
    // In a real implementation, this would save to a file or server
    // For now, we'll just simulate it
    juce::Thread::sleep(500);
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
        
        // After loading filter options, update the filter box and reset to default
        updateFilterBox();
        
        // Log filter options for debugging
        DBG("Loaded " + juce::String(filterOptions.size()) + " filter options:");
        for (int i = 0; i < filterOptions.size(); ++i)
        {
            DBG(" - " + filterOptions[i].displayName + " (category: " + 
                (filterOptions[i].category == FilterCategory::All ? "All" : 
                 filterOptions[i].category == FilterCategory::Type ? "Type" : "Category") + 
                ", value: " + filterOptions[i].value + ")");
        }
    }
}

void GearLibrary::updateFilterBox()
{
    // Clear the box first
    filterBox.clear();
    
    // Add all available filter options
    for (int i = 0; i < filterOptions.size(); ++i)
    {
        filterBox.addItem(filterOptions[i].displayName, i + 1);
    }
    
    // Reset to "All Items" and trigger filter update
    filterBox.setSelectedId(1);
    currentFilter = {FilterCategory::All, ""};
    updateFilteredItems();
} 