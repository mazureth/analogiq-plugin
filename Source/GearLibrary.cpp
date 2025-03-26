#include "GearLibrary.h"

GearLibrary::GearLibrary()
    : gearListBox("Gear Library", this)
{
    // Set up search box
    searchBox.setTextToShowWhenEmpty("Search gear...", juce::Colours::grey);
    searchBox.onTextChange = [this] { 
        gearListBox.updateContent();
        gearListBox.repaint();
    };
    addAndMakeVisible(searchBox);

    // Set up filter box
    filterBox.addItem("All Types", 1);
    filterBox.addItem("500 Series", 2);
    filterBox.addItem("19\" Rack", 3);
    filterBox.addItem("User Created", 4);
    filterBox.setSelectedId(1);
    filterBox.onChange = [this] {
        gearListBox.updateContent();
        gearListBox.repaint();
    };
    addAndMakeVisible(filterBox);

    // Set up refresh button
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { loadLibraryAsync(); };
    addAndMakeVisible(refreshButton);

    // Set up add user gear button
    addUserGearButton.setButtonText("Add Custom Gear");
    addUserGearButton.onClick = [this] { 
        // Open user gear creation dialog (to be implemented)
    };
    addAndMakeVisible(addUserGearButton);

    // Set up list box
    gearListBox.setRowHeight(60);
    gearListBox.setMultipleSelectionEnabled(false);
    addAndMakeVisible(gearListBox);

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
    auto area = getLocalBounds().reduced(5);
    
    // Title space
    area.removeFromTop(30);
    
    // Search box
    auto topControls = area.removeFromTop(30);
    searchBox.setBounds(topControls.removeFromLeft(topControls.getWidth() * 0.7f).reduced(2));
    filterBox.setBounds(topControls.reduced(2));
    
    // Buttons
    auto buttonArea = area.removeFromBottom(30);
    refreshButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2).reduced(2));
    addUserGearButton.setBounds(buttonArea.reduced(2));
    
    // List box
    gearListBox.setBounds(area.reduced(0, 5));
}

int GearLibrary::getNumRows()
{
    // Filter gear items based on search and filter
    juce::String searchText = searchBox.getText().toLowerCase();
    int filterType = filterBox.getSelectedId();
    
    int count = 0;
    for (auto& item : gearItems)
    {
        bool matchesSearch = searchText.isEmpty() || 
                            item.name.toLowerCase().contains(searchText) ||
                            item.manufacturer.toLowerCase().contains(searchText);
                            
        bool matchesFilter = filterType == 1 || 
                            (filterType == 2 && item.type == GearType::Series500) ||
                            (filterType == 3 && item.type == GearType::Rack19Inch) ||
                            (filterType == 4 && item.type == GearType::UserCreated);
                            
        if (matchesSearch && matchesFilter)
            count++;
    }
    
    return count;
}

void GearLibrary::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= 0 && rowNumber < gearItems.size())
    {
        auto& item = gearItems[rowNumber];
        
        // Background
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue.darker(0.7f));
        else if (rowNumber % 2)
            g.fillAll(juce::Colours::darkgrey.darker(0.5f));
        else
            g.fillAll(juce::Colours::darkgrey.darker(0.6f));
            
        // Text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(8);
        
        // Draw manufacturer
        g.setFont(12.0f);
        g.drawText(item.manufacturer, bounds.removeFromTop(15), juce::Justification::topLeft, true);
        
        // Draw name
        g.setFont(16.0f);
        g.drawText(item.name, bounds.removeFromTop(20), juce::Justification::topLeft, true);
        
        // Draw type
        g.setFont(12.0f);
        juce::String typeText;
        if (item.type == GearType::Series500)
            typeText = "500 Series";
        else if (item.type == GearType::Rack19Inch)
            typeText = "19\" Rack";
        else
            typeText = "User Gear";
            
        g.drawText(typeText, bounds, juce::Justification::topLeft, true);
    }
}

juce::Component* GearLibrary::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    // We're using paintListBoxItem for now, so return nullptr
    return nullptr;
}

void GearLibrary::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    // Select the row
    gearListBox.selectRow(row);
}

void GearLibrary::listBoxItemDoubleClicked(int row, const juce::MouseEvent& e)
{
    // Start drag and drop operation
    if (row >= 0 && row < gearItems.size())
    {
        juce::var dragData(row);
        juce::DragAndDropContainer* dragContainer = juce::DynamicCast<juce::DragAndDropContainer>(getParentComponent());
        
        if (dragContainer != nullptr)
        {
            dragContainer->startDragging(dragData, this, juce::ScaledImage(),
                                       true, nullptr, juce::Point<int>(-10, -10));
        }
    }
}

void GearLibrary::loadLibraryAsync()
{
    // In a real implementation, this would fetch gear data from a server
    // For now, we'll just simulate it
    
    class DummyLoadThread : public juce::Thread
    {
    public:
        DummyLoadThread(GearLibrary& owner) : juce::Thread("GearLoader"), owner(owner) {}
        
        void run() override
        {
            // Simulate server delay
            juce::Thread::sleep(1000);
            
            // Create some dummy JSON data
            juce::String jsonData = R"(
            {
                "gear": [
                    {
                        "name": "API 550A",
                        "manufacturer": "API",
                        "type": "500Series",
                        "category": "EQ",
                        "slotSize": 1,
                        "imageUrl": "https://example.com/api550a.jpg"
                    },
                    {
                        "name": "Neve 1073",
                        "manufacturer": "Neve",
                        "type": "Rack19Inch",
                        "category": "Preamp",
                        "slotSize": 2,
                        "imageUrl": "https://example.com/neve1073.jpg"
                    },
                    {
                        "name": "SSL G-Comp",
                        "manufacturer": "SSL",
                        "type": "Rack19Inch",
                        "category": "Compressor",
                        "slotSize": 1,
                        "imageUrl": "https://example.com/sslgcomp.jpg"
                    },
                    {
                        "name": "Empirical Labs Distressor",
                        "manufacturer": "Empirical Labs",
                        "type": "Rack19Inch",
                        "category": "Compressor",
                        "slotSize": 1,
                        "imageUrl": "https://example.com/distressor.jpg"
                    },
                    {
                        "name": "Chandler Limited TG2",
                        "manufacturer": "Chandler Limited",
                        "type": "Rack19Inch",
                        "category": "Preamp",
                        "slotSize": 1,
                        "imageUrl": "https://example.com/tg2.jpg"
                    },
                    {
                        "name": "API 525",
                        "manufacturer": "API",
                        "type": "500Series",
                        "category": "Compressor",
                        "slotSize": 1,
                        "imageUrl": "https://example.com/api525.jpg"
                    }
                ]
            })";
            
            // Update UI on the message thread
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
        gearListBox.updateContent();
        gearListBox.repaint();
    }
} 