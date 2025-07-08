#include "FileSystem.h"
#include <JuceHeader.h>

bool FileSystem::createDirectory(const juce::String &path)
{
    juce::File file(path);
    return file.createDirectory();
}

bool FileSystem::writeFile(const juce::String &path, const juce::String &content)
{
    juce::File file(path);
    return file.replaceWithText(content);
}

bool FileSystem::writeFile(const juce::String &path, const juce::MemoryBlock &data)
{
    juce::File file(path);
    return file.replaceWithData(data.getData(), data.getSize());
}

juce::String FileSystem::readFile(const juce::String &path)
{
    juce::File file(path);
    if (file.existsAsFile())
    {
        return file.loadFileAsString();
    }
    return {};
}

juce::MemoryBlock FileSystem::readBinaryFile(const juce::String &path)
{
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
    juce::File file(path);
    return file.existsAsFile();
}

bool FileSystem::directoryExists(const juce::String &path)
{
    juce::File file(path);
    return file.isDirectory();
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
    juce::File file(path);
    return file.getFileName();
}

juce::String FileSystem::getParentDirectory(const juce::String &path)
{
    juce::File file(path);
    return file.getParentDirectory().getFullPathName();
}

juce::String FileSystem::joinPath(const juce::String &path1, const juce::String &path2)
{
    juce::File file1(path1);
    return file1.getChildFile(path2).getFullPathName();
}

bool FileSystem::isAbsolutePath(const juce::String &path)
{
    return juce::File::isAbsolutePath(path);
}

juce::String FileSystem::normalizePath(const juce::String &path)
{
    juce::File file(path);
    return file.getFullPathName();
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
};

IFileSystem &IFileSystem::getDummy()
{
    static DummyFileSystem instance;
    return instance;
}