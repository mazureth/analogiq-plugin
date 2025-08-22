#include "FileSystem.h"
#include <juce_core/juce_core.h>

const juce::String FileSystem::ANALOGIQ_CACHE_DIR = "AnalogIQ";

// Core file operations
bool FileSystem::createDirectory(const juce::String &path)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File dir(path);
        return dir.createDirectory();
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::writeFile(const juce::String &path, const juce::String &content)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File file(path);
        return file.replaceWithText(content);
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::writeFile(const juce::String &path, const juce::MemoryBlock &data)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File file(path);
        return file.replaceWithData(data.getData(), data.getSize());
    }
    catch (...)
    {
        return false;
    }
}

juce::String FileSystem::readFile(const juce::String &path)
{
    if (path.isEmpty())
        return juce::String();

    try
    {
        juce::File file(path);
        if (file.existsAsFile())
            return file.loadFileAsString();
    }
    catch (...)
    {
        // Return empty string on error
    }
    return juce::String();
}

juce::MemoryBlock FileSystem::readBinaryFile(const juce::String &path)
{
    if (path.isEmpty())
        return juce::MemoryBlock();

    try
    {
        juce::File file(path);
        if (file.existsAsFile())
        {
            juce::MemoryBlock block;
            if (file.loadFileAsData(block))
                return block;
        }
    }
    catch (...)
    {
        // Return empty block on error
    }
    return juce::MemoryBlock();
}

bool FileSystem::fileExists(const juce::String &path)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File file(path);
        bool exists = file.existsAsFile();

        // Special handling for JPEG files to prevent JUCE assertions
        if (exists && path.toLowerCase().endsWith(".jpg"))
        {
            try
            {
                juce::FileInputStream fis(file);
                if (fis.failedToOpen())
                    return false;
            }
            catch (...)
            {
                return false;
            }
        }

        return exists;
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::directoryExists(const juce::String &path)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File dir(path);
        return dir.isDirectory();
    }
    catch (...)
    {
        return false;
    }
}

// File system management
juce::StringArray FileSystem::getFiles(const juce::String &directory)
{
    if (directory.isEmpty())
        return juce::StringArray();

    try
    {
        juce::File dir(directory);
        if (dir.isDirectory())
        {
            juce::Array<juce::File> files;
            dir.findChildFiles(files, juce::File::findFiles, false);

            juce::StringArray result;
            for (const auto &file : files)
                result.add(file.getFileName());
            return result;
        }
    }
    catch (...)
    {
        // Return empty array on error
    }
    return juce::StringArray();
}

juce::StringArray FileSystem::getDirectories(const juce::String &directory)
{
    if (directory.isEmpty())
        return juce::StringArray();

    try
    {
        juce::File dir(directory);
        if (dir.isDirectory())
        {
            juce::Array<juce::File> dirs;
            dir.findChildFiles(dirs, juce::File::findDirectories, false);

            juce::StringArray result;
            for (const auto &subdir : dirs)
                result.add(subdir.getFileName());
            return result;
        }
    }
    catch (...)
    {
        // Return empty array on error
    }
    return juce::StringArray();
}

bool FileSystem::deleteFile(const juce::String &path)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File file(path);
        return file.deleteFile();
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::deleteDirectory(const juce::String &path, bool recursive)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File dir(path);
        if (recursive)
            return dir.deleteRecursively();
        else
            return dir.deleteRecursively();
    }
    catch (...)
    {
        return false;
    }
}

bool FileSystem::moveFile(const juce::String &sourcePath, const juce::String &destPath)
{
    if (sourcePath.isEmpty() || destPath.isEmpty())
        return false;

    try
    {
        juce::File sourceFile(sourcePath);
        juce::File destFile(destPath);
        return sourceFile.moveFileTo(destFile);
    }
    catch (...)
    {
        return false;
    }
}

// File metadata
juce::int64 FileSystem::getFileSize(const juce::String &path)
{
    if (path.isEmpty())
        return -1;

    try
    {
        juce::File file(path);
        if (file.existsAsFile())
            return file.getSize();
    }
    catch (...)
    {
        // Return -1 on error
    }
    return -1;
}

juce::Time FileSystem::getFileTime(const juce::String &path)
{
    if (path.isEmpty())
        return juce::Time(0);

    try
    {
        juce::File file(path);
        if (file.existsAsFile())
            return file.getLastModificationTime();
    }
    catch (...)
    {
        // Return Time(0) on error
    }
    return juce::Time(0);
}

// Path utilities
juce::String FileSystem::getFileName(const juce::String &path)
{
    if (path.isEmpty())
        return juce::String();

    try
    {
        // Handle relative paths manually to avoid JUCE issues
        if (path.startsWith("./") || path.startsWith("../"))
        {
            int lastSlash = path.lastIndexOfChar('/');
            if (lastSlash >= 0)
                return path.substring(lastSlash + 1);
            return path;
        }

        juce::File file(path);
        return file.getFileName();
    }
    catch (...)
    {
        return juce::String();
    }
}

juce::String FileSystem::getParentDirectory(const juce::String &path)
{
    if (path.isEmpty())
        return juce::String();

    try
    {
        juce::File file(path);
        juce::File parent = file.getParentDirectory();
        return parent.getFullPathName();
    }
    catch (...)
    {
        return juce::String();
    }
}

juce::String FileSystem::joinPath(const juce::String &path1, const juce::String &path2)
{
    if (path1.isEmpty())
        return path2;
    if (path2.isEmpty())
        return path1;

    try
    {
        juce::File file1(path1);
        juce::File file2(path2);
        return file1.getChildFile(file2.getFileName()).getFullPathName();
    }
    catch (...)
    {
        return path1 + "/" + path2;
    }
}

bool FileSystem::isAbsolutePath(const juce::String &path)
{
    if (path.isEmpty())
        return false;

    try
    {
        juce::File file(path);
        return juce::File::isAbsolutePath(path);
    }
    catch (...)
    {
        return false;
    }
}

juce::String FileSystem::normalizePath(const juce::String &path)
{
    if (path.isEmpty())
        return juce::String();

    try
    {
        juce::File file(path);
        return file.getFullPathName();
    }
    catch (...)
    {
        return path;
    }
}

// Cache directory management
juce::String FileSystem::getCacheRootDirectory()
{
    try
    {
        juce::File cacheDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        return cacheDir.getChildFile(ANALOGIQ_CACHE_DIR).getFullPathName();
    }
    catch (...)
    {
        return juce::String();
    }
}
