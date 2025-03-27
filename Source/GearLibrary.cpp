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
    
    // CRITICAL: Make the parent GearLibrary component forward all mouse events to the list box
    // This is often the root cause of drag and drop not working
    setInterceptsMouseClicks(false, true);
    
    // Set up search box
    searchBox.setTextToShowWhenEmpty("Search gear...", juce::Colours::grey);
    searchBox.onTextChange = [this] { 
        gearListBox->updateContent();
        gearListBox->repaint();
    };
    addAndMakeVisible(searchBox);

    // Set up filter box
    filterBox.addItem("All Types", 1);
    filterBox.addItem("500 Series", 2);
    filterBox.addItem("19\" Rack", 3);
    filterBox.addItem("User Created", 4);
    filterBox.setSelectedId(1);
    filterBox.onChange = [this] {
        gearListBox->updateContent();
        gearListBox->repaint();
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

    // Add some dummy gear items for testing
    gearItems.add(GearItem("API 550A", "API", GearType::Series500, GearCategory::EQ, 1, "", {}));
    gearItems.add(GearItem("Neve 1073", "Neve", GearType::Rack19Inch, GearCategory::Preamp, 2, "", {}));
    gearItems.add(GearItem("SSL G-Comp", "SSL", GearType::Rack19Inch, GearCategory::Compressor, 1, "", {}));
}

GearLibrary::~GearLibrary()
{
}

void GearLibrary::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.7f));
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Gear Library", getLocalBounds().removeFromTop(30).withTrimmedLeft(10),
               juce::Justification::centredLeft, true);
}

void GearLibrary::resized()
{
    auto area = getLocalBounds();
    
    // Top bar
    auto topBar = area.removeFromTop(30);
    addUserGearButton.setBounds(topBar.removeFromRight(150).reduced(2));
    refreshButton.setBounds(topBar.removeFromRight(80).reduced(2));
    
    // Search and filter controls
    auto topControls = area.removeFromTop(30);
    searchBox.setBounds(topControls.removeFromLeft(static_cast<int>(topControls.getWidth() * 0.7f)).reduced(2));
    filterBox.setBounds(topControls.reduced(2));
    
    // List box
    gearListBox->setBounds(area.reduced(0, 5));
}

int GearLibrary::getNumRows()
{
    return gearItems.size();
}

void GearLibrary::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int /*height*/, bool rowIsSelected)
{
    if (rowNumber >= 0 && rowNumber < gearItems.size())
    {
        const auto& item = gearItems[rowNumber];
        
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
    // In a real implementation, this would fetch gear data from a server
    // For now, we'll just simulate it
    
    class DummyLoadThread : public juce::Thread
    {
    public:
        DummyLoadThread(GearLibrary& ownerRef) : juce::Thread("GearLoader"), owner(ownerRef) {}
        
        void run() override
        {
            // Simulate network delay
            juce::Thread::sleep(500);
            
            // Create a JSON string with dummy gear data
            juce::String jsonData = 
                "{"
                "  \"gear\": ["
                "    {"
                "      \"name\": \"API 512c\","
                "      \"manufacturer\": \"API\","
                "      \"type\": \"500Series\","
                "      \"category\": \"Preamp\","
                "      \"slotSize\": 1,"
                "      \"imageUrl\": \"https://apiaudio.com/wp-content/uploads/2024/08/api-550a-23_9143_1.jpg\""
                "    },"
                "    {"
                "      \"name\": \"SSL Bus Comp\","
                "      \"manufacturer\": \"SSL\","
                "      \"type\": \"Rack19Inch\","
                "      \"category\": \"Compressor\","
                "      \"slotSize\": 1,"
                "      \"imageUrl\": \"https://brazilbox.us/wp-content/uploads/2021/07/1.jpg\""
                "    },"
                "    {"
                "      \"name\": \"Pultec EQP-1A\","
                "      \"manufacturer\": \"Pultec\","
                "      \"type\": \"Rack19Inch\","
                "      \"category\": \"EQ\","
                "      \"slotSize\": 2,"
                "      \"imageUrl\": \"https://media.sweetwater.com/m/products/image/2f36089c5b5svON7leBtYM0aeR6L8Ron1HzNhCVN.jpg\""
                "    }"
                "  ]"
                "}";
            
            juce::MessageManager::callAsync([this, jsonData]() {
                owner.parseGearLibrary(jsonData);
            });
        }
        
    private:
        GearLibrary& owner;
    };
    
    (new DummyLoadThread(*this))->startThread();
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
    if (index >= 0 && index < gearItems.size())
    {
        // Return a pointer to the item in the array
        return &(gearItems.getReference(index));
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