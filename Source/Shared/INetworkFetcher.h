#pragma once

#include <juce_core/juce_core.h>

/**
 * Abstract interface for network operations.
 * Provides platform-independent abstraction for HTTP requests, JSON data fetching, and binary data retrieval.
 */
class INetworkFetcher
{
public:
    virtual ~INetworkFetcher() = default;

    /**
     * Fetches JSON data from the specified URL.
     * @param url The URL to fetch from
     * @param success Reference to boolean that will be set to true on success, false on failure
     * @return The fetched JSON content as a string, or empty string on failure
     */
    virtual juce::String fetchJsonBlocking(const juce::URL &url, bool &success) = 0;

    /**
     * Fetches binary data from the specified URL.
     * @param url The URL to fetch from
     * @param success Reference to boolean that will be set to true on success, false on failure
     * @return The fetched binary content as a MemoryBlock, or empty MemoryBlock on failure
     */
    virtual juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) = 0;

    // Null Object Pattern implementation
    static INetworkFetcher &getDummy();
};

// Dummy implementation for testing and fallback
class DummyNetworkFetcher : public INetworkFetcher
{
public:
    juce::String fetchJsonBlocking(const juce::URL &, bool &success) override
    {
        success = false;
        return "";
    }
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &, bool &success) override
    {
        success = false;
        return juce::MemoryBlock();
    }
};

inline INetworkFetcher &INetworkFetcher::getDummy()
{
    static DummyNetworkFetcher dummy;
    return dummy;
}
