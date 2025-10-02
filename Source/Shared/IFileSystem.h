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
    virtual bool createDirectory(const juce::String &path) = 0;
    virtual bool writeFile(const juce::String &path, const juce::String &content) = 0;
    virtual bool writeFile(const juce::String &path, const juce::MemoryBlock &data) = 0;
    virtual juce::String readFile(const juce::String &path) = 0;
    virtual juce::MemoryBlock readBinaryFile(const juce::String &path) = 0;
    virtual bool fileExists(const juce::String &path) = 0;
    virtual bool directoryExists(const juce::String &path) = 0;

    // File system management
    virtual juce::StringArray getFiles(const juce::String &directory) = 0;
    virtual juce::StringArray getDirectories(const juce::String &directory) = 0;
    virtual bool deleteFile(const juce::String &path) = 0;
    virtual bool deleteDirectory(const juce::String &path, bool recursive = false) = 0;
    virtual bool moveFile(const juce::String &sourcePath, const juce::String &destPath) = 0;

    // File metadata
    virtual juce::int64 getFileSize(const juce::String &path) = 0;
    virtual juce::Time getFileTime(const juce::String &path) = 0;

    // Path utilities
    virtual juce::String getFileName(const juce::String &path) = 0;
    virtual juce::String getParentDirectory(const juce::String &path) = 0;
    virtual juce::String joinPath(const juce::String &path1, const juce::String &path2) = 0;
    virtual bool isAbsolutePath(const juce::String &path) = 0;
    virtual juce::String normalizePath(const juce::String &path) = 0;

    // Cache directory management
    virtual juce::String getCacheRootDirectory() = 0;

    // Null Object Pattern implementation
    static IFileSystem &getDummy();
};

// Dummy implementation for testing and fallback
class DummyFileSystem : public IFileSystem
{
public:
    bool createDirectory(const juce::String &) override { return false; }
    bool writeFile(const juce::String &, const juce::String &) override { return false; }
    bool writeFile(const juce::String &, const juce::MemoryBlock &) override { return false; }
    juce::String readFile(const juce::String &) override { return ""; }
    juce::MemoryBlock readBinaryFile(const juce::String &) override { return juce::MemoryBlock(); }
    bool fileExists(const juce::String &) override { return false; }
    bool directoryExists(const juce::String &) override { return false; }
    juce::StringArray getFiles(const juce::String &) override { return juce::StringArray(); }
    juce::StringArray getDirectories(const juce::String &) override { return juce::StringArray(); }
    bool deleteFile(const juce::String &) override { return false; }
    bool deleteDirectory(const juce::String &, bool) override { return false; }
    bool moveFile(const juce::String &, const juce::String &) override { return false; }
    juce::int64 getFileSize(const juce::String &) override { return 0; }
    juce::Time getFileTime(const juce::String &) override { return juce::Time(); }
    juce::String getFileName(const juce::String &) override { return ""; }
    juce::String getParentDirectory(const juce::String &) override { return ""; }
    juce::String joinPath(const juce::String &path1, const juce::String &path2) override { return path1 + "/" + path2; }
    bool isAbsolutePath(const juce::String &) override { return false; }
    juce::String normalizePath(const juce::String &path) override { return path; }
    juce::String getCacheRootDirectory() override { return "/tmp/analogiq_cache"; }
};

inline IFileSystem &IFileSystem::getDummy()
{
    static DummyFileSystem dummy;
    return dummy;
}
