#pragma once

#include "INetworkFetcher.h"
#include <map>
#include <set>

/**
 * @brief Mock implementation of INetworkFetcher for testing purposes.
 *
 * This class provides a singleton instance that can be used across test cases
 * to mock network responses and verify network calls.
 */
class MockNetworkFetcher : public INetworkFetcher
{
public:
    /**
     * @brief Get the singleton instance of the mock network fetcher.
     *
     * @return Reference to the singleton instance
     */
    static MockNetworkFetcher &getInstance()
    {
        static MockNetworkFetcher instance;
        return instance;
    }

    /**
     * @brief Set a mock response for a specific URL.
     *
     * @param url The URL to mock
     * @param response The response to return for this URL
     */
    void setResponse(const juce::String &url, const juce::String &response)
    {
        responses[url] = response;
    }

    /**
     * @brief Set a URL to return an error.
     *
     * @param url The URL that should return an error
     */
    void setError(const juce::String &url)
    {
        errors.insert(url);
    }

    /**
     * @brief Implementation of INetworkFetcher::fetchJsonBlocking.
     *
     * @param url The URL to fetch
     * @param success Output parameter indicating if the fetch was successful
     * @return The response string, or empty string on error
     */
    juce::String fetchJsonBlocking(const juce::URL &url, bool &success) override
    {
        auto urlString = url.toString(false);
        requestedUrls.insert(urlString);

        if (errors.find(urlString) != errors.end())
        {
            success = false;
            return "";
        }

        auto it = responses.find(urlString);
        if (it != responses.end())
        {
            success = true;
            return it->second;
        }

        success = false;
        return "";
    }

    /**
     * @brief Check if a URL was requested.
     *
     * @param url The URL to check
     * @return true if the URL was requested, false otherwise
     */
    bool wasUrlRequested(const juce::String &url) const
    {
        return requestedUrls.find(url) != requestedUrls.end();
    }

    /**
     * @brief Reset the mock state.
     *
     * Clears all responses, errors, and requested URLs.
     */
    void reset()
    {
        responses.clear();
        errors.clear();
        requestedUrls.clear();
    }

private:
    MockNetworkFetcher() = default; // Private constructor for singleton
    std::map<juce::String, juce::String> responses;
    std::set<juce::String> errors;
    std::set<juce::String> requestedUrls;
};