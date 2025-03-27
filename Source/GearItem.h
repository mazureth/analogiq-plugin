#pragma once

#include <JuceHeader.h>

enum class GearType
{
    Series500,
    Rack19Inch,
    UserCreated
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

    GearControl(Type typeParam, const juce::String& nameParam, const juce::Rectangle<float>& positionParam)
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
    
    GearItem(const juce::String& nameParam, 
             const juce::String& manufacturerParam,
             GearType typeParam,
             GearCategory categoryParam,
             int slotSizeParam,
             const juce::String& imageUrlParam,
             const juce::Array<GearControl>& controlsParam)
        : name(nameParam),
          manufacturer(manufacturerParam),
          type(typeParam),
          category(categoryParam),
          slotSize(slotSizeParam),
          imageUrl(imageUrlParam),
          controls(controlsParam) {}

    juce::String name;
    juce::String manufacturer;
    GearType type;
    GearCategory category;
    int slotSize;
    juce::String imageUrl;
    juce::Image image;
    juce::Array<GearControl> controls;

    bool loadImage();
    void saveToJSON(juce::File destinationFile);
    static GearItem loadFromJSON(juce::File sourceFile);
}; 