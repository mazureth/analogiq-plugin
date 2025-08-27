#pragma once

#include "../Shared/INetworkFetcher.h"
#include <juce_core/juce_core.h>
#include <functional>

/**
 * Enhanced NetworkFetcher with progress tracking, better error handling, and remote gear library operations.
 * Provides robust HTTP operations for remote data fetching with proper error handling, timeout configuration,
 * and progress tracking for large downloads.
 */
class NetworkFetcher : public INetworkFetcher
{
public:
    // Progress callback type for tracking download progress
    using ProgressCallback = std::function<void(int percentage, juce::int64 bytesDownloaded, juce::int64 totalBytes)>;

    // Error callback type for detailed error reporting
    using ErrorCallback = std::function<void(const juce::String &error, int httpCode)>;

    NetworkFetcher() = default;
    ~NetworkFetcher() override = default;

    // Enhanced network operations with progress tracking
    juce::String fetchJsonBlocking(const juce::URL &url, bool &success) override;
    juce::MemoryBlock fetchBinaryBlocking(const juce::URL &url, bool &success) override;

    // New methods for remote gear library operations
    juce::String fetchRemoteGearLibrary(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) override;
    juce::MemoryBlock fetchGearImage(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) override;
    juce::MemoryBlock fetchGearSchema(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr) override;

    // Network status and health checking
    bool isNetworkAvailable() const override;
    bool testConnection(const juce::URL &url, int timeoutMs = 5000) const override;
    juce::String getLastErrorMessage() const override;
    int getLastHttpCode() const override;

    // Configuration methods
    void setConnectionTimeout(int timeoutMs);
    void setMaxRedirects(int maxRedirects);
    void setRetryAttempts(int attempts);
    void setRetryDelay(int delayMs);

private:
    // Enhanced network configuration
    static const int DEFAULT_CONNECTION_TIMEOUT_MS = 10000; // 10 seconds
    static const int DEFAULT_MAX_REDIRECTS = 5;
    static const int DEFAULT_RETRY_ATTEMPTS = 3;
    static const int DEFAULT_RETRY_DELAY_MS = 1000; // 1 second

    int connectionTimeoutMs = DEFAULT_CONNECTION_TIMEOUT_MS;
    int maxRedirects = DEFAULT_MAX_REDIRECTS;
    int retryAttempts = DEFAULT_RETRY_ATTEMPTS;
    int retryDelayMs = DEFAULT_RETRY_DELAY_MS;

    // Error tracking
    mutable juce::String lastErrorMessage;
    mutable int lastHttpCode = 0;

    // Helper methods
    juce::String performHttpRequest(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr);
    juce::MemoryBlock performBinaryRequest(const juce::URL &url, bool &success, ProgressCallback progressCallback = nullptr);
    void updateProgress(ProgressCallback callback, juce::int64 current, juce::int64 total);
    bool shouldRetryRequest(int httpCode, const juce::String &error) const;
    void waitForRetry() const;
};
