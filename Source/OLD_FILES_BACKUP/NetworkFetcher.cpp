/**
 * @file NetworkFetcher.cpp
 * @brief Implementation of the NetworkFetcher class for network operations.
 *
 * This file provides the concrete implementation of the INetworkFetcher interface
 * using JUCE's URL and InputStream classes for network operations. It includes
 * methods for fetching JSON data and binary data from URLs with proper error
 * handling and timeout configuration. The file also includes a DummyNetworkFetcher
 * implementation for the Null Object Pattern used in testing.
 */

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
    juce::String fetchJsonBlocking(const juce::URL &url, bool &success) override
    {
        success = false;
        return "";
    }
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) override
    {
        success = false;
        return juce::MemoryBlock();
    }
};

INetworkFetcher &INetworkFetcher::getDummy()
{
    static DummyNetworkFetcher instance;
    return instance;
}
