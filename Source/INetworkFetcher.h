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
};