#include "GearItem.h"
#include "GearLibrary.h" // Include for RemoteResources

bool GearItem::loadImage()
{
    // If no thumbnail specified, use placeholder
    if (thumbnailImage.isEmpty())
        return createPlaceholderImage();

    // Check if the thumbnail is a remote path
    if (thumbnailImage.startsWith("assets/") || thumbnailImage.startsWith("http"))
    {
        // Determine the full URL using the helper method
        juce::String imageUrl = GearLibrary::getFullUrl(thumbnailImage);

        // Create URL object for the image
        juce::URL url(imageUrl);

        // Try to download the image
        auto urlStream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                   .withConnectionTimeoutMs(10000)
                                                   .withNumRedirectsToFollow(5));

        if (urlStream != nullptr)
        {
            // Read the image data
            juce::MemoryBlock imageData;
            urlStream->readIntoMemoryBlock(imageData);

            if (imageData.getSize() > 0)
            {
                // Create image from the memory block
                juce::MemoryInputStream inputStream(imageData, false);
                juce::JPEGImageFormat jpegFormat;
                juce::PNGImageFormat pngFormat;

                // Try to load as JPEG first, then PNG
                if (jpegFormat.canUnderstand(inputStream))
                {
                    inputStream.setPosition(0);
                    image = jpegFormat.decodeImage(inputStream);
                }
                else
                {
                    inputStream.setPosition(0);
                    if (pngFormat.canUnderstand(inputStream))
                    {
                        inputStream.setPosition(0);
                        image = pngFormat.decodeImage(inputStream);
                    }
                }

                // If successfully loaded image, return true
                if (image.isValid())
                {
                    DBG("Successfully loaded thumbnail image for " + name);
                    return true;
                }
            }
        }

        DBG("Failed to load thumbnail image for " + name + ", using placeholder");
    }

    // If we get here, loading the actual image failed, so create a placeholder
    return createPlaceholderImage();
}

// Helper method to create a placeholder image
bool GearItem::createPlaceholderImage()
{
    // Create a placeholder colored image based on category
    image = juce::Image(juce::Image::ARGB, 24, 24, true);
    juce::Graphics g(image);

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

    return true;
}

void GearItem::saveToJSON(juce::File destinationFile)
{
    // Create a JSON object with all of our properties
    juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();

    // Add all properties to the JSON object
    jsonObj->setProperty("unitId", unitId);
    jsonObj->setProperty("name", name);
    jsonObj->setProperty("manufacturer", manufacturer);
    jsonObj->setProperty("category", categoryString);
    jsonObj->setProperty("version", version);
    jsonObj->setProperty("schemaPath", schemaPath);
    jsonObj->setProperty("thumbnailImage", thumbnailImage);

    // Convert GearType enum to string
    juce::String typeStr;
    switch (type)
    {
    case GearType::Series500:
        typeStr = "500Series";
        break;
    case GearType::Rack19Inch:
        typeStr = "Rack19Inch";
        break;
    case GearType::UserCreated:
        typeStr = "UserCreated";
        break;
    case GearType::Other:
        typeStr = "Other";
        break;
    }
    jsonObj->setProperty("type", typeStr);

    // Add slot size
    jsonObj->setProperty("slotSize", slotSize);

    // Add tags as an array
    juce::Array<juce::var> tagsArray;
    for (const auto &tag : tags)
        tagsArray.add(tag);
    jsonObj->setProperty("tags", tagsArray);

    // Add controls as an array
    juce::Array<juce::var> controlsArray;
    for (const auto &control : controls)
    {
        juce::DynamicObject::Ptr controlObj = new juce::DynamicObject();

        // Convert control type to string
        juce::String controlTypeStr;
        switch (control.type)
        {
        case GearControl::Type::Button:
            controlTypeStr = "Button";
            break;
        case GearControl::Type::Fader:
            controlTypeStr = "Fader";
            break;
        case GearControl::Type::Switch:
            controlTypeStr = "Switch";
            break;
        }

        controlObj->setProperty("type", controlTypeStr);
        controlObj->setProperty("name", control.name);

        // Convert position rectangle to JSON
        juce::DynamicObject::Ptr posObj = new juce::DynamicObject();
        posObj->setProperty("x", control.position.getX());
        posObj->setProperty("y", control.position.getY());
        posObj->setProperty("width", control.position.getWidth());
        posObj->setProperty("height", control.position.getHeight());

        controlObj->setProperty("position", posObj.get());
        controlObj->setProperty("value", control.value);

        controlsArray.add(controlObj.get());
    }

    jsonObj->setProperty("controls", controlsArray);

    // Convert to JSON string
    juce::var jsonVar(jsonObj.get());
    juce::String jsonString = juce::JSON::toString(jsonVar);

    // Write to file
    destinationFile.replaceWithText(jsonString);
}

GearItem GearItem::loadFromJSON(juce::File sourceFile)
{
    // Read the JSON from file
    juce::String jsonString = sourceFile.loadFileAsString();

    // Parse the JSON
    juce::var jsonVar = juce::JSON::parse(jsonString);

    // Check if we have a valid JSON object
    if (!jsonVar.isObject())
        return GearItem(); // Return empty gear item

    // Extract fields from JSON
    juce::String unitId = jsonVar.getProperty("unitId", "");
    juce::String name = jsonVar.getProperty("name", "");
    juce::String manufacturer = jsonVar.getProperty("manufacturer", "");
    juce::String categoryStr = jsonVar.getProperty("category", "");
    juce::String version = jsonVar.getProperty("version", "1.0.0");
    juce::String schemaPath = jsonVar.getProperty("schemaPath", "");
    juce::String thumbnailImage = jsonVar.getProperty("thumbnailImage", "");

    // Parse tags
    juce::StringArray tags;
    if (jsonVar.hasProperty("tags") && jsonVar["tags"].isArray())
    {
        juce::Array<juce::var> *tagsArray = jsonVar["tags"].getArray();
        for (const auto &tag : *tagsArray)
            tags.add(tag.toString());
    }

    // Determine type
    GearType type = GearType::Other;
    juce::String typeStr = jsonVar.getProperty("type", "");
    if (typeStr == "500Series")
        type = GearType::Series500;
    else if (typeStr == "Rack19Inch")
        type = GearType::Rack19Inch;
    else if (typeStr == "UserCreated")
        type = GearType::UserCreated;

    // Determine category
    GearCategory category = GearCategory::Other;
    if (categoryStr == "equalizer" || categoryStr == "eq")
        category = GearCategory::EQ;
    else if (categoryStr == "compressor")
        category = GearCategory::Compressor;
    else if (categoryStr == "preamp")
        category = GearCategory::Preamp;

    // Get slot size
    int slotSize = jsonVar.getProperty("slotSize", 1);

    // Parse controls
    juce::Array<GearControl> controls;
    if (jsonVar.hasProperty("controls") && jsonVar["controls"].isArray())
    {
        juce::Array<juce::var> *controlsArray = jsonVar["controls"].getArray();
        for (const auto &controlVar : *controlsArray)
        {
            if (!controlVar.isObject())
                continue;

            // Get control type
            GearControl::Type controlType = GearControl::Type::Button;
            juce::String controlTypeStr = controlVar.getProperty("type", "Button");
            if (controlTypeStr == "Fader")
                controlType = GearControl::Type::Fader;
            else if (controlTypeStr == "Switch")
                controlType = GearControl::Type::Switch;

            // Get control name
            juce::String controlName = controlVar.getProperty("name", "");

            // Get position
            juce::Rectangle<float> position;
            if (controlVar.hasProperty("position") && controlVar["position"].isObject())
            {
                position.setX(controlVar["position"].getProperty("x", 0.0f));
                position.setY(controlVar["position"].getProperty("y", 0.0f));
                position.setWidth(controlVar["position"].getProperty("width", 0.0f));
                position.setHeight(controlVar["position"].getProperty("height", 0.0f));
            }

            // Create control and add to array
            GearControl control(controlType, controlName, position);
            control.value = controlVar.getProperty("value", 0.0f);
            controls.add(control);
        }
    }

    // Create and return the gear item with the new constructor
    GearItem item(unitId, name, manufacturer, categoryStr, version, schemaPath,
                  thumbnailImage, tags, type, category, slotSize, controls);

    // Try to load the image
    item.loadImage();

    return item;
}