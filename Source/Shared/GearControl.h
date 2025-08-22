#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

/**
 * Sophisticated control system supporting multiple control types.
 * Provides value management, validation, and control-specific rendering data.
 */
class GearControl
{
public:
    enum class ControlType
    {
        Button,
        Fader,
        Switch,
        Knob
    };

    enum class Orientation
    {
        Vertical,
        Horizontal
    };

    // Switch-specific structures
    struct SwitchOptionFrame
    {
        juce::Rectangle<float> position;
        juce::Rectangle<float> size;
        float value;
        juce::String label;
    };

    // Constructor
    GearControl();
    GearControl(ControlType type, const juce::Rectangle<float>& pos, float initialValue = 0.0f);
    
    // Destructor with proper cleanup
    ~GearControl();

    // Control type and properties
    ControlType type;
    juce::Rectangle<float> position;
    float currentValue;
    float initialValue;
    float minValue;
    float maxValue;
    float stepSize;
    Orientation orientation;

    // Switch-specific properties
    juce::Array<SwitchOptionFrame> switchFrames;
    juce::Array<juce::String> options;
    int currentIndex;
    juce::Image switchSpriteSheet;

    // Button-specific properties
    bool isMomentary;
    juce::Array<SwitchOptionFrame> buttonFrames;
    juce::Image buttonSpriteSheet;

    // Fader-specific properties
    float length;
    juce::Image faderImage;

    // Knob-specific properties
    float startAngle;
    float endAngle;
    juce::Array<float> steps;
    int currentStepIndex;
    juce::Image loadedImage;

    // Utility methods
    void resetToDefault();
    bool isValidValue(float value) const;
    float normalizeValue(float value) const;
    void setValue(float newValue);
    void setIndex(int newIndex);
    
    // Image management
    void setSwitchSpriteSheet(const juce::Image& image);
    void setButtonSpriteSheet(const juce::Image& image);
    void setFaderImage(const juce::Image& image);
    void setKnobImage(const juce::Image& image);
    
    // Cleanup
    void clearImages();

private:
    void initializeDefaults();
};
