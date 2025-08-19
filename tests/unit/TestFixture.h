#pragma once

#include <JuceHeader.h>
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"

/**
 * @brief Enhanced test fixture with mock isolation and cleanup.
 *
 * This fixture ensures proper JUCE initialization and mock state isolation
 * between tests to prevent test interference.
 */
class TestFixture
{
public:
    TestFixture()
    {
        // Initialize JUCE's message thread and component system
        juceInit = std::make_unique<juce::ScopedJuceInitialiser_GUI>();

        // Disable JUCE's internal image caching to prevent leaks
        juce::ImageCache::setCacheTimeout(0);
        juce::ImageCache::releaseUnusedImages();

        // Reset all singleton mocks to ensure clean state
        resetAllMocks();
    }

    ~TestFixture()
    {
        // Comprehensive JUCE cleanup to prevent assertions and memory leaks

        // Clear the default LookAndFeel to prevent destruction order issues
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

        // Process any pending messages
        if (auto *mm = juce::MessageManager::getInstance())
        {
            // Process all pending messages to complete any async operations
            // Use a brief sleep to allow pending operations to complete
            juce::Thread::sleep(10);

            // Clear any pending broadcast messages
            mm->deliverBroadcastMessage(juce::String());
        }

        // Reset all singleton mocks to clean up any remaining state
        resetAllMocks();

        // Force cleanup of any remaining JUCE image caches and string pools
        cleanupJuceResources();

        // Additional aggressive cleanup per test class
        juce::ImageCache::releaseUnusedImages();
        juce::Thread::sleep(1);
        juce::ImageCache::releaseUnusedImages();

        // Clean up JUCE's message thread (this must be last)
        juceInit.reset();
    }

    /**
     * @brief Reset all singleton mocks to ensure test isolation.
     */
    static void resetAllMocks()
    {
        ConcreteMockNetworkFetcher::getInstance().reset();
        ConcreteMockFileSystem::getInstance().reset();
    }

    /**
     * @brief Verify that mocks are in a clean state.
     *
     * @return true if mocks are clean, false otherwise
     */
    static bool verifyMocksClean()
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        // Check if network fetcher is clean
        bool fetcherClean = mockFetcher.isClean();

        // Check if any files were accessed
        bool fileSystemClean = mockFileSystem.getAccessedPaths().empty();

        return fetcherClean && fileSystemClean;
    }

    /**
     * @brief Get a summary of mock states for debugging.
     *
     * @return String containing mock state information
     */
    static juce::String getMockStateSummary()
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        juce::String summary = "Mock State Summary:\n";
        summary += "Network Fetcher: " + juce::String(mockFetcher.wasUrlRequested("") ? "Has requests" : "Clean") + "\n";
        summary += "File System: " + mockFileSystem.getState() + "\n";

        return summary;
    }

    /**
     * @brief Cleanup remaining JUCE resources to prevent memory leaks.
     *
     * This method helps clean up any lingering JUCE objects that might
     * trigger the leak detector at program shutdown.
     */
    static void cleanupJuceResources()
    {
        // Clear any cached image format readers/writers
        // JUCE internally caches these and they can trigger leak detection

        // Force a garbage collection cycle by creating and destroying
        // temporary objects to trigger cleanup of internal caches
        {
            // Create a temporary image to trigger any lazy initialization cleanup
            juce::Image tempImage(juce::Image::RGB, 1, 1, true);
            tempImage = juce::Image(); // Clear it immediately
        }

        {
            // Create a temporary string array to trigger string pool cleanup
            juce::StringArray tempArray;
            tempArray.add("temp");
            tempArray.clear();
        }

        // Give JUCE a moment to clean up internal resources
        juce::Thread::sleep(5);
    }

private:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juceInit;
};