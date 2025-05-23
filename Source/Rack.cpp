#include "Rack.h"
#include "GearLibrary.h"

Rack::Rack()
{
    DBG("Rack constructor");
    setComponentID("Rack");

    // Create viewport and container
    rackViewport = std::make_unique<juce::Viewport>();
    rackContainer = std::make_unique<RackContainer>();
    rackViewport->setViewedComponent(rackContainer.get(), false); // false = don't delete when viewport is deleted
    addAndMakeVisible(rackViewport.get());

    // Set up the container
    rackContainer->rack = this;

    // Create rack slots
    DBG("Creating " + juce::String(numSlots) + " rack slots");
    for (int i = 0; i < numSlots; ++i)
    {
        RackSlot *newSlot = new RackSlot(i);
        slots.add(newSlot);
        rackContainer->addAndMakeVisible(newSlot);
    }

    // Set up this component as a drag-and-drop target
    setInterceptsMouseClicks(true, true);
}

Rack::~Rack()
{
    DBG("Rack destructor");
    // unique_ptr members will be automatically deleted
}

void Rack::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);
}

int Rack::getSlotHeight(int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= slots.size())
        return getDefaultSlotHeight();

    RackSlot *slot = slots[slotIndex];
    if (slot == nullptr)
        return getDefaultSlotHeight();

    // If the slot has a gear item with a faceplate image, use the image's height plus padding
    GearItem *item = slot->getGearItem();
    if (item != nullptr && item->faceplateImage.isValid())
    {
        // Calculate a reasonable height based on the faceplate image
        // Use aspect ratio of the image, but constrained to reasonable bounds
        int imageHeight = item->faceplateImage.getHeight();
        int imageWidth = item->faceplateImage.getWidth();

        if (imageHeight > 0 && imageWidth > 0)
        {
            // Calculate what the height would be if the width matched the slot width
            // Add extra padding for controls and slot UI elements
            int effectiveSlotWidth = rackContainer->getWidth() - (2 * slotSpacing);
            int scaledHeight = (imageHeight * effectiveSlotWidth) / imageWidth;

            // Add padding for slot UI elements (buttons, labels, etc.)
            int paddedHeight = scaledHeight + 40; // 20px padding top and bottom

            // Constrain to reasonable bounds
            return juce::jlimit(100, 400, paddedHeight);
        }
    }

    // Default height if no special considerations apply
    return getDefaultSlotHeight();
}

void Rack::resized()
{
    DBG("Rack::resized");

    // Size the viewport to fill our area
    auto area = getLocalBounds();
    rackViewport->setBounds(area);

    // Get the available width from the viewport
    const int availableWidth = rackViewport->getWidth();

    // Calculate total height needed for all slots with their variable heights
    int totalHeight = slotSpacing; // Start with top spacing
    for (int i = 0; i < slots.size(); ++i)
    {
        totalHeight += getSlotHeight(i) + slotSpacing;
    }

    // Size the container to fit all slots with spacing
    const int containerWidth = availableWidth; // Use full viewport width
    rackContainer->setSize(containerWidth, totalHeight);

    // Calculate the slot width based on container width minus margins
    const int effectiveSlotWidth = containerWidth - (2 * slotSpacing);

    // Position the slots within the container
    int currentY = slotSpacing;
    for (int i = 0; i < slots.size(); ++i)
    {
        auto *slot = slots[i];
        int slotHeight = getSlotHeight(i);

        slot->setBounds(
            slotSpacing,
            currentY,
            effectiveSlotWidth,
            slotHeight);

        currentY += slotHeight + slotSpacing;
    }

    DBG("Rack resized: viewport=" + rackViewport->getBounds().toString() +
        ", container=" + rackContainer->getBounds().toString());
}

bool Rack::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    // Accept drops from DraggableListBox (GearLibrary) or other RackSlots
    auto *sourceComp = dragSourceDetails.sourceComponent.get();

    // Check for drags from the legacy list box
    if (sourceComp && (sourceComp->getComponentID() == "DraggableListBox" ||
                       sourceComp->getComponentID() == "GearListBox" ||
                       dynamic_cast<RackSlot *>(sourceComp) != nullptr))
    {
        return true;
    }

    // Check for drags from the TreeView
    if (sourceComp && dynamic_cast<juce::TreeView *>(sourceComp) != nullptr)
    {
        if (dragSourceDetails.description.isString())
        {
            juce::String desc = dragSourceDetails.description.toString();
            if (desc.startsWith("GEAR:"))
            {
                return true;
            }
        }
    }

    return false;
}

void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/)
{
    // Nothing to do here
}

void Rack::itemDragMove(const juce::DragAndDropTarget::SourceDetails &details)
{
    // This is only used for dragging from GearLibrary now, not for reordering
    RackSlot *nearestSlot = findNearestSlot(details.localPosition);

    // Highlight the nearest slot
    for (auto *slot : slots)
    {
        slot->setHighlighted(slot == nearestSlot);
    }
}

void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/)
{
    // Clear all highlights
    for (auto *slot : slots)
    {
        slot->setHighlighted(false);
    }
}

void Rack::itemDropped(const juce::DragAndDropTarget::SourceDetails &details)
{
    DBG("Rack::itemDropped");

    // Find the nearest slot to the drop position
    RackSlot *targetSlot = findNearestSlot(details.localPosition);
    if (targetSlot == nullptr)
    {
        DBG("No target slot found for drop position");
        return;
    }

    // Clear highlights
    for (auto *slot : slots)
    {
        slot->setHighlighted(false);
    }

    // Handle drops from GearLibrary (legacy list box)
    if (details.description.isInt() && gearLibrary != nullptr)
    {
        int gearIndex = static_cast<int>(details.description);
        GearItem *item = gearLibrary->getGearItem(gearIndex);

        if (item != nullptr)
        {
            DBG("Adding gear item from listbox: " + item->name + " to slot " + juce::String(targetSlot->getIndex()));
            targetSlot->setGearItem(item);

            // Fetch schema for this item
            fetchSchemaForGearItem(item);
        }
    }
    // Handle drops from TreeView
    else if (details.description.isString() && gearLibrary != nullptr)
    {
        juce::String desc = details.description.toString();
        if (desc.startsWith("GEAR:"))
        {
            // Parse the gear index from the drag descriptor
            juce::StringArray parts;
            parts.addTokens(desc, ":", "");

            if (parts.size() >= 3)
            {
                int gearIndex = parts[1].getIntValue();
                GearItem *item = gearLibrary->getGearItem(gearIndex);

                if (item != nullptr)
                {
                    DBG("Adding gear item from TreeView: " + item->name + " to slot " + juce::String(targetSlot->getIndex()));
                    targetSlot->setGearItem(item);

                    // Fetch schema for this item
                    fetchSchemaForGearItem(item);
                }
            }
        }
    }
    // Handle drops from other RackSlots (rearranging)
    else if (auto *sourceSlot = dynamic_cast<RackSlot *>(details.sourceComponent.get()))
    {
        if (sourceSlot != targetSlot && sourceSlot->getGearItem() != nullptr)
        {
            // Swap the items between slots
            GearItem *sourceItem = sourceSlot->getGearItem();
            GearItem *targetItem = targetSlot->getGearItem();

            sourceSlot->setGearItem(targetItem);
            targetSlot->setGearItem(sourceItem);

            DBG("Swapped gear items between slots " + juce::String(sourceSlot->getIndex()) +
                " and " + juce::String(targetSlot->getIndex()));
        }
    }
}

void Rack::rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex)
{
    DBG("===============================================");
    DBG("Rack::rearrangeGearAsSortableList - sourceIndex: " + juce::String(sourceSlotIndex) +
        ", targetIndex: " + juce::String(targetSlotIndex));

    // Validate indices
    if (sourceSlotIndex < 0 || sourceSlotIndex >= slots.size() ||
        targetSlotIndex < 0 || targetSlotIndex >= slots.size() ||
        sourceSlotIndex == targetSlotIndex)
    {
        DBG("Invalid source or target index, or they are the same. Aborting rearrangement.");
        return;
    }

    // Get pointers to the source and target slots
    RackSlot *sourceSlot = slots[sourceSlotIndex];
    RackSlot *targetSlot = slots[targetSlotIndex];

    if (sourceSlot == nullptr || targetSlot == nullptr)
    {
        DBG("Source or target slot is null. Aborting rearrangement.");
        return;
    }

    // Get the gear items from both slots
    GearItem *sourceGearItem = sourceSlot->getGearItem();
    GearItem *targetGearItem = targetSlot->getGearItem();

    if (sourceGearItem == nullptr)
    {
        DBG("Source gear item is null. Cannot move an empty slot. Aborting rearrangement.");
        return;
    }

    // Simply swap the two slots
    DBG("Swapping gear items between slot " + juce::String(sourceSlotIndex) +
        " and slot " + juce::String(targetSlotIndex));

    // First clear both slots
    sourceSlot->clearGearItem();
    targetSlot->clearGearItem();

    // Then set the items in their new positions
    targetSlot->setGearItem(sourceGearItem);

    // If the target slot had an item, move it to the source slot
    if (targetGearItem != nullptr)
    {
        sourceSlot->setGearItem(targetGearItem);
    }

    // Update the rack view - call resized() on the rack itself to recalculate all slot heights
    resized();

    DBG("Gear items successfully swapped.");
    DBG("===============================================");
}

RackSlot *Rack::findNearestSlot(const juce::Point<int> &position)
{
    // This method is kept for compatibility with drops from GearLibrary
    // but we've simplified it since we're not using it for reordering

    // Convert global position to container coordinates
    auto containerPos = rackContainer->getLocalPoint(this, position);

    // First check for a direct hit
    for (auto *slot : slots)
    {
        if (slot->getBounds().contains(containerPos))
        {
            return slot;
        }
    }

    // If no direct hit, find the closest slot
    RackSlot *bestSlot = nullptr;
    int bestDistance = std::numeric_limits<int>::max();

    for (auto *slot : slots)
    {
        juce::Rectangle<int> slotBounds = slot->getBounds();
        int distance = slotBounds.getCentre().getDistanceFrom(containerPos);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestSlot = slot;
        }
    }

    return bestSlot;
}

// New method to fetch schema for a gear item
void Rack::fetchSchemaForGearItem(GearItem *item)
{
    if (item == nullptr || item->schemaPath.isEmpty())
    {
        DBG("Cannot fetch schema: item is null or schema path is empty");
        return;
    }

    DBG("Fetching schema for " + item->name + " from " + item->schemaPath);

    // Construct the full URL if it's a relative path
    juce::String fullUrl = item->schemaPath;
    if (!fullUrl.startsWith("http"))
    {
        fullUrl = GearLibrary::getFullUrl(fullUrl);
    }

    DBG("Full schema URL: " + fullUrl);

    // Use JUCE's URL class to fetch the schema asynchronously
    juce::URL schemaUrl(fullUrl);

    // Create a new thread to download the schema asynchronously
    struct SchemaDownloader : public juce::Thread
    {
        SchemaDownloader(juce::URL urlToUse, GearItem *itemToUpdate, Rack *rackToNotify)
            : juce::Thread("Schema Downloader"),
              url(urlToUse), item(itemToUpdate), rack(rackToNotify)
        {
            startThread();
        }

        ~SchemaDownloader() override
        {
            stopThread(2000);
        }

        void run() override
        {
            DBG("SchemaDownloader thread started for " + item->name);

            // Download the schema data
            schemaData = url.readEntireTextStream(false);

            if (threadShouldExit())
                return;

            success = schemaData.isNotEmpty();

            // Need to get back on the message thread to update the UI
            juce::MessageManager::callAsync([this]()
                                            {
                if (success)
                {
                    DBG("Successfully downloaded schema for " + item->name);
                    rack->parseSchema(schemaData, item);
                }
                else
                {
                    DBG("Failed to download schema for " + item->name);
                }
                delete this; });
        }

        juce::URL url;
        GearItem *item;
        Rack *rack;
        juce::String schemaData;
        bool success = false;
    };

    // Create and start the download thread (it will delete itself when done)
    new SchemaDownloader(schemaUrl, item, this);
}

// Method to parse the schema data
void Rack::parseSchema(const juce::String &schemaData, GearItem *item)
{
    if (schemaData.isEmpty() || item == nullptr)
    {
        DBG("Cannot parse schema: data is empty or item is null");
        return;
    }

    DBG("Parsing schema for " + item->name);
    DBG("Schema data: " + schemaData.substring(0, 500) + "..."); // Print first part of schema for debugging

    // Parse the JSON schema
    juce::var schemaJson = juce::JSON::parse(schemaData);

    if (!schemaJson.isObject())
    {
        DBG("Schema parsing failed: not a valid JSON object");
        return;
    }

    // Extract schema information
    auto *schemaObj = schemaJson.getDynamicObject();
    if (schemaObj == nullptr)
    {
        DBG("Schema parsing failed: couldn't get dynamic object");
        return;
    }

    // Debug all available properties in the schema
    DBG("Schema contains the following properties:");
    for (auto &prop : schemaObj->getProperties())
    {
        DBG(" - " + prop.name.toString());
    }

    // Check for different possible property names for the faceplate image
    bool foundFaceplate = false;
    juce::String faceplateImagePath;

    // List of possible property names for the faceplate image
    const std::vector<juce::String> faceplateProperties = {
        "faceplateImage", "faceplate", "panelImage", "panel", "frontPanel",
        "image", "fullImage", "uiImage", "mainImage"};

    // Try each possible property name
    for (const auto &propName : faceplateProperties)
    {
        if (schemaJson.hasProperty(propName))
        {
            faceplateImagePath = schemaJson.getProperty(propName, "").toString();
            if (faceplateImagePath.isNotEmpty())
            {
                DBG("Found faceplate image with property name '" + propName + "': " + faceplateImagePath);
                foundFaceplate = true;

                // Store the path in the gear item
                item->faceplateImagePath = faceplateImagePath;

                // Fetch the faceplate image
                fetchFaceplateImage(item);
                break;
            }
        }
    }

    if (!foundFaceplate)
    {
        DBG("No faceplate image found in schema. Using thumbnail image instead.");
    }

    // Update controls if available
    if (schemaJson.hasProperty("controls") && schemaJson["controls"].isArray())
    {
        item->controls.clear();

        auto controlsArray = schemaJson["controls"].getArray();
        for (auto &controlVar : *controlsArray)
        {
            if (!controlVar.isObject())
                continue;

            // Get control type
            GearControl::Type controlType = GearControl::Type::Knob;
            juce::String controlTypeStr = controlVar.getProperty("type", "Knob");
            if (controlTypeStr == "Button")
                controlType = GearControl::Type::Button;
            else if (controlTypeStr == "Fader")
                controlType = GearControl::Type::Fader;

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
            item->controls.add(control);
        }

        DBG("Added " + juce::String(item->controls.size()) + " controls to " + item->name);
    }

    // Notify that the schema has been loaded
    DBG("Schema successfully parsed for " + item->name);
}

// New method to fetch the faceplate image
void Rack::fetchFaceplateImage(GearItem *item)
{
    if (item == nullptr || item->faceplateImagePath.isEmpty())
    {
        DBG("Cannot fetch faceplate image: item is null or faceplate path is empty");
        return;
    }

    DBG("Fetching faceplate image for " + item->name + " from " + item->faceplateImagePath);

    // Construct the full URL if it's a relative path
    juce::String fullUrl = item->faceplateImagePath;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = GearLibrary::getFullUrl(fullUrl);
        }
    }

    DBG("Full faceplate image URL: " + fullUrl);

    // Use JUCE's URL class to fetch the image asynchronously
    juce::URL imageUrl(fullUrl);

    // Direct image loading isn't available in this version of JUCE, so skip straight to async
    DBG("Starting async download for faceplate image...");

    // Create a new thread to download the image asynchronously
    struct FaceplateImageDownloader : public juce::Thread
    {
        FaceplateImageDownloader(juce::URL urlToUse, GearItem *itemToUpdate, Rack *parentRack)
            : juce::Thread("Faceplate Image Downloader"),
              url(urlToUse), item(itemToUpdate), rack(parentRack)
        {
            startThread();
        }

        ~FaceplateImageDownloader() override
        {
            stopThread(2000);
        }

        void run() override
        {
            DBG("FaceplateImageDownloader thread started for " + item->name);

            // Try to download using the simple API - this is for older JUCE versions
            std::unique_ptr<juce::InputStream> inputStream = url.createInputStream(false);

            if (inputStream == nullptr || threadShouldExit())
            {
                DBG("Failed to create input stream for faceplate image: " + url.toString(true));

                // Clean up on the message thread
                juce::MessageManager::callAsync([this]()
                                                { delete this; });

                return;
            }

            // Try to determine image format
            juce::String urlStr = url.toString(true).toLowerCase();
            juce::ImageFileFormat *format = nullptr;

            if (urlStr.contains(".jpg") || urlStr.contains(".jpeg"))
                format = new juce::JPEGImageFormat();
            else if (urlStr.contains(".png"))
                format = new juce::PNGImageFormat();
            else if (urlStr.contains(".gif"))
                format = new juce::GIFImageFormat();

            // Load the image from the input stream
            juce::Image downloadedImage;

            if (format != nullptr)
            {
                // Use specific format if we could determine it
                downloadedImage = format->decodeImage(*inputStream);
                delete format;
            }
            else
            {
                // Otherwise use the generic loader
                downloadedImage = juce::ImageFileFormat::loadFrom(*inputStream);
            }

            if (downloadedImage.isValid())
            {
                DBG("Successfully loaded image with dimensions: " +
                    juce::String(downloadedImage.getWidth()) + "x" +
                    juce::String(downloadedImage.getHeight()));

                // Need to get back on the message thread to update the UI
                juce::MessageManager::callAsync([this, downloadedImage]()
                                                {
                    DBG("Successfully downloaded faceplate image for " + item->name);
                    
                    // Update the item's faceplate image
                    item->faceplateImage = downloadedImage;
                    
                    // Notify any slots that have this item to repaint
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot && slot->getGearItem() == item)
                            {
                                DBG("Repainting slot " + juce::String(i) + " with faceplate image");
                                slot->repaint();
                            }
                        }
                        
                        // Trigger a re-layout to adjust slot heights for the new image
                        rack->resized();
                    }
                    
                    delete this; });
            }
            else
            {
                // Image loading failed, clean up
                juce::MessageManager::callAsync([this]()
                                                {
                    DBG("Failed to load faceplate image for " + item->name);
                    
                    // Create a placeholder image instead
                    juce::Image placeholderImage(juce::Image::RGB, 200, 100, true);
                    juce::Graphics g(placeholderImage);
                    g.fillAll(juce::Colours::darkgrey);
                    g.setColour(juce::Colours::white);
                    g.drawText("Faceplate Unavailable", placeholderImage.getBounds(), juce::Justification::centred, true);
                    
                    // Set as faceplate image
                    item->faceplateImage = placeholderImage;
                    
                    // Repaint any slots with this item
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot && slot->getGearItem() == item)
                            {
                                slot->repaint();
                            }
                        }
                        
                        // Trigger a re-layout to adjust slot heights for the placeholder image
                        rack->resized();
                    }
                    
                    delete this; });
            }
        }

        juce::URL url;
        GearItem *item;
        Rack *rack;
    };

    // Create and start the download thread (it will delete itself when done)
    new FaceplateImageDownloader(imageUrl, item, this);
}