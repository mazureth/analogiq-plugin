#pragma once

#include <juce_core/juce_core.h>
#include <functional>

/**
 * Enhanced abstract interface for network operations.
 * Provides platform-independent abstraction for HTTP requests, JSON data fetching, binary data retrieval,
 * and specialized remote gear library operations with progress tracking and error handling.
 */
class INetworkFetcher
{
public:
    // Progress callback type for tracking download progress
    using ProgressCallback = std::function<void(int percentage, juce::int64 bytesDownloaded, juce::int64 totalBytes)>;

    // Error callback type for detailed error reporting
    using ErrorCallback = std::function<void(const juce::String &error, int httpCode)>;

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

    /**
     * Fetches remote gear library data with progress tracking.
     * @param url The URL to fetch from
     * @param success Reference to boolean that will be set to true on success, false on failure
     * @param progressCallback Optional callback for progress updates
     * @return The fetched gear library content as a string, or empty string on failure
     */
    virtual juce::String fetchRemoteGearLibrary(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Fetches gear image data with progress tracking.
     * @param url The URL to fetch from
     * @param success Reference to boolean that will be set to true on success, false on failure
     * @param progressCallback Optional callback for progress updates
     * @return The fetched image data as a MemoryBlock, or empty MemoryBlock on failure
     */
    virtual juce::MemoryBlock fetchGearImage(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Fetches gear schema data with progress tracking.
     * @param url The URL to fetch from
     * @param success Reference to boolean that will be set to true on success, false on failure
     * @param progressCallback Optional callback for progress updates
     * @return The fetched schema data as a MemoryBlock, or empty MemoryBlock on failure
     */
    virtual juce::MemoryBlock fetchGearSchema(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) = 0;

    /**
     * Checks if network is available.
     * @return true if network is accessible, false otherwise
     */
    virtual bool isNetworkAvailable() const = 0;

    /**
     * Tests connection to a specific URL.
     * @param url The URL to test
     * @param timeoutMs Timeout in milliseconds
     * @return true if connection successful, false otherwise
     */
    virtual bool testConnection(const juce::URL &url, int timeoutMs = 5000) const = 0;

    /**
     * Gets the last error message from network operations.
     * @return The last error message, or empty string if no error
     */
    virtual juce::String getLastErrorMessage() const = 0;

    /**
     * Gets the last HTTP response code from network operations.
     * @return The last HTTP response code, or 0 if no response
     */
    virtual int getLastHttpCode() const = 0;

    // Null Object Pattern implementation
    static INetworkFetcher &getDummy();
};

// Enhanced dummy implementation for testing and fallback
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

    juce::String fetchRemoteGearLibrary(const juce::URL &, bool &success, ProgressCallback = nullptr) override
    {
        success = false;
        return "";
    }

    juce::MemoryBlock fetchGearImage(const juce::URL &, bool &success, ProgressCallback = nullptr) override
    {
        success = false;
        return juce::MemoryBlock();
    }

    juce::MemoryBlock fetchGearSchema(const juce::URL &, bool &success, ProgressCallback = nullptr) override
    {
        success = false;
        return juce::MemoryBlock();
    }

    bool isNetworkAvailable() const override
    {
        return false;
    }

    bool testConnection(const juce::URL &, int) const override
    {
        return false;
    }

    juce::String getLastErrorMessage() const override
    {
        return "Dummy network fetcher - no network operations available";
    }

    int getLastHttpCode() const override
    {
        return 0;
    }
};

inline INetworkFetcher &INetworkFetcher::getDummy()
{
    static DummyNetworkFetcher dummy;
    return dummy;
}
