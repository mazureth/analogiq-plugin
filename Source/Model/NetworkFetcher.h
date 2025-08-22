#pragma once

#include "../Shared/INetworkFetcher.h"
#include <juce_core/juce_core.h>

/**
 * Concrete implementation of INetworkFetcher using JUCE's URL and InputStream classes.
 * Provides robust HTTP operations for remote data fetching with proper error handling and timeout configuration.
 */
class NetworkFetcher : public INetworkFetcher
{
public:
    NetworkFetcher() = default;
    ~NetworkFetcher() override = default;

    // Network operations
    juce::String fetchJsonBlocking(const juce::URL& url, bool& success) override;
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL& url, bool& success) override;

private:
    static const int CONNECTION_TIMEOUT_MS = 10000; // 10 seconds
    static const int MAX_REDIRECTS = 5;
};
