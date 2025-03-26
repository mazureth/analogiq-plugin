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

    GearControl(Type type, const juce::String& name, const juce::Rectangle<float>& position)
        : type(type), name(name), position(position), value(0.0f) {}

    Type type;
    juce::String name;
    juce::Rectangle<float> position;
    float value;
};

class GearItem
{
public:
    GearItem() = default;
    
    GearItem(const juce::String& name, 
             const juce::String& manufacturer,
             GearType type,
             GearCategory category,
             int slotSize,
             const juce::String& imageUrl,
             const juce::Array<GearControl>& controls)
        : name(name),
          manufacturer(manufacturer),
          type(type),
          category(category),
          slotSize(slotSize),
          imageUrl(imageUrl),
          controls(controls) {}

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