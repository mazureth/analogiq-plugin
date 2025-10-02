/**
 * @file CrashLogger.cpp
 * @brief Implementation of CrashLogger utility class.
 */

#include "CrashLogger.h"
#include <juce_core/juce_core.h>

CrashLogger &CrashLogger::getInstance()
{
    static CrashLogger instance;
    return instance;
}

CrashLogger::CrashLogger()
{
    // Create log file in user's home directory
    auto homeDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    logFile = homeDir.getChildFile("AnalogIQ_CrashLog.txt");

    // Write initial log entry
    writeToFile("=== AnalogIQ Crash Logger Started ===");
    writeToFile("Log file: " + logFile.getFullPathName());
}

CrashLogger::~CrashLogger()
{
    writeToFile("=== AnalogIQ Crash Logger Shutdown ===");
    flush();
}

void CrashLogger::log(const juce::String &message)
{
    log("INFO", message);
}

void CrashLogger::log(const juce::String &category, const juce::String &message)
{
    juce::String logEntry = "[" + getTimestamp() + "] [" + category + "] " + message;

    const juce::ScopedLock lock(logLock);
    writeToFile(logEntry);
}

void CrashLogger::flush()
{
    const juce::ScopedLock lock(logLock);
    // File operations are automatically flushed by JUCE
}

juce::String CrashLogger::getLogFilePath() const
{
    return logFile.getFullPathName();
}

juce::String CrashLogger::getTimestamp()
{
    auto now = juce::Time::getCurrentTime();
    return now.formatted("%Y-%m-%d %H:%M:%S.%f");
}

void CrashLogger::writeToFile(const juce::String &message)
{
    try
    {
        juce::FileOutputStream stream(logFile);
        if (stream.openedOk())
        {
            stream.writeText(message + juce::newLine, false, false, nullptr);
        }
    }
    catch (...)
    {
        // Silently fail to avoid recursive logging issues
    }
}
