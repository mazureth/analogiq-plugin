/**
 * @file NetworkFetcher.h
 * @brief Header file for the NetworkFetcher class.
 *
 * This file defines the NetworkFetcher class, which provides the concrete
 * implementation of the INetworkFetcher interface for performing network
 * operations using JUCE's URL and InputStream classes.
 */

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