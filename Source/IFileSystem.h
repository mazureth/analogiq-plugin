#pragma once

#include <JuceHeader.h>

/**
 * @brief Interface for file system operations.
 *
 * This interface provides a abstraction layer for file system operations,
 * allowing for easy mocking in tests and different implementations
 * for different platforms or use cases.
 */
class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    /**
     * @brief Creates a directory at the specified path.
     *
     * @param path The directory path to create
     * @return true if the directory was created successfully, false otherwise
     */
    virtual bool createDirectory(const juce::String &path) = 0;

    /**
     * @brief Writes content to a file at the specified path.
     *
     * @param path The file path to write to
     * @param content The content to write to the file
     * @return true if the file was written successfully, false otherwise
     */
    virtual bool writeFile(const juce::String &path, const juce::String &content) = 0;

    /**
     * @brief Writes binary data to a file at the specified path.
     *
     * @param path The file path to write to
     * @param data The binary data to write to the file
     * @return true if the file was written successfully, false otherwise
     */
    virtual bool writeFile(const juce::String &path, const juce::MemoryBlock &data) = 0;

    /**
     * @brief Reads content from a file at the specified path.
     *
     * @param path The file path to read from
     * @return The content of the file as a string, or empty string if read failed
     */
    virtual juce::String readFile(const juce::String &path) = 0;

    /**
     * @brief Reads binary data from a file at the specified path.
     *
     * @param path The file path to read from
     * @return The binary data from the file, or empty MemoryBlock if read failed
     */
    virtual juce::MemoryBlock readBinaryFile(const juce::String &path) = 0;

    /**
     * @brief Checks if a file exists at the specified path.
     *
     * @param path The file path to check
     * @return true if the file exists, false otherwise
     */
    virtual bool fileExists(const juce::String &path) = 0;

    /**
     * @brief Checks if a directory exists at the specified path.
     *
     * @param path The directory path to check
     * @return true if the directory exists, false otherwise
     */
    virtual bool directoryExists(const juce::String &path) = 0;

    /**
     * @brief Gets a list of files in the specified directory.
     *
     * @param directory The directory path to list files from
     * @return Array of file names in the directory
     */
    virtual juce::StringArray getFiles(const juce::String &directory) = 0;

    /**
     * @brief Gets a list of subdirectories in the specified directory.
     *
     * @param directory The directory path to list subdirectories from
     * @return Array of subdirectory names
     */
    virtual juce::StringArray getDirectories(const juce::String &directory) = 0;

    /**
     * @brief Gets the size of a file in bytes.
     *
     * @param path The file path
     * @return The file size in bytes, or -1 if the file doesn't exist or can't be accessed
     */
    virtual juce::int64 getFileSize(const juce::String &path) = 0;

    /**
     * @brief Gets the last modification time of a file.
     *
     * @param path The file path
     * @return The last modification time, or Time(0) if the file doesn't exist
     */
    virtual juce::Time getFileTime(const juce::String &path) = 0;

    /**
     * @brief Deletes a file at the specified path.
     *
     * @param path The file path to delete
     * @return true if the file was deleted successfully, false otherwise
     */
    virtual bool deleteFile(const juce::String &path) = 0;

    /**
     * @brief Deletes a directory and all its contents recursively.
     *
     * @param path The directory path to delete
     * @return true if the directory was deleted successfully, false otherwise
     */
    virtual bool deleteDirectory(const juce::String &path) = 0;

    /**
     * @brief Moves or renames a file or directory.
     *
     * @param sourcePath The source path
     * @param destPath The destination path
     * @return true if the move/rename was successful, false otherwise
     */
    virtual bool moveFile(const juce::String &sourcePath, const juce::String &destPath) = 0;

    // Path utility functions to avoid direct juce::File usage

    /**
     * @brief Extracts the filename from a path.
     *
     * @param path The path to extract filename from
     * @return The filename without directory path
     */
    virtual juce::String getFileName(const juce::String &path) = 0;

    /**
     * @brief Gets the parent directory of a path.
     *
     * @param path The path to get parent directory from
     * @return The parent directory path
     */
    virtual juce::String getParentDirectory(const juce::String &path) = 0;

    /**
     * @brief Joins path components together.
     *
     * @param path1 The first path component
     * @param path2 The second path component
     * @return The joined path
     */
    virtual juce::String joinPath(const juce::String &path1, const juce::String &path2) = 0;

    /**
     * @brief Checks if a path is absolute.
     *
     * @param path The path to check
     * @return true if the path is absolute, false if relative
     */
    virtual bool isAbsolutePath(const juce::String &path) = 0;

    /**
     * @brief Normalizes a path by removing redundant separators and resolving relative components.
     *
     * @param path The path to normalize
     * @return The normalized path
     */
    virtual juce::String normalizePath(const juce::String &path) = 0;

    /**
     * @brief Returns a reference to a dummy file system (Null Object Pattern).
     *
     * This can be used for default-constructed objects or in cases where a real file system is not available.
     */
    static IFileSystem &getDummy();
};