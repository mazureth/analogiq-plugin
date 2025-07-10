// FileSystem.h
#pragma once

#include "IFileSystem.h"

/**
 * @brief Real implementation of IFileSystem that performs file operations using JUCE.
 *
 * This class provides the production implementation of file system operations,
 * using JUCE's File class for all file I/O operations.
 */
class FileSystem : public IFileSystem
{
public:
    bool createDirectory(const juce::String &path) override;
    bool writeFile(const juce::String &path, const juce::String &content) override;
    bool writeFile(const juce::String &path, const juce::MemoryBlock &data) override;
    juce::String readFile(const juce::String &path) override;
    juce::MemoryBlock readBinaryFile(const juce::String &path) override;
    bool fileExists(const juce::String &path) override;
    bool directoryExists(const juce::String &path) override;
    juce::StringArray getFiles(const juce::String &directory) override;
    juce::StringArray getDirectories(const juce::String &directory) override;
    juce::int64 getFileSize(const juce::String &path) override;
    juce::Time getFileTime(const juce::String &path) override;
    bool deleteFile(const juce::String &path) override;
    bool deleteDirectory(const juce::String &path) override;
    bool moveFile(const juce::String &sourcePath, const juce::String &destPath) override;

    // Path utility functions
    juce::String getFileName(const juce::String &path) override;
    juce::String getParentDirectory(const juce::String &path) override;
    juce::String joinPath(const juce::String &path1, const juce::String &path2) override;
    bool isAbsolutePath(const juce::String &path) override;
    juce::String normalizePath(const juce::String &path) override;
    juce::String getCacheRootDirectory() override;
};