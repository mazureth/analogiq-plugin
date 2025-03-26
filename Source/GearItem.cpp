#include "GearItem.h"

bool GearItem::loadImage()
{
    // In a real implementation, this would load the image from the URL or local cache
    // For now, we'll just create a dummy image
    
    image = juce::Image(juce::Image::RGB, 200, 100, true);
    juce::Graphics g(image);
    
    // Draw a placeholder image
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::black);
    g.drawRect(0, 0, image.getWidth(), image.getHeight(), 1);
    
    // Draw name
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText(manufacturer + " - " + name, 
               juce::Rectangle<int>(0, 0, image.getWidth(), image.getHeight()),
               juce::Justification::centred, true);
    
    return true;
}

void GearItem::saveToJSON(juce::File destinationFile)
{
    juce::DynamicObject* obj = new juce::DynamicObject();
    
    // Add gear properties
    obj->setProperty("name", name);
    obj->setProperty("manufacturer", manufacturer);
    
    // Set type
    juce::String typeStr;
    switch (type)
    {
        case GearType::Series500: typeStr = "500Series"; break;
        case GearType::Rack19Inch: typeStr = "Rack19Inch"; break;
        case GearType::UserCreated: typeStr = "UserCreated"; break;
    }
    obj->setProperty("type", typeStr);
    
    // Set category
    juce::String catStr;
    switch (category)
    {
        case GearCategory::EQ: catStr = "EQ"; break;
        case GearCategory::Compressor: catStr = "Compressor"; break;
        case GearCategory::Preamp: catStr = "Preamp"; break;
        case GearCategory::Other: catStr = "Other"; break;
    }
    obj->setProperty("category", catStr);
    
    obj->setProperty("slotSize", slotSize);
    obj->setProperty("imageUrl", imageUrl);
    
    // Save controls
    juce::Array<juce::var> controlsArray;
    
    for (auto& control : controls)
    {
        juce::DynamicObject* controlObj = new juce::DynamicObject();
        
        // Set control type
        juce::String controlTypeStr;
        switch (control.type)
        {
            case GearControl::Type::Knob: controlTypeStr = "Knob"; break;
            case GearControl::Type::Button: controlTypeStr = "Button"; break;
            case GearControl::Type::Fader: controlTypeStr = "Fader"; break;
        }
        controlObj->setProperty("type", controlTypeStr);
        
        controlObj->setProperty("name", control.name);
        
        // Save position
        juce::DynamicObject* posObj = new juce::DynamicObject();
        posObj->setProperty("x", control.position.getX());
        posObj->setProperty("y", control.position.getY());
        posObj->setProperty("width", control.position.getWidth());
        posObj->setProperty("height", control.position.getHeight());
        controlObj->setProperty("position", posObj);
        
        controlObj->setProperty("value", control.value);
        
        controlsArray.add(controlObj);
    }
    
    obj->setProperty("controls", controlsArray);
    
    // Write to file
    juce::var json(obj);
    destinationFile.replaceWithText(juce::JSON::toString(json));
}

GearItem GearItem::loadFromJSON(juce::File sourceFile)
{
    GearItem item;
    
    if (sourceFile.existsAsFile())
    {
        juce::String jsonStr = sourceFile.loadFileAsString();
        auto json = juce::JSON::parse(jsonStr);
        
        if (json.isObject())
        {
            auto obj = json.getDynamicObject();
            
            // Load properties
            item.name = obj->getProperty("name").toString();
            item.manufacturer = obj->getProperty("manufacturer").toString();
            
            // Get type
            juce::String typeStr = obj->getProperty("type");
            if (typeStr == "500Series")
                item.type = GearType::Series500;
            else if (typeStr == "Rack19Inch")
                item.type = GearType::Rack19Inch;
            else
                item.type = GearType::UserCreated;
            
            // Get category
            juce::String catStr = obj->getProperty("category");
            if (catStr == "EQ")
                item.category = GearCategory::EQ;
            else if (catStr == "Compressor")
                item.category = GearCategory::Compressor;
            else if (catStr == "Preamp")
                item.category = GearCategory::Preamp;
            else
                item.category = GearCategory::Other;
            
            item.slotSize = (int)obj->getProperty("slotSize");
            item.imageUrl = obj->getProperty("imageUrl").toString();
            
            // Load controls
            if (obj->hasProperty("controls") && obj->getProperty("controls").isArray())
            {
                auto controlsArray = obj->getProperty("controls").getArray();
                
                for (auto& controlJson : *controlsArray)
                {
                    if (controlJson.isObject())
                    {
                        auto controlObj = controlJson.getDynamicObject();
                        
                        // Get control type
                        GearControl::Type controlType;
                        juce::String controlTypeStr = controlObj->getProperty("type");
                        if (controlTypeStr == "Knob")
                            controlType = GearControl::Type::Knob;
                        else if (controlTypeStr == "Button")
                            controlType = GearControl::Type::Button;
                        else
                            controlType = GearControl::Type::Fader;
                        
                        juce::String name = controlObj->getProperty("name");
                        
                        // Get position
                        juce::Rectangle<float> position;
                        if (controlObj->hasProperty("position") && controlObj->getProperty("position").isObject())
                        {
                            auto posObj = controlObj->getProperty("position").getDynamicObject();
                            position.setX((float)posObj->getProperty("x"));
                            position.setY((float)posObj->getProperty("y"));
                            position.setWidth((float)posObj->getProperty("width"));
                            position.setHeight((float)posObj->getProperty("height"));
                        }
                        
                        // Create control
                        GearControl control(controlType, name, position);
                        control.value = (float)controlObj->getProperty("value");
                        
                        item.controls.add(control);
                    }
                }
            }
        }
    }
    
    return item;
} 