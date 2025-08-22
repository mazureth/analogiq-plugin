#pragma once

#include <juce_core/juce_core.h>

/**
 * Abstract interface for file system operations.
 * Provides platform-independent abstraction for file I/O, directory management, and path utilities.
 */
class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    // Core file operations
    virtual bool createDirectory(const juce::String& path) = 0;
    virtual bool writeFile(const juce::String& path, const juce::String& content) = 0;
    virtual bool writeFile(const juce::String& path, const juce::MemoryBlock& data) = 0;
    virtual juce::String readFile(const juce::String& path) = 0;
    virtual juce::MemoryBlock readBinaryFile(const juce::String& path) = 0;
    virtual bool fileExists(const juce::String& path) = 0;
    virtual bool directoryExists(const juce::String& path) = 0;

    // File system management
    virtual juce::StringArray getFiles(const juce::String& directory) = 0;
    virtual juce::StringArray getDirectories(const juce::String& directory) = 0;
    virtual bool deleteFile(const juce::String& path) = 0;
    virtual bool deleteDirectory(const juce::String& path, bool recursive = false) = 0;
    virtual bool moveFile(const juce::String& sourcePath, const juce::String& destPath) = 0;

    // File metadata
    virtual juce::int64 getFileSize(const juce::String& path) = 0;
    virtual juce::Time getFileTime(const juce::String& path) = 0;

    // Path utilities
    virtual juce::String getFileName(const juce::String& path) = 0;
    virtual juce::String getParentDirectory(const juce::String& path) = 0;
    virtual juce::String joinPath(const juce::String& path1, const juce::String& path2) = 0;
    virtual bool isAbsolutePath(const juce::String& path) = 0;
    virtual juce::String normalizePath(const juce::String& path) = 0;

    // Cache directory management
    virtual juce::String getCacheRootDirectory() = 0;

    // Null Object Pattern implementation
    static IFileSystem& getDummy();
};
