#include "NetworkFetcher.h"
#include <juce_core/juce_core.h>

// Network operations
juce::String NetworkFetcher::fetchJsonBlocking(const juce::URL& url, bool& success)
{
    success = false;
    
    try
    {
        auto stream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                         .withConnectionTimeoutMs(CONNECTION_TIMEOUT_MS)
                                         .withNumRedirectsToFollow(MAX_REDIRECTS));
        
        if (stream == nullptr)
            return juce::String();
        
        success = true;
        return stream->readEntireStreamAsString();
    }
    catch (...)
    {
        success = false;
        return juce::String();
    }
}

juce::MemoryBlock NetworkFetcher::fetchBinaryBlocking(const juce::URL& url, bool& success)
{
    success = false;
    
    try
    {
        auto stream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                         .withConnectionTimeoutMs(CONNECTION_TIMEOUT_MS)
                                         .withNumRedirectsToFollow(MAX_REDIRECTS));
        
        if (stream == nullptr)
            return juce::MemoryBlock();
        
        juce::MemoryBlock block;
        if (stream->readIntoMemoryBlock(block) > 0)
        {
            success = true;
            return block;
        }
        
        success = false;
        return juce::MemoryBlock();
    }
    catch (...)
    {
        success = false;
        return juce::MemoryBlock();
    }
}
