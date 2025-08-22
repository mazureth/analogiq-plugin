#pragma once

#include <JuceHeader.h>
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"

// Ensure JUCE_UNIT_TESTS is defined for this header
#ifndef JUCE_UNIT_TESTS
#define JUCE_UNIT_TESTS 1
#endif

/**
 * @brief Helper class for verifying mock states and ensuring test isolation.
 *
 * This class provides utilities to verify that mocks are in expected states
 * and to help debug test interference issues.
 */
class MockStateVerifier
{
public:
    /**
     * @brief Verify that all mocks are in a clean state.
     *
     * This method checks if mocks have any configured responses, errors, or accessed paths.
     * When mocks are not clean, a detailed state summary is logged for debugging.
     *
     * @param testName Name of the test for debugging purposes
     * @return true if all mocks are clean, false otherwise
     */
    static bool verifyCleanState(const juce::String &testName = "")
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        bool isClean = true;
        juce::String issues;

        // Check file system state
        auto accessedPaths = mockFileSystem.getAccessedPaths();
        if (!accessedPaths.empty())
        {
            isClean = false;
            issues += "File system has files: " + juce::String(accessedPaths.size()) + " accessed paths\n";
        }

        // Check network fetcher state
        if (!mockFetcher.isClean())
        {
            isClean = false;
            issues += "Network fetcher has responses:\n";
            issues += "  Responses: " + juce::String(static_cast<int>(mockFetcher.getResponseCount())) + "\n";
            issues += "  Binary Responses: " + juce::String(static_cast<int>(mockFetcher.getBinaryResponseCount())) + "\n";
            issues += "  Errors: " + juce::String(static_cast<int>(mockFetcher.getErrorCount())) + "\n";
            issues += "  Requested URLs: " + juce::String(static_cast<int>(mockFetcher.getRequestedUrlCount())) + "\n";
        }

        if (!isClean)
        {
            juce::Logger::writeToLog("MockStateVerifier: " + testName + " - State detected:\n" + issues);
        }

        return isClean;
    }

    /**
     * @brief Reset all mocks and verify they are clean.
     *
     * @param testName Name of the test for debugging purposes
     * @return true if reset was successful and mocks are clean
     */
    static bool resetAndVerify(const juce::String &testName = "")
    {
        ConcreteMockNetworkFetcher::getInstance().reset();
        ConcreteMockFileSystem::getInstance().reset();

        return verifyCleanState(testName);
    }

    /**
     * @brief Get a detailed report of mock states.
     *
     * @return String containing detailed mock state information
     */
    static juce::String getDetailedStateReport()
    {
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();

        juce::String report = "=== Mock State Summary ===\n";

        // File system state
        report += "File System:\n";
        report += "  Accessed Paths: " + juce::String(static_cast<int>(mockFileSystem.getAccessedPaths().size())) + "\n";
        report += "  State: " + mockFileSystem.getState() + "\n";

        // Network fetcher state (limited visibility)
        report += "Network Fetcher:\n";
        report += "  Has any requests: " + juce::String(mockFetcher.wasUrlRequested("") ? "Yes" : "No") + "\n";

        return report;
    }

    /**
     * @brief Assert that mocks are clean, fail test if not.
     *
     * @param testName Name of the test for debugging purposes
     * @param unitTest The unit test object
     */
    static void assertCleanState(const juce::String &testName, juce::UnitTest &unitTest)
    {
        bool isClean = verifyCleanState(testName);
        unitTest.expect(isClean, "Mocks should have no state for " + testName);

        if (!isClean)
        {
            juce::Logger::writeToLog("MockStateVerifier: " + testName + " - Mocks have state:\n" + getDetailedStateReport());
        }
    }

    /**
     * @brief Setup mocks for a test and verify clean state.
     *
     * @param testName Name of the test
     * @param unitTest The unit test object
     */
    static void setupForTest(const juce::String &testName, juce::UnitTest &unitTest)
    {
        // Reset all mocks
        resetAndVerify(testName + "_setup");

        // Verify clean state
        assertCleanState(testName + "_setup", unitTest);
    }

    /**
     * @brief Cleanup after a test and verify clean state.
     *
     * @param testName Name of the test
     * @param unitTest The unit test object
     */
    static void cleanupAfterTest(const juce::String &testName, juce::UnitTest &unitTest)
    {
        // Reset all mocks
        resetAndVerify(testName + "_cleanup");

        // Verify clean state
        assertCleanState(testName + "_cleanup", unitTest);
    }
};