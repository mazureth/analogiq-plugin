// RealNetworkFetcher.cpp
#include "NetworkFetcher.h"
#include <JuceHeader.h>

juce::String NetworkFetcher::fetchJsonBlocking(const juce::URL &url, bool &success)
{
    success = false;

    std::unique_ptr<juce::InputStream> stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(10000)
            .withNumRedirectsToFollow(5));

    if (stream != nullptr)
    {
        success = true;
        return stream->readEntireStreamAsString();
    }

    return {};
}
