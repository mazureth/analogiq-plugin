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
        Knob,
        Button,
        Fader
    };

    GearControl(Type typeParam, const juce::String &nameParam, const juce::Rectangle<float> &positionParam)
        : type(typeParam), name(nameParam), position(positionParam), value(0.0f) {}

    Type type;
    juce::String name;
    juce::Rectangle<float> position;
    float value;
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
    juce::Array<GearControl> controls;

    bool loadImage();
    void saveToJSON(juce::File destinationFile);
    static GearItem loadFromJSON(juce::File sourceFile);
};