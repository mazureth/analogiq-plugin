/**
 * @file CrashLogger.h
 * @brief Utility class for crash-safe logging to file.
 *
 * This class provides a simple, crash-safe logging mechanism that writes
 * to a file in the user's home directory. It's designed to help diagnose
 * crashes by providing a persistent log that survives application crashes.
 */

#pragma once

#include <juce_core/juce_core.h>

class CrashLogger
{
public:
    /**
     * @brief Get the singleton instance of CrashLogger
     */
    static CrashLogger &getInstance();

    /**
     * @brief Log a message with timestamp
     * @param message The message to log
     */
    void log(const juce::String &message);

    /**
     * @brief Log a message with a specific category
     * @param category The category (e.g., "CONSTRUCTOR", "DESTRUCTOR", "CRASH")
     * @param message The message to log
     */
    void log(const juce::String &category, const juce::String &message);

    /**
     * @brief Flush any pending log entries to disk
     */
    void flush();

    /**
     * @brief Get the path to the log file
     */
    juce::String getLogFilePath() const;

private:
    CrashLogger();
    ~CrashLogger();

    juce::File logFile;
    juce::CriticalSection logLock;

    juce::String getTimestamp();
    void writeToFile(const juce::String &message);
};
