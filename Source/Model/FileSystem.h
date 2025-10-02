#pragma once

#include "../Shared/IFileSystem.h"
#include <juce_core/juce_core.h>

/**
 * Concrete implementation of IFileSystem using JUCE's File class.
 * Provides comprehensive file operations, path utilities, and cross-platform support.
 */
class FileSystem : public IFileSystem
{
public:
    FileSystem() = default;
    ~FileSystem() override = default;

    // Core file operations
    bool createDirectory(const juce::String& path) override;
    bool writeFile(const juce::String& path, const juce::String& content) override;
    bool writeFile(const juce::String& path, const juce::MemoryBlock& data) override;
    juce::String readFile(const juce::String& path) override;
    juce::MemoryBlock readBinaryFile(const juce::String& path) override;
    bool fileExists(const juce::String& path) override;
    bool directoryExists(const juce::String& path) override;

    // File system management
    juce::StringArray getFiles(const juce::String& directory) override;
    juce::StringArray getDirectories(const juce::String& directory) override;
    bool deleteFile(const juce::String& path) override;
    bool deleteDirectory(const juce::String& path, bool recursive = false) override;
    bool moveFile(const juce::String& sourcePath, const juce::String& destPath) override;

    // File metadata
    juce::int64 getFileSize(const juce::String& path) override;
    juce::Time getFileTime(const juce::String& path) override;

    // Path utilities
    juce::String getFileName(const juce::String& path) override;
    juce::String getParentDirectory(const juce::String& path) override;
    juce::String joinPath(const juce::String& path1, const juce::String& path2) override;
    bool isAbsolutePath(const juce::String& path) override;
    juce::String normalizePath(const juce::String& path) override;

    // Cache directory management
    juce::String getCacheRootDirectory() override;

private:
    static const juce::String ANALOGIQ_CACHE_DIR;
};
