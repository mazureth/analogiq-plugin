#pragma once

#include <JuceHeader.h>

class TestFixture
{
public:
    TestFixture()
    {
        // Initialize JUCE's message thread and component system
        juceInit = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
    }

    ~TestFixture()
    {
        // Clean up JUCE's message thread
        juceInit.reset();
    }

private:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juceInit;
};