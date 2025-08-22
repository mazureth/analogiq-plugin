// INetworkFetcher.h
#pragma once

#include <JuceHeader.h>

class INetworkFetcher
{
public:
    virtual ~INetworkFetcher() = default;

    /** Performs a blocking fetch of the given URL and returns its contents.
        @param url The JUCE URL to fetch.
        @param success Output flag indicating whether the fetch succeeded.
        @return The content returned by the URL as a String, or an empty String on failure.
    */
    virtual juce::String fetchJsonBlocking(const juce::URL &url, bool &success) = 0;

    /** Performs a blocking fetch of binary data from the given URL.
        @param url The JUCE URL to fetch.
        @param success Output flag indicating whether the fetch succeeded.
        @return The binary content returned by the URL as a MemoryBlock, or an empty MemoryBlock on failure.
    */
    virtual juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) = 0;

    /**
     * @brief Returns a reference to a dummy network fetcher (Null Object Pattern).
     *
     * This can be used for default-constructed GearItems or in cases where a real fetcher is not available.
     */
    static INetworkFetcher &getDummy();
};