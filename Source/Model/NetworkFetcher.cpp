#include "NetworkFetcher.h"
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Enhanced network operations with progress tracking
juce::String NetworkFetcher::fetchJsonBlocking(const juce::URL &url, bool &success)
{
    return performHttpRequest(url, success);
}

juce::MemoryBlock NetworkFetcher::fetchBinaryBlocking(const juce::URL &url, bool &success)
{
    return performBinaryRequest(url, success);
}

// New methods for remote gear library operations
juce::String NetworkFetcher::fetchRemoteGearLibrary(const juce::URL &url, bool &success, ProgressCallback progressCallback)
{
    return performHttpRequest(url, success, progressCallback);
}

juce::MemoryBlock NetworkFetcher::fetchGearImage(const juce::URL &url, bool &success, ProgressCallback progressCallback)
{
    return performBinaryRequest(url, success, progressCallback);
}

juce::MemoryBlock NetworkFetcher::fetchGearSchema(const juce::URL &url, bool &success, ProgressCallback progressCallback)
{
    return performBinaryRequest(url, success, progressCallback);
}

// Network status and health checking
bool NetworkFetcher::isNetworkAvailable() const
{
    // Test with a simple, reliable endpoint
    juce::URL testUrl("https://httpbin.org/get");
    return testConnection(testUrl, 3000);
}

bool NetworkFetcher::testConnection(const juce::URL &url, int timeoutMs) const
{
    try
    {
        auto stream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                .withConnectionTimeoutMs(timeoutMs)
                                                .withNumRedirectsToFollow(1));
        return stream != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

juce::String NetworkFetcher::getLastErrorMessage() const
{
    return lastErrorMessage;
}

int NetworkFetcher::getLastHttpCode() const
{
    return lastHttpCode;
}

// Configuration methods
void NetworkFetcher::setConnectionTimeout(int timeoutMs)
{
    connectionTimeoutMs = timeoutMs;
}

void NetworkFetcher::setMaxRedirects(int redirects)
{
    this->maxRedirects = redirects;
}

void NetworkFetcher::setRetryAttempts(int attempts)
{
    retryAttempts = attempts;
}

void NetworkFetcher::setRetryDelay(int delayMs)
{
    retryDelayMs = delayMs;
}

// Private helper methods
juce::String NetworkFetcher::performHttpRequest(const juce::URL &url, bool &success, ProgressCallback progressCallback)
{
    success = false;
    lastErrorMessage.clear();
    lastHttpCode = 0;

    for (int attempt = 0; attempt <= retryAttempts; ++attempt)
    {
        try
        {

            auto stream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                    .withConnectionTimeoutMs(connectionTimeoutMs)
                                                    .withNumRedirectsToFollow(maxRedirects));

            if (stream == nullptr)
            {
                lastErrorMessage = "Failed to create input stream";
                if (attempt < retryAttempts && shouldRetryRequest(0, lastErrorMessage))
                {
                    waitForRetry();
                    continue;
                }
                return juce::String();
            }

            // Get content length if available for progress tracking
            juce::int64 totalBytes = stream->getTotalLength();
            if (totalBytes < 0)
                totalBytes = 0;

            juce::String result;
            juce::int64 bytesRead = 0;
            juce::int64 lastProgressUpdate = 0;

            // Read data in chunks for progress tracking
            const int chunkSize = 8192;
            juce::MemoryBlock buffer(chunkSize);

            while (!stream->isExhausted())
            {
                int bytesThisChunk = stream->read(buffer.getData(), chunkSize);
                if (bytesThisChunk <= 0)
                    break;

                result += juce::String::fromUTF8(static_cast<const char *>(buffer.getData()), bytesThisChunk);
                bytesRead += bytesThisChunk;

                // Update progress every 1KB or when complete
                if (progressCallback && (bytesRead - lastProgressUpdate >= 1024 || stream->isExhausted()))
                {
                    int percentage = totalBytes > 0 ? static_cast<int>((bytesRead * 100) / totalBytes) : 0;
                    progressCallback(percentage, bytesRead, totalBytes);
                    lastProgressUpdate = bytesRead;
                }
            }

            success = true;
            lastHttpCode = 200; // Assume success if we got data
            return result;
        }
        catch (const std::exception &e)
        {
            lastErrorMessage = "Exception: " + juce::String(e.what());
        }
        catch (...)
        {
            lastErrorMessage = "Unknown error occurred";
        }

        if (attempt < retryAttempts && shouldRetryRequest(lastHttpCode, lastErrorMessage))
        {
            waitForRetry();
        }
    }

    return juce::String();
}

juce::MemoryBlock NetworkFetcher::performBinaryRequest(const juce::URL &url, bool &success, ProgressCallback progressCallback)
{
    success = false;
    lastErrorMessage.clear();
    lastHttpCode = 0;

    for (int attempt = 0; attempt <= retryAttempts; ++attempt)
    {
        try
        {

            auto stream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                                    .withConnectionTimeoutMs(connectionTimeoutMs)
                                                    .withNumRedirectsToFollow(maxRedirects));

            if (stream == nullptr)
            {
                lastErrorMessage = "Failed to create input stream";
                if (attempt < retryAttempts && shouldRetryRequest(0, lastErrorMessage))
                {
                    waitForRetry();
                    continue;
                }
                return juce::MemoryBlock();
            }

            juce::MemoryBlock block;
            juce::int64 totalBytes = stream->getTotalLength();
            if (totalBytes < 0)
                totalBytes = 0;

            if (stream->readIntoMemoryBlock(block) > 0)
            {
                success = true;
                lastHttpCode = 200;

                if (progressCallback)
                {
                    progressCallback(100, block.getSize(), totalBytes);
                }

                return block;
            }

            lastErrorMessage = "Failed to read binary data";
        }
        catch (const std::exception &e)
        {
            lastErrorMessage = "Exception: " + juce::String(e.what());
        }
        catch (...)
        {
            lastErrorMessage = "Unknown binary error occurred";
        }

        if (attempt < retryAttempts && shouldRetryRequest(lastHttpCode, lastErrorMessage))
        {
            waitForRetry();
        }
    }

    return juce::MemoryBlock();
}

void NetworkFetcher::updateProgress(ProgressCallback callback, juce::int64 current, juce::int64 total)
{
    if (callback)
    {
        int percentage = total > 0 ? static_cast<int>((current * 100) / total) : 0;
        callback(percentage, current, total);
    }
}

bool NetworkFetcher::shouldRetryRequest(int httpCode, const juce::String &error) const
{
    // Retry on network errors, timeouts, and certain HTTP status codes
    if (httpCode == 0)
        return true; // Network error
    if (httpCode >= 500)
        return true; // Server errors
    if (httpCode == 408)
        return true; // Request timeout
    if (httpCode == 429)
        return true; // Too many requests

    // Retry on specific error messages
    if (error.containsIgnoreCase("timeout"))
        return true;
    if (error.containsIgnoreCase("connection"))
        return true;
    if (error.containsIgnoreCase("network"))
        return true;

    return false;
}

void NetworkFetcher::waitForRetry() const
{
    juce::Thread::sleep(retryDelayMs);
}
