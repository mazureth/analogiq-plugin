// RealNetworkFetcher.h
#pragma once

#include "INetworkFetcher.h"

/** Real implementation of INetworkFetcher that performs network calls using JUCE. */
class NetworkFetcher : public INetworkFetcher
{
public:
    juce::String fetchJsonBlocking(const juce::URL &url, bool &success) override;
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) override;
};