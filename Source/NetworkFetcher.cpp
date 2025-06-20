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

juce::MemoryBlock NetworkFetcher::fetchBinaryBlocking(const juce::URL &url, bool &success)
{
    success = false;
    juce::MemoryBlock data;

    auto inputStream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                 .withConnectionTimeoutMs(10000)
                                                 .withNumRedirectsToFollow(5));

    if (inputStream != nullptr)
    {
        inputStream->readIntoMemoryBlock(data);
        success = data.getSize() > 0;
    }

    return data;
}

// Null Object Pattern: DummyNetworkFetcher implementation
class DummyNetworkFetcher : public INetworkFetcher
{
public:
    juce::String fetchJsonBlocking(const juce::URL &, bool &success) override
    {
        success = false;
        juce::Logger::writeToLog("[DummyNetworkFetcher] fetchJsonBlocking called. Returning empty string.");
        return {};
    }
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &, bool &success) override
    {
        success = false;
        juce::Logger::writeToLog("[DummyNetworkFetcher] fetchBinaryBlocking called. Returning empty block.");
        return {};
    }
};

INetworkFetcher &INetworkFetcher::getDummy()
{
    static DummyNetworkFetcher instance;
    return instance;
}
