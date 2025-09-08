#include <JuceHeader.h>
#include "TestFixture.h"
#include "../../Source/NetworkFetcher.h"
#include "../../Source/INetworkFetcher.h"

/**
 * @class NetworkFetcherTests
 * @brief Comprehensive tests for the NetworkFetcher class.
 * 
 * These tests cover the NetworkFetcher implementation including:
 * - JSON fetching functionality
 * - Binary data fetching functionality  
 * - Error handling for invalid URLs
 * - Success and failure scenarios
 * - Dummy implementation testing
 */
class NetworkFetcherTests : public juce::UnitTest
{
public:
    NetworkFetcherTests() : juce::UnitTest("NetworkFetcherTests", "NetworkFetcherTests") {}

    void runTest() override
    {
        beginTest("JSON Fetching - Invalid URLs");
        testJsonFetchingInvalidUrls();

        beginTest("Binary Fetching - Invalid URLs");
        testBinaryFetchingInvalidUrls();

        beginTest("NetworkFetcher Basic Functionality");
        testBasicFunctionality();

        beginTest("Dummy NetworkFetcher Implementation");
        testDummyImplementation();

        beginTest("Error Handling and Edge Cases");
        testErrorHandling();

        beginTest("URL Parameter Handling");
        testUrlParameterHandling();
    }

private:
    void testJsonFetchingInvalidUrls()
    {
        NetworkFetcher fetcher;
        bool success = true;

        // Test with completely invalid URL
        juce::URL invalidUrl("invalid://not.a.real.url");
        juce::String result = fetcher.fetchJsonBlocking(invalidUrl, success);

        expect(!success, "Should fail for invalid URL");
        expect(result.isEmpty(), "Should return empty string for invalid URL");

        // Test with malformed URL
        juce::URL malformedUrl("ht tp://malformed url.com");
        success = true;
        result = fetcher.fetchJsonBlocking(malformedUrl, success);

        expect(!success, "Should fail for malformed URL");
        expect(result.isEmpty(), "Should return empty string for malformed URL");

        // Test with non-existent domain
        juce::URL nonExistentUrl("https://thisdoesnotexist12345.nonexistentdomain");
        success = true;
        result = fetcher.fetchJsonBlocking(nonExistentUrl, success);

        expect(!success, "Should fail for non-existent domain");
        expect(result.isEmpty(), "Should return empty string for non-existent domain");
    }

    void testBinaryFetchingInvalidUrls()
    {
        NetworkFetcher fetcher;
        bool success = true;

        // Test with completely invalid URL
        juce::URL invalidUrl("invalid://not.a.real.url");
        juce::MemoryBlock result = fetcher.fetchBinaryBlocking(invalidUrl, success);

        expect(!success, "Should fail for invalid URL");
        expect(result.getSize() == 0, "Should return empty MemoryBlock for invalid URL");

        // Test with malformed URL
        juce::URL malformedUrl("ht tp://malformed url.com");
        success = true;
        result = fetcher.fetchBinaryBlocking(malformedUrl, success);

        expect(!success, "Should fail for malformed URL");
        expect(result.getSize() == 0, "Should return empty MemoryBlock for malformed URL");

        // Test with non-existent domain
        juce::URL nonExistentUrl("https://thisdoesnotexist12345.nonexistentdomain");
        success = true;
        result = fetcher.fetchBinaryBlocking(nonExistentUrl, success);

        expect(!success, "Should fail for non-existent domain");
        expect(result.getSize() == 0, "Should return empty MemoryBlock for non-existent domain");
    }

    void testBasicFunctionality()
    {
        NetworkFetcher fetcher;

        // Test that the methods exist and can be called without crashing
        bool success = false;
        juce::URL testUrl("http://example.com");

        // These will likely fail due to no network in test environment,
        // but we're testing that the methods execute without crashing
        juce::String jsonResult = fetcher.fetchJsonBlocking(testUrl, success);
        // Success will likely be false, but method should not crash

        success = false;
        juce::MemoryBlock binaryResult = fetcher.fetchBinaryBlocking(testUrl, success);
        // Success will likely be false, but method should not crash

        // Test with file:// URL (local test data)
        // Create a temporary file for testing
        juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("network_test.txt");
        tempFile.replaceWithText("test content for network fetcher");

        juce::URL fileUrl = juce::URL(tempFile);
        success = false;
        juce::String fileContent = fetcher.fetchJsonBlocking(fileUrl, success);

        if (success)
        {
            expect(fileContent.contains("test content"), "Should read file content successfully");
        }

        // Test binary fetch with same file
        success = false;
        juce::MemoryBlock fileData = fetcher.fetchBinaryBlocking(fileUrl, success);

        if (success)
        {
            expect(fileData.getSize() > 0, "Should read binary file data");
        }

        // Clean up
        tempFile.deleteFile();
    }

    void testDummyImplementation()
    {
        INetworkFetcher& dummy = INetworkFetcher::getDummy();

        // Test that dummy always fails
        bool success = true;
        juce::URL testUrl("http://any.url.com");

        juce::String jsonResult = dummy.fetchJsonBlocking(testUrl, success);
        expect(!success, "Dummy should always fail for JSON");
        expect(jsonResult.isEmpty(), "Dummy should return empty string");

        success = true;
        juce::MemoryBlock binaryResult = dummy.fetchBinaryBlocking(testUrl, success);
        expect(!success, "Dummy should always fail for binary");
        expect(binaryResult.getSize() == 0, "Dummy should return empty MemoryBlock");

        // Test multiple calls to ensure consistent behavior
        success = true;
        juce::String jsonResult2 = dummy.fetchJsonBlocking(testUrl, success);
        expect(!success, "Dummy should consistently fail");
        expect(jsonResult2.isEmpty(), "Dummy should consistently return empty");

        // Test that getDummy returns the same instance
        INetworkFetcher& dummy2 = INetworkFetcher::getDummy();
        expect(&dummy == &dummy2, "getDummy should return same instance (singleton)");
    }

    void testErrorHandling()
    {
        NetworkFetcher fetcher;

        // Test with various problematic URLs
        juce::Array<juce::String> problematicUrls = {
            "",                                    // Empty URL
            "not-a-url",                          // Not a URL at all
            "ftp://unsupported.protocol.com",     // Unsupported protocol
            "https://",                           // Incomplete URL
            "https:// space in url.com",          // URL with spaces
            "https://localhost:99999",            // Invalid port
        };

        for (const auto& urlString : problematicUrls)
        {
            juce::URL problematicUrl(urlString);
            bool success = true;

            // Test JSON fetch
            juce::String jsonResult = fetcher.fetchJsonBlocking(problematicUrl, success);
            expect(!success, "Should fail for problematic URL: " + urlString);
            expect(jsonResult.isEmpty(), "Should return empty string for problematic URL: " + urlString);

            // Test binary fetch
            success = true;
            juce::MemoryBlock binaryResult = fetcher.fetchBinaryBlocking(problematicUrl, success);
            expect(!success, "Should fail for binary fetch with problematic URL: " + urlString);
            expect(binaryResult.getSize() == 0, "Should return empty MemoryBlock for problematic URL: " + urlString);
        }
    }

    void testUrlParameterHandling()
    {
        NetworkFetcher fetcher;

        // Test that URL parameters are handled properly (even if requests fail)
        juce::URL urlWithParams("https://example.com/api?param1=value1&param2=value2");
        
        bool success = true;
        juce::String result = fetcher.fetchJsonBlocking(urlWithParams, success);
        
        // The request will likely fail, but the method should handle the URL parameters
        // without crashing (success will be false in test environment)

        // Test URL with special characters
        juce::URL urlWithSpecialChars("https://example.com/path with spaces");
        success = true;
        result = fetcher.fetchJsonBlocking(urlWithSpecialChars, success);
        
        // Again, likely to fail but should not crash

        // Test URL with encoded characters
        juce::URL urlWithEncoding("https://example.com/path%20with%20encoding");
        success = true;
        result = fetcher.fetchJsonBlocking(urlWithEncoding, success);
        
        // Should handle encoded URLs without crashing
    }
};

static NetworkFetcherTests networkFetcherTests;