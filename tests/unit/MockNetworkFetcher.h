#pragma once

#include "INetworkFetcher.h"
#include <map>
#include <set>

/**
 * @brief Abstract base class for network fetcher interface.
 *
 * This class defines the interface for network operations.
 * The concrete implementation is provided by ConcreteMockNetworkFetcher.
 */
class MockNetworkFetcher : public INetworkFetcher
{
public:
    virtual ~MockNetworkFetcher() = default;
};

/**
 * @brief Concrete implementation of MockNetworkFetcher for testing purposes.
 *
 * This class provides the actual implementation of network operations
 * and includes functionality for mocking responses and verifying network calls.
 */
class ConcreteMockNetworkFetcher : public MockNetworkFetcher
{
public:
    /**
     * @brief Get the singleton instance of the mock network fetcher.
     *
     * @return Reference to the singleton instance
     */
    static ConcreteMockNetworkFetcher &getInstance()
    {
        static ConcreteMockNetworkFetcher instance;
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
     * @brief Set a mock binary response for a specific URL.
     *
     * @param url The URL to mock
     * @param response The binary response to return for this URL
     */
    void setBinaryResponse(const juce::String &url, const juce::MemoryBlock &response)
    {
        binaryResponses[url] = response;
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
        binaryResponses.clear();
        errors.clear();
        requestedUrls.clear();
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
     * @brief Implementation of INetworkFetcher::fetchBinaryBlocking.
     *
     * @param url The URL to fetch
     * @param success Output parameter indicating if the fetch was successful
     * @return The binary response data, or empty MemoryBlock on error
     */
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) override
    {
        auto urlString = url.toString(false);
        requestedUrls.insert(urlString);

        if (errors.find(urlString) != errors.end())
        {
            success = false;
            return juce::MemoryBlock();
        }

        auto it = binaryResponses.find(urlString);
        if (it != binaryResponses.end())
        {
            success = true;
            return it->second;
        }

        success = false;
        return juce::MemoryBlock();
    }

private:
    ConcreteMockNetworkFetcher() = default; // Private constructor for singleton
    std::map<juce::String, juce::String> responses;
    std::map<juce::String, juce::MemoryBlock> binaryResponses;
    std::set<juce::String> errors;
    std::set<juce::String> requestedUrls;
};