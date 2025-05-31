#pragma once

#include <JuceHeader.h>

enum class GearType
{
    Series500,
    Rack19Inch,
    UserCreated,
    Other
};

enum class GearCategory
{
    EQ,
    Compressor,
    Preamp,
    Other
};

class GearControl
{
public:
    enum class Type
    {
        Button,
        Fader,
        Switch,
        Knob
    };

    // Add default constructor
    GearControl()
        : type(Type::Button),
          name(""),
          id(""),
          position(0.0f, 0.0f, 0.0f, 0.0f),
          value(0.0f),
          initialValue(0.0f),
          currentIndex(0),
          orientation("vertical"),
          startAngle(0.0f),
          endAngle(360.0f),
          currentStepIndex(0)
    {
    }

    GearControl(Type typeParam, const juce::String &nameParam, const juce::Rectangle<float> &positionParam)
        : type(typeParam), name(nameParam), position(positionParam), value(0.0f), initialValue(0.0f) {}

    // Add copy constructor
    GearControl(const GearControl &other)
        : type(other.type),
          name(other.name),
          id(other.id),
          position(other.position),
          value(other.value),
          initialValue(other.value),
          options(other.options),
          currentIndex(other.currentIndex),
          orientation(other.orientation),
          image(other.image),
          startAngle(other.startAngle),
          endAngle(other.endAngle),
          steps(other.steps),
          currentStepIndex(other.currentStepIndex),
          loadedImage(other.loadedImage)
    {
        DBG("GearControl copy constructor called for control: " + name + " with ID: " + id);
    }

    // Add assignment operator
    GearControl &operator=(const GearControl &other)
    {
        if (this != &other)
        {
            type = other.type;
            name = other.name;
            id = other.id;
            position = other.position;
            value = other.value;
            initialValue = other.initialValue;
            options = other.options;
            currentIndex = other.currentIndex;
            orientation = other.orientation;
            image = other.image;
            startAngle = other.startAngle;
            endAngle = other.endAngle;
            steps = other.steps;
            currentStepIndex = other.currentStepIndex;
            loadedImage = other.loadedImage;
            DBG("GearControl assignment operator called for control: " + name + " with ID: " + id);
        }
        return *this;
    }

    Type type;
    juce::String name;
    juce::String id; // Unique identifier for the control
    juce::Rectangle<float> position;
    float value;
    float initialValue; // Store the original value from schema

    // Additional properties for switches
    juce::StringArray options;
    int currentIndex = 0;
    juce::String orientation = "vertical"; // "horizontal" or "vertical"
    juce::String image;                    // URI to the sprite sheet image

    // Additional properties for knobs
    float startAngle = 0.0f;  // Starting angle in degrees (0-360)
    float endAngle = 360.0f;  // Ending angle in degrees (0-360)
    juce::Array<float> steps; // Array of rotation degrees for stepped knobs
    int currentStepIndex = 0; // Index of current step (only used with steps)
    juce::Image loadedImage;  // The actual loaded knob image
};

class GearItem
{
public:
    GearItem() = default;

    GearItem(const juce::String &unitIdParam,
             const juce::String &nameParam,
             const juce::String &manufacturerParam,
             const juce::String &categoryParam,
             const juce::String &versionParam,
             const juce::String &schemaPathParam,
             const juce::String &thumbnailImageParam,
             const juce::StringArray &tagsParam,
             GearType typeParam = GearType::Other,
             GearCategory gearCategoryParam = GearCategory::Other,
             int slotSizeParam = 1,
             const juce::Array<GearControl> &controlsParam = {})
        : unitId(unitIdParam),
          name(nameParam),
          manufacturer(manufacturerParam),
          type(typeParam),
          category(gearCategoryParam),
          slotSize(slotSizeParam),
          version(versionParam),
          schemaPath(schemaPathParam),
          thumbnailImage(thumbnailImageParam),
          categoryString(categoryParam),
          tags(tagsParam),
          controls(controlsParam)
    {
        // Map category string to enum (for backward compatibility)
        if (categoryString == "equalizer" || categoryString == "eq")
            category = GearCategory::EQ;
        else if (categoryString == "compressor")
            category = GearCategory::Compressor;
        else if (categoryString == "preamp")
            category = GearCategory::Preamp;
        else
            category = GearCategory::Other;

        // Try to determine type from tags
        if (tags.contains("500 series"))
            type = GearType::Series500;
        else if (tags.contains("rack") || tags.contains("19 inch"))
            type = GearType::Rack19Inch;
    }

    // Original constructor for backward compatibility
    GearItem(const juce::String &nameParam,
             const juce::String &manufacturerParam,
             GearType typeParam,
             GearCategory categoryParam,
             int slotSizeParam,
             const juce::String &imageUrlParam,
             const juce::Array<GearControl> &controlsParam)
        : unitId(nameParam.toLowerCase().replaceCharacter(' ', '-')),
          name(nameParam),
          manufacturer(manufacturerParam),
          type(typeParam),
          category(categoryParam),
          slotSize(slotSizeParam),
          version("1.0.0"),
          schemaPath(""),
          thumbnailImage(imageUrlParam),
          categoryString(""),
          tags(),
          controls(controlsParam)
    {
        // Map category enum to string
        switch (category)
        {
        case GearCategory::EQ:
            categoryString = "equalizer";
            break;
        case GearCategory::Compressor:
            categoryString = "compressor";
            break;
        case GearCategory::Preamp:
            categoryString = "preamp";
            break;
        case GearCategory::Other:
            categoryString = "other";
            break;
        }
    }

    // New fields from updated schema
    juce::String unitId;
    juce::String name;
    juce::String manufacturer;
    GearType type;
    GearCategory category;
    int slotSize;
    juce::String version;
    juce::String schemaPath;
    juce::String thumbnailImage;
    juce::String categoryString;
    juce::StringArray tags;
    juce::Image image;
    juce::String faceplateImagePath;
    juce::Image faceplateImage;
    juce::Array<GearControl> controls;

    bool loadImage();
    void saveToJSON(juce::File destinationFile);
    static GearItem loadFromJSON(juce::File sourceFile);

private:
    // Helper method to create a placeholder image
    bool createPlaceholderImage();
};