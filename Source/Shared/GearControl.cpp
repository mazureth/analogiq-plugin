#include "GearControl.h"

GearControl::GearControl()
    : type(ControlType::Button)
    , position(0.0f, 0.0f, 50.0f, 50.0f)
    , currentValue(0.0f)
    , initialValue(0.0f)
    , minValue(0.0f)
    , maxValue(1.0f)
    , stepSize(0.01f)
    , orientation(Orientation::Vertical)
    , currentIndex(0)
    , isMomentary(false)
    , length(100.0f)
    , startAngle(0.0f)
    , endAngle(270.0f)
    , currentStepIndex(0)
{
    initializeDefaults();
}

GearControl::GearControl(ControlType type, const juce::Rectangle<float>& pos, float initialValue)
    : type(type)
    , position(pos)
    , currentValue(initialValue)
    , initialValue(initialValue)
    , minValue(0.0f)
    , maxValue(1.0f)
    , stepSize(0.01f)
    , orientation(Orientation::Vertical)
    , currentIndex(0)
    , isMomentary(false)
    , length(100.0f)
    , startAngle(0.0f)
    , endAngle(270.0f)
    , currentStepIndex(0)
{
    initializeDefaults();
}

GearControl::~GearControl()
{
    clearImages();
}

void GearControl::initializeDefaults()
{
    // Initialize based on control type
    switch (type)
    {
        case ControlType::Switch:
            options.add("Off");
            options.add("On");
            currentIndex = 0;
            break;
            
        case ControlType::Button:
            isMomentary = false;
            break;
            
        case ControlType::Fader:
            length = position.getHeight();
            break;
            
        case ControlType::Knob:
            steps.add(0.0f);
            steps.add(0.5f);
            steps.add(1.0f);
            currentStepIndex = 0;
            break;
    }
}

void GearControl::resetToDefault()
{
    currentValue = initialValue;
    currentIndex = 0;
    currentStepIndex = 0;
}

bool GearControl::isValidValue(float value) const
{
    return value >= minValue && value <= maxValue;
}

float GearControl::normalizeValue(float value) const
{
    if (maxValue == minValue)
        return 0.0f;
    
    return (value - minValue) / (maxValue - minValue);
}

void GearControl::setValue(float newValue)
{
    if (isValidValue(newValue))
    {
        currentValue = newValue;
        
        // Update indices for discrete controls
        switch (type)
        {
            case ControlType::Switch:
                if (options.size() > 0)
                {
                    float normalized = normalizeValue(newValue);
                    currentIndex = static_cast<int>(normalized * (options.size() - 1));
                    currentIndex = juce::jlimit(0, options.size() - 1, currentIndex);
                }
                break;
                
            case ControlType::Knob:
                if (steps.size() > 0)
                {
                    // Find closest step
                    float minDistance = std::numeric_limits<float>::max();
                    for (int i = 0; i < steps.size(); ++i)
                    {
                        float distance = std::abs(steps[i] - newValue);
                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            currentStepIndex = i;
                        }
                    }
                }
                break;
        }
    }
}

void GearControl::setIndex(int newIndex)
{
    switch (type)
    {
        case ControlType::Switch:
            if (newIndex >= 0 && newIndex < options.size())
            {
                currentIndex = newIndex;
                if (options.size() > 1)
                {
                    float normalized = static_cast<float>(newIndex) / (options.size() - 1);
                    currentValue = minValue + normalized * (maxValue - minValue);
                }
            }
            break;
            
        case ControlType::Button:
            if (newIndex >= 0 && newIndex < buttonFrames.size())
            {
                currentIndex = newIndex;
                if (buttonFrames.size() > 1)
                {
                    float normalized = static_cast<float>(newIndex) / (buttonFrames.size() - 1);
                    currentValue = minValue + normalized * (maxValue - minValue);
                }
            }
            break;
            
        case ControlType::Knob:
            if (newIndex >= 0 && newIndex < steps.size())
            {
                currentStepIndex = newIndex;
                currentValue = steps[newIndex];
            }
            break;
    }
}

void GearControl::setSwitchSpriteSheet(const juce::Image& image)
{
    switchSpriteSheet = image;
}

void GearControl::setButtonSpriteSheet(const juce::Image& image)
{
    buttonSpriteSheet = image;
}

void GearControl::setFaderImage(const juce::Image& image)
{
    faderImage = image;
}

void GearControl::setKnobImage(const juce::Image& image)
{
    loadedImage = image;
}

void GearControl::clearImages()
{
    switchSpriteSheet = juce::Image();
    buttonSpriteSheet = juce::Image();
    faderImage = juce::Image();
    loadedImage = juce::Image();
}
