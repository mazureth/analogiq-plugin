/**
 * @file Rack.cpp
 * @brief Implementation of the Rack class for managing audio gear in a virtual rack.
 *
 * This file implements the Rack class which provides a visual interface for managing
 * audio gear items in a virtual rack system. It handles the layout, drag-and-drop
 * functionality, and resource management for gear items and their controls.
 */

#include "Rack.h"
#include "GearLibrary.h"
#include <fstream>

// Set up logging to file
static std::ofstream logFile("/tmp/rack.log");

// Helper function for logging
static void logToFile(const juce::String &message)
{
    if (logFile.is_open())
    {
        logFile << message << std::endl;
        logFile.flush();
    }
}

// Replace DBG with logToFile
#define DBG(msg) logToFile(msg)

/**
 * @brief Constructs a new Rack instance.
 *
 * Initializes the rack with a viewport and container, creates the specified number
 * of rack slots, and sets up drag-and-drop functionality.
 */
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

/**
 * @brief Destructor for the Rack class.
 *
 * Cleans up resources and logs the destruction.
 */
Rack::~Rack()
{
    DBG("Rack destructor");
    // unique_ptr members will be automatically deleted
}

/**
 * @brief Paints the rack's background.
 *
 * @param g The graphics context to paint with
 */
void Rack::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);
}

/**
 * @brief Gets the height of a specific rack slot.
 *
 * @param slotIndex The index of the slot to get the height for
 * @return The height of the slot in pixels
 */
int Rack::getSlotHeight(int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= slots.size())
        return getDefaultSlotHeight();

    RackSlot *slot = slots[slotIndex];
    if (slot == nullptr || slot->isAvailable())
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

/**
 * @brief Handles resizing of the rack component.
 *
 * Adjusts the layout of the viewport, container, and all rack slots
 * based on the new dimensions.
 */
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

/**
 * @brief Checks if the rack is interested in a drag source.
 *
 * @param dragSourceDetails Details about the drag source
 * @return true if the rack accepts drops from this source
 */
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

/**
 * @brief Handles when a dragged item enters the rack.
 *
 * @param dragSourceDetails Details about the drag source
 */
void Rack::itemDragEnter(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/)
{
    // Nothing to do here
}

/**
 * @brief Handles when a dragged item moves over the rack.
 *
 * @param details Details about the drag source and position
 */
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

/**
 * @brief Handles when a dragged item exits the rack.
 *
 * @param dragSourceDetails Details about the drag source
 */
void Rack::itemDragExit(const juce::DragAndDropTarget::SourceDetails & /*dragSourceDetails*/)
{
    // Clear all highlights
    for (auto *slot : slots)
    {
        slot->setHighlighted(false);
    }
}

/**
 * @brief Handles when a dragged item is dropped onto the rack.
 *
 * @param details Details about the drag source and drop position
 */
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

            // Create a new instance of the item
            GearItem *newItem = new GearItem(*item); // Use copy constructor
            targetSlot->setGearItem(newItem);

            // Fetch schema for this item
            fetchSchemaForGearItem(newItem);
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

                    // Create a new instance of the item
                    GearItem *newItem = new GearItem(*item); // Use copy constructor
                    targetSlot->setGearItem(newItem);

                    // Fetch schema for this item
                    fetchSchemaForGearItem(newItem);
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

/**
 * @brief Rearranges gear items in the rack using a sortable list.
 *
 * @param sourceSlotIndex The index of the source slot
 * @param targetSlotIndex The index of the target slot
 */
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

/**
 * @brief Finds the nearest rack slot to a given position.
 *
 * @param position The position to find the nearest slot for
 * @return Pointer to the nearest RackSlot, or nullptr if none found
 */
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

/**
 * @brief Fetches the schema for a gear item.
 *
 * @param item The gear item to fetch the schema for
 */
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

    /**
     * @brief Thread for downloading and processing schema data.
     *
     * This struct handles the asynchronous download and processing of schema data
     * for gear items, ensuring UI updates happen on the message thread.
     */
    struct SchemaDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new SchemaDownloader.
         *
         * @param urlToUse The URL to download the schema from
         * @param itemToUpdate The gear item to update with the schema
         * @param rackToNotify The rack to notify when the schema is loaded
         */
        SchemaDownloader(juce::URL urlToUse, GearItem *itemToUpdate, Rack *rackToNotify)
            : juce::Thread("Schema Downloader"),
              url(urlToUse), item(itemToUpdate), rack(rackToNotify)
        {
            startThread();
        }

        /**
         * @brief Destructor for SchemaDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~SchemaDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the schema data and processes it on the message thread.
         */
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

        juce::URL url;           ///< The URL to download from
        GearItem *item;          ///< The gear item to update
        Rack *rack;              ///< The rack to notify
        juce::String schemaData; ///< The downloaded schema data
        bool success = false;    ///< Whether the download was successful
    };

    // Create and start the download thread (it will delete itself when done)
    new SchemaDownloader(schemaUrl, item, this);
}

/**
 * @brief Parses the schema data for a gear item.
 *
 * @param schemaData The JSON schema data to parse
 * @param item The gear item to update with the parsed schema
 */
void Rack::parseSchema(const juce::String &schemaData, GearItem *item)
{
    // Parse the JSON schema
    auto schemaJson = juce::JSON::parse(schemaData);
    if (!schemaJson.isObject())
    {
        DBG("Failed to parse schema JSON");
        return;
    }

    // Look for faceplate image properties
    juce::StringArray faceplateProperties = {"faceplateImage", "thumbnailImage"};
    juce::String faceplateImagePath;
    bool foundFaceplate = false;

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
            GearControl::Type controlType = GearControl::Type::Button;
            juce::String controlTypeStr = controlVar.getProperty("type", "button").toString().toLowerCase();
            if (controlTypeStr == "fader")
                controlType = GearControl::Type::Fader;
            else if (controlTypeStr == "switch")
                controlType = GearControl::Type::Switch;
            else if (controlTypeStr == "knob")
                controlType = GearControl::Type::Knob;

            // Get control name and ID
            juce::String controlName = controlVar.getProperty("label", "");
            juce::String controlId = controlVar.getProperty("id", ""); // Get ID from schema

            // If no ID is provided, generate one from the label
            if (controlId.isEmpty())
            {
                controlId = controlName.toLowerCase().replaceCharacters(" ", "-");
            }

            // Check if we already have a control with this ID
            bool controlExists = false;
            for (const auto &existingControl : item->controls)
            {
                if (existingControl.id == controlId)
                {
                    controlExists = true;
                    DBG("Control with ID " + controlId + " already exists, skipping duplicate");
                    break;
                }
            }

            if (controlExists)
                continue;

            // Get position
            juce::Rectangle<float> position;
            if (controlVar.hasProperty("position") && controlVar["position"].isObject())
            {
                position.setX(controlVar["position"].getProperty("x", 0.0f));
                position.setY(controlVar["position"].getProperty("y", 0.0f));
                position.setWidth(controlVar["position"].getProperty("width", 0.0f));
                position.setHeight(controlVar["position"].getProperty("height", 0.0f));
            }

            // Create control
            GearControl control(controlType, controlName, position);
            control.id = controlId; // Set the control ID
            DBG("Created control: " + controlName + " with ID: " + controlId);
            control.value = controlVar.getProperty("value", 0.0f);

            // Set type-specific properties
            switch (control.type)
            {
            case GearControl::Type::Switch:
            {
                // Get switch-specific properties
                control.orientation = controlVar.getProperty("orientation", "vertical").toString();
                control.currentIndex = controlVar.getProperty("currentIndex", 0);
                control.value = (float)control.currentIndex; // Set value based on currentIndex
                control.initialValue = control.value;        // Store initial value
                control.image = controlVar.getProperty("image", "").toString();

                // Parse options array
                if (controlVar.hasProperty("options") && controlVar["options"].isArray())
                {
                    auto optionsArray = controlVar["options"].getArray();
                    for (const auto &option : *optionsArray)
                    {
                        if (option.isObject())
                        {
                            GearControl::SwitchOptionFrame frame;
                            frame.value = option.getProperty("value", "").toString();
                            frame.label = option.getProperty("label", "").toString();

                            // Parse frame data
                            if (option.hasProperty("frame") && option["frame"].isObject())
                            {
                                auto frameObj = option["frame"];
                                frame.x = frameObj.getProperty("x", 0);
                                frame.y = frameObj.getProperty("y", 0);
                                frame.width = frameObj.getProperty("width", 0);
                                frame.height = frameObj.getProperty("height", 0);
                            }

                            control.switchFrames.add(frame);
                            control.options.add(frame.value);
                        }
                    }
                }

                // Add control to item before fetching sprite sheet
                item->controls.add(control);

                // Fetch the switch sprite sheet if one is specified
                if (control.image.isNotEmpty())
                {
                    DBG("Attempting to fetch switch sprite sheet for control: " + control.name);
                    fetchSwitchSpriteSheet(item, item->controls.size() - 1);
                }
                else
                {
                    DBG("No image path specified for switch control: " + control.name);
                }
                break;
            }

            case GearControl::Type::Fader:
            {
                // Get fader-specific properties
                control.orientation = controlVar.getProperty("orientation", "vertical").toString();
                control.length = controlVar.getProperty("length", 100);
                control.value = controlVar.getProperty("value", 0.0f);
                control.initialValue = control.value; // Store initial value
                control.image = controlVar.getProperty("image", "").toString();

                DBG("Creating fader control: " + control.name +
                    " Orientation: " + control.orientation +
                    " Length: " + juce::String(control.length) +
                    " InitialValue: " + juce::String(control.value));

                // Add control to item before fetching image
                item->controls.add(control);

                // Fetch the fader image if one is specified
                if (control.image.isNotEmpty())
                {
                    DBG("Attempting to fetch fader image for control: " + control.name);
                    fetchFaderImage(item, item->controls.size() - 1);
                }
                else
                {
                    DBG("No image path specified for fader control: " + control.name);
                }
                break;
            }

            case GearControl::Type::Knob:
            {
                // Get knob-specific properties
                float startAngle = controlVar.getProperty("startAngle", 0.0f);
                float endAngle = controlVar.getProperty("endAngle", 360.0f);
                float value = controlVar.getProperty("value", 0.0f);

                DBG("Creating knob control: " + control.name +
                    " StartAngle: " + juce::String(startAngle) +
                    " EndAngle: " + juce::String(endAngle) +
                    " InitialValue: " + juce::String(value));

                control.startAngle = startAngle;
                control.endAngle = endAngle;
                control.value = value;
                control.initialValue = value; // Store the initial value from schema
                control.image = controlVar.getProperty("image", "").toString();
                DBG("Knob image path from schema: " + control.image);

                // Handle stepped knobs
                if (controlVar.hasProperty("steps") && controlVar["steps"].isArray())
                {
                    juce::Array<juce::var> *stepsArray = controlVar["steps"].getArray();
                    for (const auto &step : *stepsArray)
                        control.steps.add(step);
                    control.currentStepIndex = controlVar.getProperty("currentStepIndex", 0);
                }

                // Add control to item before fetching image
                item->controls.add(control);

                // Fetch the knob image if one is specified
                if (control.image.isNotEmpty())
                {
                    DBG("Attempting to fetch knob image for control: " + control.name);
                    // Pass the index of the control we just added
                    fetchKnobImage(item, item->controls.size() - 1);
                }
                else
                {
                    DBG("No image path specified for knob control: " + control.name);
                }
                break;
            }

            case GearControl::Type::Button:
            {
                // Get button-specific properties
                control.momentary = controlVar.getProperty("momentary", false);
                control.value = controlVar.getProperty("value", 0.0f);
                control.initialValue = control.value; // Store initial value
                control.image = controlVar.getProperty("image", "").toString();

                // Parse options array for button states
                if (controlVar.hasProperty("options") && controlVar["options"].isArray())
                {
                    auto optionsArray = controlVar["options"].getArray();
                    for (const auto &option : *optionsArray)
                    {
                        if (option.isObject())
                        {
                            GearControl::SwitchOptionFrame frame;
                            frame.value = option.getProperty("value", "").toString();
                            frame.label = option.getProperty("label", "").toString();

                            // Parse frame data
                            if (option.hasProperty("frame") && option["frame"].isObject())
                            {
                                auto frameObj = option["frame"];
                                frame.x = frameObj.getProperty("x", 0);
                                frame.y = frameObj.getProperty("y", 0);
                                frame.width = frameObj.getProperty("width", 0);
                                frame.height = frameObj.getProperty("height", 0);
                            }

                            control.buttonFrames.add(frame);
                            control.options.add(frame.value);
                        }
                    }
                }

                // Set currentIndex based on value
                if (control.momentary)
                {
                    control.currentIndex = control.value > 0.5f ? 1 : 0;
                }
                else
                {
                    control.currentIndex = (int)control.value;
                }

                // Add control to item before fetching sprite sheet
                item->controls.add(control);

                // Fetch the button sprite sheet if one is specified
                if (control.image.isNotEmpty())
                {
                    DBG("Attempting to fetch button sprite sheet for control: " + control.name);
                    fetchButtonSpriteSheet(item, item->controls.size() - 1);
                }
                else
                {
                    DBG("No image path specified for button control: " + control.name);
                }
                break;
            }

            default:
                // For non-knob controls, just add them
                item->controls.add(control);
                break;
            }
        }

        DBG("Added " + juce::String(item->controls.size()) + " controls to " + item->name);
    }

    // Notify that the schema has been loaded
    DBG("Schema successfully parsed for " + item->name);
}

/**
 * @brief Fetches the faceplate image for a gear item.
 *
 * @param item The gear item to fetch the faceplate for
 */
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

    /**
     * @brief Thread for downloading faceplate images.
     *
     * This struct handles the asynchronous download and processing of faceplate images
     * for gear items, ensuring UI updates happen on the message thread.
     */
    struct FaceplateImageDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new FaceplateImageDownloader.
         *
         * @param urlToUse The URL to download the image from
         * @param itemToUpdate The gear item to update with the image
         * @param parentRack The rack to notify when the image is loaded
         */
        FaceplateImageDownloader(juce::URL urlToUse, GearItem *itemToUpdate, Rack *parentRack)
            : juce::Thread("Faceplate Image Downloader"),
              url(urlToUse), item(itemToUpdate), rack(parentRack)
        {
            startThread();
        }

        /**
         * @brief Destructor for FaceplateImageDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~FaceplateImageDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the faceplate image and updates the UI on the message thread.
         */
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

        juce::URL url;  ///< The URL to download from
        GearItem *item; ///< The gear item to update
        Rack *rack;     ///< The rack to notify
    };

    // Create and start the download thread (it will delete itself when done)
    new FaceplateImageDownloader(imageUrl, item, this);
}

/**
 * @brief Fetches the knob image for a gear control.
 *
 * @param item The gear item containing the control
 * @param controlIndex The index of the control
 */
void Rack::fetchKnobImage(GearItem *item, int controlIndex)
{
    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
    {
        DBG("Cannot fetch knob image: invalid item or control index");
        return;
    }

    GearControl &control = item->controls.getReference(controlIndex);
    if (control.image.isEmpty())
    {
        DBG("Cannot fetch knob image: image path is empty");
        return;
    }

    DBG("Fetching knob image from " + control.image);

    // Construct the full URL if it's a relative path
    juce::String fullUrl = control.image;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = GearLibrary::getFullUrl(fullUrl);
            DBG("Resolved relative path to full URL: " + fullUrl);
        }
    }

    DBG("Full knob image URL: " + fullUrl);

    // Use JUCE's URL class to fetch the image asynchronously
    juce::URL imageUrl(fullUrl);

    // Create a new thread to download the image asynchronously
    /**
     * @brief Thread for downloading knob images.
     *
     * This struct handles the asynchronous download and processing of knob images
     * for gear controls, ensuring UI updates happen on the message thread.
     */
    struct KnobImageDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new KnobImageDownloader.
         *
         * @param urlToUse The URL to download the image from
         * @param itemToUpdate The gear item containing the control
         * @param controlIndexToUpdate The index of the control to update
         * @param parentRack The rack to notify when the image is loaded
         */
        KnobImageDownloader(juce::URL urlToUse, GearItem *itemToUpdate, int controlIndexToUpdate, Rack *parentRack)
            : juce::Thread("Knob Image Downloader"),
              url(urlToUse),
              item(itemToUpdate),
              controlIndex(controlIndexToUpdate),
              rack(parentRack),
              controlId(itemToUpdate->controls[controlIndexToUpdate].id),
              controlName(itemToUpdate->controls[controlIndexToUpdate].name)
        {
            startThread();
        }

        /**
         * @brief Destructor for KnobImageDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~KnobImageDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the knob image and updates the UI on the message thread.
         */
        void run() override
        {
            DBG("KnobImageDownloader thread started for control: " + controlName);

            // Try to download using the simple API - this is for older JUCE versions
            std::unique_ptr<juce::InputStream> inputStream = url.createInputStream(false);

            if (inputStream == nullptr || threadShouldExit())
            {
                DBG("Failed to create input stream for knob image: " + url.toString(true));
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
                DBG("Successfully loaded knob image with dimensions: " +
                    juce::String(downloadedImage.getWidth()) + "x" +
                    juce::String(downloadedImage.getHeight()));

                // Need to get back on the message thread to update the UI
                juce::MessageManager::callAsync([this, downloadedImage]()
                                                {
                    DBG("KnobImageDownloader callback started for control: " + controlName);
                    
                    // Validate item and control index are still valid
                    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
                    {
                        DBG("Item or control index is no longer valid for: " + controlName);
                        delete this;
                        return;
                    }

                    // Validate control ID matches
                    GearControl &control = item->controls.getReference(controlIndex);
                    if (control.id != controlId)
                    {
                        DBG("Control ID mismatch in callback. Expected: " + controlId + ", Got: " + control.id);
                        delete this;
                        return;
                    }

                    // Update the control's loaded image
                    control.loadedImage = downloadedImage;
                    DBG("Setting loaded image for control: " + controlName + " with ID: " + controlId);
                    
                    // Notify any slots that have this item to repaint
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot != nullptr && slot->getGearItem() == item)
                            {
                                slot->repaint();
                                break;
                            }
                        }
                    }
                    
                    delete this; });
            }
            else
            {
                DBG("Failed to load knob image for control: " + controlName);
                juce::MessageManager::callAsync([this]()
                                                { delete this; });
            }
        }

        juce::URL url;            ///< The URL to download from
        GearItem *item;           ///< The gear item containing the control
        int controlIndex;         ///< The index of the control to update
        Rack *rack;               ///< The rack to notify
        juce::String controlId;   ///< The ID of the control being updated
        juce::String controlName; ///< The name of the control being updated
    };

    // Create and start the download thread (it will delete itself when done)
    new KnobImageDownloader(imageUrl, item, controlIndex, this);
}

/**
 * @brief Fetches the fader image for a gear control.
 *
 * @param item The gear item containing the control
 * @param controlIndex The index of the control
 */
void Rack::fetchFaderImage(GearItem *item, int controlIndex)
{
    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
    {
        DBG("Invalid item or control index for fader image fetch");
        return;
    }

    const GearControl &control = item->controls[controlIndex];
    if (control.image.isEmpty())
    {
        DBG("No image path specified for fader control: " + control.name);
        return;
    }

    // Use GearLibrary::getFullUrl to resolve the image path
    juce::String fullUrl = GearLibrary::getFullUrl(control.image);

    DBG("Fetching fader image from URL: " + fullUrl);
    DBG("Original image path: " + control.image);
    DBG("Full URL after resolution: " + fullUrl);

    juce::URL imageUrl(fullUrl);

    /**
     * @brief Thread for downloading fader images.
     *
     * This struct handles the asynchronous download and processing of fader images
     * for gear controls, ensuring UI updates happen on the message thread.
     */
    struct FaderImageDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new FaderImageDownloader.
         *
         * @param urlToUse The URL to download the image from
         * @param itemToUpdate The gear item containing the control
         * @param controlIndexToUpdate The index of the control to update
         * @param parentRack The rack to notify when the image is loaded
         */
        FaderImageDownloader(juce::URL urlToUse, GearItem *itemToUpdate, int controlIndexToUpdate, Rack *parentRack)
            : juce::Thread("Fader Image Downloader"),
              url(urlToUse),
              item(itemToUpdate),
              controlIndex(controlIndexToUpdate),
              rack(parentRack),
              controlId(itemToUpdate->controls[controlIndexToUpdate].id),
              controlName(itemToUpdate->controls[controlIndexToUpdate].name)
        {
            startThread();
        }

        /**
         * @brief Destructor for FaderImageDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~FaderImageDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the fader image and updates the UI on the message thread.
         */
        void run() override
        {
            DBG("FaderImageDownloader thread started for control: " + controlName);

            // Try to download using the simple API - this is for older JUCE versions
            std::unique_ptr<juce::InputStream> inputStream = url.createInputStream(false);

            if (inputStream == nullptr || threadShouldExit())
            {
                DBG("Failed to create input stream for fader image: " + url.toString(true));
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
                DBG("Successfully loaded fader image with dimensions: " +
                    juce::String(downloadedImage.getWidth()) + "x" +
                    juce::String(downloadedImage.getHeight()));

                // Need to get back on the message thread to update the UI
                juce::MessageManager::callAsync([this, downloadedImage]()
                                                {
                    DBG("FaderImageDownloader callback started for control: " + controlName);
                    
                    // Validate item and control index are still valid
                    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
                    {
                        DBG("Item or control index is no longer valid for: " + controlName);
                        delete this;
                        return;
                    }

                    // Validate control ID matches
                    GearControl &control = item->controls.getReference(controlIndex);
                    if (control.id != controlId)
                    {
                        DBG("Control ID mismatch in callback. Expected: " + controlId + ", Got: " + control.id);
                        delete this;
                        return;
                    }

                    // Update the control's fader image
                    control.faderImage = downloadedImage;
                    DBG("Setting fader image for control: " + controlName + " with ID: " + controlId);
                    
                    // Notify any slots that have this item to repaint
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot != nullptr && slot->getGearItem() == item)
                            {
                                slot->repaint();
                                break;
                            }
                        }
                    }
                    
                    delete this; });
            }
            else
            {
                DBG("Failed to load fader image for control: " + controlName);
                juce::MessageManager::callAsync([this]()
                                                { delete this; });
            }
        }

        juce::URL url;            ///< The URL to download from
        GearItem *item;           ///< The gear item containing the control
        int controlIndex;         ///< The index of the control to update
        Rack *rack;               ///< The rack to notify
        juce::String controlId;   ///< The ID of the control being updated
        juce::String controlName; ///< The name of the control being updated
    };

    // Create and start the downloader
    new FaderImageDownloader(imageUrl, item, controlIndex, this);
}

/**
 * @brief Fetches the switch sprite sheet for a gear control.
 *
 * @param item The gear item containing the control
 * @param controlIndex The index of the control
 */
void Rack::fetchSwitchSpriteSheet(GearItem *item, int controlIndex)
{
    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
    {
        DBG("Cannot fetch switch sprite sheet: invalid item or control index");
        return;
    }

    GearControl &control = item->controls.getReference(controlIndex);
    if (control.image.isEmpty())
    {
        DBG("Cannot fetch switch sprite sheet: image path is empty");
        return;
    }

    DBG("Fetching switch sprite sheet from " + control.image);

    // Construct the full URL if it's a relative path
    juce::String fullUrl = control.image;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = GearLibrary::getFullUrl(fullUrl);
            DBG("Resolved relative path to full URL: " + fullUrl);
        }
    }

    DBG("Full switch sprite sheet URL: " + fullUrl);

    // Use JUCE's URL class to fetch the image asynchronously
    juce::URL imageUrl(fullUrl);

    // Create a new thread to download the image asynchronously
    /**
     * @brief Thread for downloading switch sprite sheets.
     *
     * This struct handles the asynchronous download and processing of switch sprite sheets
     * for gear controls, ensuring UI updates happen on the message thread.
     */
    struct SwitchSpriteSheetDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new SwitchSpriteSheetDownloader.
         *
         * @param urlToUse The URL to download the sprite sheet from
         * @param itemToUpdate The gear item containing the control
         * @param controlIndexToUpdate The index of the control to update
         * @param parentRack The rack to notify when the sprite sheet is loaded
         */
        SwitchSpriteSheetDownloader(juce::URL urlToUse, GearItem *itemToUpdate, int controlIndexToUpdate, Rack *parentRack)
            : juce::Thread("Switch Sprite Sheet Downloader"),
              url(urlToUse),
              item(itemToUpdate),
              controlIndex(controlIndexToUpdate),
              rack(parentRack),
              controlId(itemToUpdate->controls[controlIndexToUpdate].id),
              controlName(itemToUpdate->controls[controlIndexToUpdate].name)
        {
            startThread();
        }

        /**
         * @brief Destructor for SwitchSpriteSheetDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~SwitchSpriteSheetDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the switch sprite sheet and updates the UI on the message thread.
         */
        void run() override
        {
            DBG("SwitchSpriteSheetDownloader thread started for control: " + controlName);

            // Try to download using the simple API - this is for older JUCE versions
            std::unique_ptr<juce::InputStream> inputStream = url.createInputStream(false);

            if (inputStream == nullptr || threadShouldExit())
            {
                DBG("Failed to create input stream for switch sprite sheet: " + url.toString(true));
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
                DBG("Successfully loaded switch sprite sheet with dimensions: " +
                    juce::String(downloadedImage.getWidth()) + "x" +
                    juce::String(downloadedImage.getHeight()));

                // Need to get back on the message thread to update the UI
                juce::MessageManager::callAsync([this, downloadedImage]()
                                                {
                    DBG("SwitchSpriteSheetDownloader callback started for control: " + controlName);
                    
                    // Validate item and control index are still valid
                    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
                    {
                        DBG("Item or control index is no longer valid for: " + controlName);
                        delete this;
                        return;
                    }

                    // Validate control ID matches
                    GearControl &control = item->controls.getReference(controlIndex);
                    if (control.id != controlId)
                    {
                        DBG("Control ID mismatch in callback. Expected: " + controlId + ", Got: " + control.id);
                        delete this;
                        return;
                    }

                    // Update the control's sprite sheet
                    control.switchSpriteSheet = downloadedImage;
                    DBG("Setting sprite sheet for control: " + controlName + " with ID: " + controlId);
                    
                    // Notify any slots that have this item to repaint
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot != nullptr && slot->getGearItem() == item)
                            {
                                slot->repaint();
                                break;
                            }
                        }
                    }
                    
                    delete this; });
            }
            else
            {
                DBG("Failed to load switch sprite sheet for control: " + controlName);
                juce::MessageManager::callAsync([this]()
                                                { delete this; });
            }
        }

        juce::URL url;            ///< The URL to download from
        GearItem *item;           ///< The gear item containing the control
        int controlIndex;         ///< The index of the control to update
        Rack *rack;               ///< The rack to notify
        juce::String controlId;   ///< The ID of the control being updated
        juce::String controlName; ///< The name of the control being updated
    };

    // Create and start the download thread (it will delete itself when done)
    new SwitchSpriteSheetDownloader(imageUrl, item, controlIndex, this);
}

/**
 * @brief Fetches the button sprite sheet for a gear control.
 *
 * @param item The gear item containing the control
 * @param controlIndex The index of the control
 */
void Rack::fetchButtonSpriteSheet(GearItem *item, int controlIndex)
{
    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
    {
        DBG("Cannot fetch button sprite sheet: invalid item or control index");
        return;
    }

    GearControl &control = item->controls.getReference(controlIndex);
    if (control.image.isEmpty())
    {
        DBG("Cannot fetch button sprite sheet: image path is empty");
        return;
    }

    DBG("Fetching button sprite sheet from " + control.image);

    // Construct the full URL if it's a relative path
    juce::String fullUrl = control.image;
    if (!fullUrl.startsWith("http"))
    {
        // Check if the path is already a full path or needs the base URL
        if (fullUrl.startsWith("assets/") || !fullUrl.contains("/"))
        {
            fullUrl = GearLibrary::getFullUrl(fullUrl);
            DBG("Resolved relative path to full URL: " + fullUrl);
        }
    }

    DBG("Full button sprite sheet URL: " + fullUrl);

    // Use JUCE's URL class to fetch the image asynchronously
    juce::URL imageUrl(fullUrl);

    // Create a new thread to download the image asynchronously
    /**
     * @brief Thread for downloading button sprite sheets.
     *
     * This struct handles the asynchronous download and processing of button sprite sheets
     * for gear controls, ensuring UI updates happen on the message thread.
     */
    struct ButtonSpriteSheetDownloader : public juce::Thread
    {
        /**
         * @brief Constructs a new ButtonSpriteSheetDownloader.
         *
         * @param urlToUse The URL to download the sprite sheet from
         * @param itemToUpdate The gear item containing the control
         * @param controlIndexToUpdate The index of the control to update
         * @param parentRack The rack to notify when the sprite sheet is loaded
         */
        ButtonSpriteSheetDownloader(juce::URL urlToUse, GearItem *itemToUpdate, int controlIndexToUpdate, Rack *parentRack)
            : juce::Thread("Button Sprite Sheet Downloader"),
              url(urlToUse),
              item(itemToUpdate),
              controlIndex(controlIndexToUpdate),
              rack(parentRack),
              controlId(itemToUpdate->controls[controlIndexToUpdate].id),
              controlName(itemToUpdate->controls[controlIndexToUpdate].name)
        {
            startThread();
        }

        /**
         * @brief Destructor for ButtonSpriteSheetDownloader.
         *
         * Ensures the thread is properly stopped before destruction.
         */
        ~ButtonSpriteSheetDownloader() override
        {
            stopThread(2000);
        }

        /**
         * @brief Main thread execution function.
         *
         * Downloads the button sprite sheet and updates the UI on the message thread.
         */
        void run() override
        {
            DBG("ButtonSpriteSheetDownloader thread started for control: " + controlName);

            // Try to download using the simple API - this is for older JUCE versions
            std::unique_ptr<juce::InputStream> inputStream = url.createInputStream(false);

            if (inputStream == nullptr || threadShouldExit())
            {
                DBG("Failed to create input stream for button sprite sheet: " + url.toString(true));
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
                DBG("Successfully loaded button sprite sheet with dimensions: " +
                    juce::String(downloadedImage.getWidth()) + "x" +
                    juce::String(downloadedImage.getHeight()));

                // Need to get back on the message thread to update the UI
                juce::MessageManager::callAsync([this, downloadedImage]()
                                                {
                    DBG("ButtonSpriteSheetDownloader callback started for control: " + controlName);
                    
                    // Validate item and control index are still valid
                    if (item == nullptr || controlIndex < 0 || controlIndex >= item->controls.size())
                    {
                        DBG("Item or control index is no longer valid for: " + controlName);
                        delete this;
                        return;
                    }

                    // Validate control ID matches
                    GearControl &control = item->controls.getReference(controlIndex);
                    if (control.id != controlId)
                    {
                        DBG("Control ID mismatch in callback. Expected: " + controlId + ", Got: " + control.id);
                        delete this;
                        return;
                    }

                    // Update the control's sprite sheet
                    control.buttonSpriteSheet = downloadedImage;
                    DBG("Setting sprite sheet for control: " + controlName + " with ID: " + controlId);
                    
                    // Notify any slots that have this item to repaint
                    if (rack != nullptr)
                    {
                        for (int i = 0; i < rack->getNumSlots(); ++i)
                        {
                            RackSlot *slot = rack->getSlot(i);
                            if (slot != nullptr && slot->getGearItem() == item)
                            {
                                slot->repaint();
                                break;
                            }
                        }
                    }
                    
                    delete this; });
            }
            else
            {
                DBG("Failed to load button sprite sheet for control: " + controlName);
                juce::MessageManager::callAsync([this]()
                                                { delete this; });
            }
        }

        juce::URL url;            ///< The URL to download from
        GearItem *item;           ///< The gear item containing the control
        int controlIndex;         ///< The index of the control to update
        Rack *rack;               ///< The rack to notify
        juce::String controlId;   ///< The ID of the control being updated
        juce::String controlName; ///< The name of the control being updated
    };

    // Create and start the download thread (it will delete itself when done)
    new ButtonSpriteSheetDownloader(imageUrl, item, controlIndex, this);
}

/**
 * @brief Creates a new instance of a gear item in a slot.
 *
 * @param slotIndex The index of the slot to create the instance in
 */
void Rack::createInstance(int slotIndex)
{
    if (auto *slot = getSlot(slotIndex))
    {
        slot->createInstance();
    }
}

/**
 * @brief Resets a slot to its source gear item.
 *
 * @param slotIndex The index of the slot to reset
 */
void Rack::resetToSource(int slotIndex)
{
    if (auto *slot = getSlot(slotIndex))
    {
        slot->resetToSource();
    }
}

/**
 * @brief Checks if a slot contains an instance.
 *
 * @param slotIndex The index of the slot to check
 * @return true if the slot contains an instance
 */
bool Rack::isInstance(int slotIndex) const
{
    if (auto *slot = getSlot(slotIndex))
    {
        return slot->isInstance();
    }
    return false;
}

/**
 * @brief Gets the instance ID for a slot.
 *
 * @param slotIndex The index of the slot
 * @return The instance ID as a string
 */
juce::String Rack::getInstanceId(int slotIndex) const
{
    if (auto *slot = getSlot(slotIndex))
    {
        return slot->getInstanceId();
    }
    return juce::String();
}

/**
 * @brief Resets all instances in the rack to their source gear items.
 */
void Rack::resetAllInstances()
{
    for (int i = 0; i < slots.size(); ++i)
    {
        if (auto *slot = getSlot(i))
        {
            if (slot->isInstance())
            {
                slot->resetToSource();
            }
        }
    }
}