#pragma once

#include <JuceHeader.h>
#include "CacheManager.h"

class TestFixture
{
public:
    TestFixture()
    {
        // Initialize JUCE's message thread and component system
        juceInit = std::make_unique<juce::ScopedJuceInitialiser_GUI>();

        // Clear the cache at the start to ensure clean state
        CacheManager::getInstance().clearCache();
    }

    ~TestFixture()
    {
        // Process any pending messages
        if (auto *mm = juce::MessageManager::getInstance())
        {
            // Clear any pending broadcast messages
            mm->deliverBroadcastMessage(juce::String());
        }

        // Clear the cache at the end to prevent test artifacts from affecting the running app
        CacheManager::getInstance().clearCache();

        // Clean up JUCE's message thread
        juceInit.reset();
    }

private:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juceInit;
};