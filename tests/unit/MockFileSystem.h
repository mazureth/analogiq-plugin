#pragma once

#include "../../Source/IFileSystem.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief Abstract base class for file system interface.
 *
 * This class defines the interface for file system operations.
 * The concrete implementation is provided by ConcreteMockFileSystem.
 */
class MockFileSystem : public IFileSystem
{
public:
    virtual ~MockFileSystem() = default;
};

/**
 * @brief Concrete implementation of MockFileSystem for testing purposes.
 *
 * This class provides the actual implementation of file system operations
 * using in-memory storage and includes functionality for mocking file operations
 * and verifying file system calls.
 */
class ConcreteMockFileSystem : public MockFileSystem
{
public:
    /**
     * @brief Get the singleton instance of the mock file system.
     *
     * @return Reference to the singleton instance
     */
    static ConcreteMockFileSystem &getInstance()
    {
        static ConcreteMockFileSystem instance;
        return instance;
    }

    /**
     * @brief Set a mock file with content.
     *
     * @param path The file path to mock
     * @param content The content to return for this file
     */
    void setFile(const juce::String &path, const juce::String &content)
    {
        files[normalizePathHelper(path)] = content;
        fileSizes[normalizePathHelper(path)] = content.length();
        fileTimes[normalizePathHelper(path)] = juce::Time::getCurrentTime();
    }

    /**
     * @brief Set a mock binary file with data.
     *
     * @param path The file path to mock
     * @param data The binary data to return for this file
     */
    void setBinaryFile(const juce::String &path, const juce::MemoryBlock &data)
    {
        binaryFiles[normalizePathHelper(path)] = data;
        fileSizes[normalizePathHelper(path)] = data.getSize();
        fileTimes[normalizePathHelper(path)] = juce::Time::getCurrentTime();
    }

    /**
     * @brief Set a directory to exist.
     *
     * @param path The directory path to mock
     */
    void setDirectory(const juce::String &path)
    {
        directories.insert(normalizePathHelper(path));
    }

    /**
     * @brief Set a file operation to return an error.
     *
     * @param path The path that should return an error
     */
    void setError(const juce::String &path)
    {
        errors.insert(normalizePathHelper(path));
    }

    /**
     * @brief Check if a file operation was performed.
     *
     * @param path The path to check
     * @return true if the path was accessed, false otherwise
     */
    bool wasPathAccessed(const juce::String &path) const
    {
        return accessedPaths.find(normalizePathHelper(path)) != accessedPaths.end();
    }

    /**
     * @brief Get all accessed paths.
     *
     * @return Set of all paths that were accessed
     */
    std::unordered_set<juce::String> getAccessedPaths() const
    {
        return accessedPaths;
    }

    /**
     * @brief Reset the mock state.
     *
     * Clears all files, directories, errors, and accessed paths.
     */
    void reset()
    {
        files.clear();
        binaryFiles.clear();
        directories.clear();
        errors.clear();
        accessedPaths.clear();
        fileSizes.clear();
        fileTimes.clear();
    }

    /**
     * @brief Get the current state of the mock file system.
     *
     * @return String representation of the current state
     */
    juce::String getState() const
    {
        juce::String state = "MockFileSystem State:\n";
        state += "Files: " + juce::String(files.size()) + "\n";
        state += "Binary Files: " + juce::String(binaryFiles.size()) + "\n";
        state += "Directories: " + juce::String(directories.size()) + "\n";
        state += "Accessed Paths: " + juce::String(accessedPaths.size()) + "\n";
        return state;
    }

    // IFileSystem implementation
    bool createDirectory(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        directories.insert(normalizedPath);
        return true;
    }

    bool writeFile(const juce::String &path, const juce::String &content) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        files[normalizedPath] = content;
        fileSizes[normalizedPath] = content.length();
        fileTimes[normalizedPath] = juce::Time::getCurrentTime();
        return true;
    }

    bool writeFile(const juce::String &path, const juce::MemoryBlock &data) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        binaryFiles[normalizedPath] = data;
        fileSizes[normalizedPath] = data.getSize();
        fileTimes[normalizedPath] = juce::Time::getCurrentTime();
        return true;
    }

    juce::String readFile(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return {};
        }

        auto it = files.find(normalizedPath);
        if (it != files.end())
        {
            return it->second;
        }

        return {};
    }

    juce::MemoryBlock readBinaryFile(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return {};
        }

        auto it = binaryFiles.find(normalizedPath);
        if (it != binaryFiles.end())
        {
            return it->second;
        }

        return {};
    }

    bool fileExists(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        return files.find(normalizedPath) != files.end() ||
               binaryFiles.find(normalizedPath) != binaryFiles.end();
    }

    bool directoryExists(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        return directories.find(normalizedPath) != directories.end();
    }

    juce::StringArray getFiles(const juce::String &directory) override
    {
        auto normalizedDir = normalizePathHelper(directory);
        accessedPaths.insert(normalizedDir);

        if (errors.find(normalizedDir) != errors.end())
        {
            return {};
        }

        juce::StringArray result;
        for (const auto &file : files)
        {
            if (isInDirectory(file.first, normalizedDir))
            {
                result.add(getFileNameHelper(file.first));
            }
        }
        for (const auto &file : binaryFiles)
        {
            if (isInDirectory(file.first, normalizedDir))
            {
                result.add(getFileNameHelper(file.first));
            }
        }
        return result;
    }

    juce::StringArray getDirectories(const juce::String &directory) override
    {
        auto normalizedDir = normalizePathHelper(directory);
        accessedPaths.insert(normalizedDir);

        if (errors.find(normalizedDir) != errors.end())
        {
            return {};
        }

        juce::StringArray result;
        for (const auto &dir : directories)
        {
            if (isInDirectory(dir, normalizedDir))
            {
                result.add(getFileNameHelper(dir));
            }
        }
        return result;
    }

    juce::int64 getFileSize(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return -1;
        }

        auto it = fileSizes.find(normalizedPath);
        if (it != fileSizes.end())
        {
            return it->second;
        }

        return -1;
    }

    juce::Time getFileTime(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return juce::Time(0);
        }

        auto it = fileTimes.find(normalizedPath);
        if (it != fileTimes.end())
        {
            return it->second;
        }

        return juce::Time(0);
    }

    bool deleteFile(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        bool deleted = false;
        if (files.erase(normalizedPath) > 0)
            deleted = true;
        if (binaryFiles.erase(normalizedPath) > 0)
            deleted = true;
        fileSizes.erase(normalizedPath);
        fileTimes.erase(normalizedPath);
        return deleted;
    }

    bool deleteDirectory(const juce::String &path) override
    {
        auto normalizedPath = normalizePathHelper(path);
        accessedPaths.insert(normalizedPath);

        if (errors.find(normalizedPath) != errors.end())
        {
            return false;
        }

        // Remove the directory itself
        bool deleted = directories.erase(normalizedPath) > 0;

        // Remove all files and subdirectories in this directory
        std::vector<juce::String> toDelete;
        for (const auto &file : files)
        {
            if (isInDirectory(file.first, normalizedPath))
            {
                toDelete.push_back(file.first);
            }
        }
        for (const auto &file : binaryFiles)
        {
            if (isInDirectory(file.first, normalizedPath))
            {
                toDelete.push_back(file.first);
            }
        }
        for (const auto &dir : directories)
        {
            if (isInDirectory(dir, normalizedPath))
            {
                toDelete.push_back(dir);
            }
        }

        for (const auto &item : toDelete)
        {
            files.erase(item);
            binaryFiles.erase(item);
            directories.erase(item);
            fileSizes.erase(item);
            fileTimes.erase(item);
        }

        return deleted || !toDelete.empty();
    }

    bool moveFile(const juce::String &sourcePath, const juce::String &destPath) override
    {
        auto normalizedSource = normalizePathHelper(sourcePath);
        auto normalizedDest = normalizePathHelper(destPath);
        accessedPaths.insert(normalizedSource);
        accessedPaths.insert(normalizedDest);

        if (errors.find(normalizedSource) != errors.end() ||
            errors.find(normalizedDest) != errors.end())
        {
            return false;
        }

        bool moved = false;

        // Move text file
        auto textIt = files.find(normalizedSource);
        if (textIt != files.end())
        {
            files[normalizedDest] = textIt->second;
            files.erase(textIt);
            fileSizes[normalizedDest] = fileSizes[normalizedSource];
            fileTimes[normalizedDest] = fileTimes[normalizedSource];
            fileSizes.erase(normalizedSource);
            fileTimes.erase(normalizedSource);
            moved = true;
        }

        // Move binary file
        auto binaryIt = binaryFiles.find(normalizedSource);
        if (binaryIt != binaryFiles.end())
        {
            binaryFiles[normalizedDest] = binaryIt->second;
            binaryFiles.erase(binaryIt);
            fileSizes[normalizedDest] = fileSizes[normalizedSource];
            fileTimes[normalizedDest] = fileTimes[normalizedSource];
            fileSizes.erase(normalizedSource);
            fileTimes.erase(normalizedSource);
            moved = true;
        }

        return moved;
    }

    // Path utility functions
    juce::String getFileName(const juce::String &path) override
    {
        return getFileNameHelper(path);
    }

    juce::String getParentDirectory(const juce::String &path) override
    {
        return getParentDirectoryHelper(path);
    }

    juce::String joinPath(const juce::String &path1, const juce::String &path2) override
    {
        return joinPathHelper(path1, path2);
    }

    bool isAbsolutePath(const juce::String &path) override
    {
        return isAbsolutePathHelper(path);
    }

    juce::String normalizePath(const juce::String &path) override
    {
        return normalizePathHelper(path);
    }

    // Mock cache root directory (configurable for tests)
    juce::String mockCacheRoot = "/tmp/AnalogiqCacheMock";
    juce::String getCacheRootDirectory() override { return mockCacheRoot; }
    void setMockCacheRootDirectory(const juce::String &path) { mockCacheRoot = path; }

private:
    ConcreteMockFileSystem() = default; // Private constructor for singleton

    std::unordered_map<juce::String, juce::String> files;
    std::unordered_map<juce::String, juce::MemoryBlock> binaryFiles;
    std::unordered_set<juce::String> directories;
    std::unordered_set<juce::String> errors;
    std::unordered_set<juce::String> accessedPaths;
    std::unordered_map<juce::String, juce::int64> fileSizes;
    std::unordered_map<juce::String, juce::Time> fileTimes;

    /**
     * @brief Check if a path is within a directory.
     *
     * @param path The path to check
     * @param directory The directory to check against
     * @return true if the path is in the directory, false otherwise
     */
    bool isInDirectory(const juce::String &path, const juce::String &directory) const
    {
        if (directory.isEmpty())
            return false;

        auto normalizedPath = normalizePathHelper(path);
        auto normalizedDir = normalizePathHelper(directory);

        if (!normalizedPath.startsWith(normalizedDir))
            return false;

        // Check if it's a direct child (not a subdirectory)
        auto relativePath = normalizedPath.substring(normalizedDir.length());
        if (relativePath.startsWith("/"))
            relativePath = relativePath.substring(1);

        return relativePath.isNotEmpty() && !relativePath.contains("/");
    }

    /**
     * @brief Get the filename from a full path.
     *
     * @param path The full path
     * @return The filename
     */
    juce::String getFileNameHelper(const juce::String &path) const
    {
        auto normalizedPath = normalizePathHelper(path);
        auto lastSlash = normalizedPath.lastIndexOf("/");
        if (lastSlash >= 0)
        {
            return normalizedPath.substring(lastSlash + 1);
        }
        return normalizedPath;
    }

    /**
     * @brief Get the parent directory of a path.
     *
     * @param path The path to get parent directory from
     * @return The parent directory path
     */
    juce::String getParentDirectoryHelper(const juce::String &path) const
    {
        auto normalizedPath = normalizePathHelper(path);
        auto lastSlash = normalizedPath.lastIndexOf("/");
        if (lastSlash >= 0)
        {
            return normalizedPath.substring(0, lastSlash);
        }
        return {};
    }

    /**
     * @brief Join path components together.
     *
     * @param path1 The first path component
     * @param path2 The second path component
     * @return The joined path
     */
    juce::String joinPathHelper(const juce::String &path1, const juce::String &path2) const
    {
        auto normalizedPath1 = normalizePathHelper(path1);
        auto normalizedPath2 = normalizePathHelper(path2);

        if (normalizedPath1.endsWith("/"))
            return normalizedPath1 + normalizedPath2;
        else
            return normalizedPath1 + "/" + normalizedPath2;
    }

    /**
     * @brief Check if a path is absolute.
     *
     * @param path The path to check
     * @return true if the path is absolute, false if relative
     */
    bool isAbsolutePathHelper(const juce::String &path) const
    {
        auto normalizedPath = normalizePathHelper(path);
        return normalizedPath.startsWith("/") ||
               (normalizedPath.length() >= 2 && normalizedPath[1] == ':');
    }

    /**
     * @brief Normalize a path for consistent storage and lookup.
     *
     * @param path The path to normalize
     * @return The normalized path
     */
    juce::String normalizePathHelper(const juce::String &path) const
    {
        // Pure string-based path normalization without creating JUCE File objects
        juce::String normalized = path;

        // Replace backslashes with forward slashes
        normalized = normalized.replaceCharacter('\\', '/');

        // Remove duplicate slashes
        while (normalized.contains("//"))
        {
            normalized = normalized.replace("//", "/");
        }

        // Remove trailing slash unless it's the root
        if (normalized.length() > 1 && normalized.endsWith("/"))
        {
            normalized = normalized.substring(0, normalized.length() - 1);
        }

        // Handle relative paths by making them absolute (for consistent storage)
        if (!normalized.startsWith("/") && !normalized.contains(":"))
        {
            normalized = "/" + normalized;
        }

        return normalized;
    }
};