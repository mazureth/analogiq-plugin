/**
 * @file FileSystem.cpp
 * @brief Implementation of the FileSystem class for file operations.
 *
 * This file provides the concrete implementation of the IFileSystem interface
 * using JUCE's File class for all file I/O operations. It includes methods
 * for creating directories, reading/writing files, checking file existence,
 * and path manipulation utilities. The file also includes a DummyFileSystem
 * implementation for the Null Object Pattern used in testing.
 */

#include "FileSystem.h"
#include <JuceHeader.h>

// Cache directory name constant
static const juce::String ANALOGIQ_CACHE_DIR = "AnalogiqCache";

bool FileSystem::createDirectory(const juce::String &path)
{
    if (path.isEmpty())
    {
        return false;
    }

    try
    {
        juce::File file(path);
        bool result = file.createDirectory();
        return result;
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::writeFile(const juce::String &path, const juce::String &content)
{
    juce::File file(path);
    bool result = file.replaceWithText(content);
    return result;
}

bool FileSystem::writeFile(const juce::String &path, const juce::MemoryBlock &data)
{
    juce::File file(path);
    return file.replaceWithData(data.getData(), data.getSize());
}

juce::String FileSystem::readFile(const juce::String &path)
{
    if (path.isEmpty())
    {
        return {};
    }

    try
    {
        juce::File file(path);
        if (file.existsAsFile())
        {
            juce::String result = file.loadFileAsString();
            return result;
        }
        return {};
    }
    catch (...)
    {
        return {};
    }
}

juce::MemoryBlock FileSystem::readBinaryFile(const juce::String &path)
{
    if (path.isEmpty())
    {
        return {};
    }

    juce::File file(path);
    juce::MemoryBlock data;
    if (file.existsAsFile())
    {
        file.loadFileAsData(data);
    }
    return data;
}

bool FileSystem::fileExists(const juce::String &path)
{
    if (path.isEmpty())
    {
        return false;
    }

    try
    {
        juce::File file(path);
        bool result = file.existsAsFile();

        // Special handling for JPEG files that cause JUCE assertions
        if (result && (path.endsWith(".jpg") || path.endsWith(".jpeg")))
        {
            // For JPEG files, also verify the file is readable to prevent assertions
            try
            {
                juce::FileInputStream stream(file);
                if (stream.failedToOpen())
                {
                    return false;
                }
            }
            catch (...)
            {
                return false;
            }
        }

        return result;
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::directoryExists(const juce::String &path)
{
    if (path.isEmpty())
    {
        return false;
    }

    juce::File file(path);
    bool result = file.isDirectory();
    return result;
}

juce::StringArray FileSystem::getFiles(const juce::String &directory)
{
    juce::File dir(directory);
    juce::Array<juce::File> files = dir.findChildFiles(juce::File::findFiles, false);
    juce::StringArray result;
    for (const auto &file : files)
        result.add(file.getFileName());
    return result;
}

juce::StringArray FileSystem::getDirectories(const juce::String &directory)
{
    juce::File dir(directory);
    juce::Array<juce::File> dirs = dir.findChildFiles(juce::File::findDirectories, false);
    juce::StringArray result;
    for (const auto &d : dirs)
        result.add(d.getFileName());
    return result;
}

juce::int64 FileSystem::getFileSize(const juce::String &path)
{
    juce::File file(path);
    if (file.existsAsFile())
    {
        return file.getSize();
    }
    return -1;
}

juce::Time FileSystem::getFileTime(const juce::String &path)
{
    juce::File file(path);
    if (file.existsAsFile())
    {
        return file.getLastModificationTime();
    }
    return juce::Time(0);
}

bool FileSystem::deleteFile(const juce::String &path)
{
    juce::File file(path);
    return file.deleteFile();
}

bool FileSystem::deleteDirectory(const juce::String &path)
{
    juce::File dir(path);
    return dir.deleteRecursively();
}

bool FileSystem::moveFile(const juce::String &sourcePath, const juce::String &destPath)
{
    juce::File source(sourcePath);
    juce::File dest(destPath);
    return source.moveFileTo(dest);
}

// Path utility functions
juce::String FileSystem::getFileName(const juce::String &path)
{
    if (path.isEmpty())
    {
        return {};
    }

    // Handle relative paths by extracting just the filename part
    if (!juce::File::isAbsolutePath(path))
    {
        // For relative paths, just return the last component
        int lastSlash = path.lastIndexOfChar('/');
        if (lastSlash >= 0)
        {
            juce::String result = path.substring(lastSlash + 1);
            return result;
        }
        else
        {
            // No slashes, return the whole path as filename
            return path;
        }
    }

    // For absolute paths, use JUCE File
    juce::File file(path);
    juce::String result = file.getFileName();
    return result;
}

juce::String FileSystem::getParentDirectory(const juce::String &path)
{
    if (path.isEmpty())
    {
        return {};
    }

    juce::File file(path);
    juce::String result = file.getParentDirectory().getFullPathName();
    return result;
}

juce::String FileSystem::joinPath(const juce::String &path1, const juce::String &path2)
{
    if (path1.isEmpty())
    {
        return {};
    }

    if (path2.isEmpty())
    {
        return path1;
    }

    juce::File file1(path1);
    juce::String result = file1.getChildFile(path2).getFullPathName();
    return result;
}

bool FileSystem::isAbsolutePath(const juce::String &path)
{
    return juce::File::isAbsolutePath(path);
}

juce::String FileSystem::normalizePath(const juce::String &path)
{
    if (path.isEmpty())
    {
        return {};
    }

    juce::File file(path);
    juce::String result = file.getFullPathName();
    return result;
}

juce::String FileSystem::getCacheRootDirectory()
{
    // Use OS-agnostic JUCE approach for user application data directory
    juce::File userDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File cacheRoot = userDataDir.getChildFile(ANALOGIQ_CACHE_DIR);
    juce::String result = cacheRoot.getFullPathName();
    return result;
}

// Null Object Pattern: DummyFileSystem implementation
class DummyFileSystem : public IFileSystem
{
public:
    bool createDirectory(const juce::String &) override { return false; }
    bool writeFile(const juce::String &, const juce::String &) override { return false; }
    bool writeFile(const juce::String &, const juce::MemoryBlock &) override { return false; }
    juce::String readFile(const juce::String &) override { return {}; }
    juce::MemoryBlock readBinaryFile(const juce::String &) override { return {}; }
    bool fileExists(const juce::String &) override { return false; }
    bool directoryExists(const juce::String &) override { return false; }
    juce::StringArray getFiles(const juce::String &) override { return {}; }
    juce::StringArray getDirectories(const juce::String &) override { return {}; }
    juce::int64 getFileSize(const juce::String &) override { return -1; }
    juce::Time getFileTime(const juce::String &) override { return juce::Time(0); }
    bool deleteFile(const juce::String &) override { return false; }
    bool deleteDirectory(const juce::String &) override { return false; }
    bool moveFile(const juce::String &, const juce::String &) override { return false; }
    juce::String getFileName(const juce::String &) override { return {}; }
    juce::String getParentDirectory(const juce::String &) override { return {}; }
    juce::String joinPath(const juce::String &, const juce::String &) override { return {}; }
    bool isAbsolutePath(const juce::String &) override { return false; }
    juce::String normalizePath(const juce::String &) override { return {}; }
    juce::String getCacheRootDirectory() override { return {}; }
};

IFileSystem &IFileSystem::getDummy()
{
    static DummyFileSystem instance;
    return instance;
}