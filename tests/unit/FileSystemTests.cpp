#include <JuceHeader.h>
#include "TestFixture.h"
#include "../../Source/FileSystem.h"
#include <cstring>

/**
 * @class FileSystemTests
 * @brief Comprehensive tests for the FileSystem class.
 * 
 * These tests cover all methods of the FileSystem class including:
 * - Directory creation and checking
 * - File writing and reading (both text and binary)
 * - File existence checking
 * - File and directory operations
 * - Path utility functions
 * - Error handling and edge cases
 */
class FileSystemTests : public juce::UnitTest
{
public:
    FileSystemTests() : juce::UnitTest("FileSystemTests", "FileSystemTests") {}

    void runTest() override
    {
        beginTest("Directory Operations");
        testDirectoryOperations();

        beginTest("File Writing and Reading");
        testFileOperations();

        beginTest("Binary File Operations");
        testBinaryFileOperations();

        beginTest("File Existence Checking");
        testFileExistence();

        beginTest("Directory Listing");
        testDirectoryListing();

        beginTest("File Properties");
        testFileProperties();

        beginTest("File and Directory Deletion");
        testDeletion();

        beginTest("File Moving");
        testFileMoving();

        beginTest("Path Utility Functions");
        testPathUtilities();

        beginTest("Error Handling and Edge Cases");
        testErrorHandling();

        beginTest("JPEG File Handling");
        testJpegFileHandling();

        beginTest("Cache Directory");
        testCacheDirectory();
    }

private:
    void testDirectoryOperations()
    {
        FileSystem fs;
        juce::String testDir = "/tmp/analogiq_test_dir";

        // Test directory creation
        expect(fs.createDirectory(testDir), "Should create directory successfully");
        expect(fs.directoryExists(testDir), "Directory should exist after creation");

        // Test creating directory that already exists
        expect(fs.createDirectory(testDir), "Should handle existing directory gracefully");

        // Clean up
        fs.deleteDirectory(testDir);
    }

    void testFileOperations()
    {
        FileSystem fs;
        juce::String testFile = "/tmp/analogiq_test_file.txt";
        juce::String testContent = "Test content for file operations";

        // Test writing and reading text file
        expect(fs.writeFile(testFile, testContent), "Should write text file successfully");
        expect(fs.fileExists(testFile), "File should exist after writing");

        juce::String readContent = fs.readFile(testFile);
        expectEquals(readContent, testContent, "Read content should match written content");

        // Clean up
        fs.deleteFile(testFile);
    }

    void testBinaryFileOperations()
    {
        FileSystem fs;
        juce::String testFile = "/tmp/analogiq_test_binary.dat";
        
        // Create simple test binary data (avoid complex memory operations)
        juce::MemoryBlock originalData;
        const char* testData = "Binary test data";
        originalData.append(testData, strlen(testData));

        // Test writing and reading binary file
        expect(fs.writeFile(testFile, originalData), "Should write binary file successfully");
        expect(fs.fileExists(testFile), "Binary file should exist after writing");

        juce::MemoryBlock readData = fs.readBinaryFile(testFile);
        expect(readData.getSize() == originalData.getSize(), "Read data size should match original");
        
        // Compare data more safely
        bool dataMatches = (readData.getSize() == originalData.getSize());
        if (dataMatches && readData.getSize() > 0)
        {
            dataMatches = memcmp(readData.getData(), originalData.getData(), readData.getSize()) == 0;
        }
        expect(dataMatches, "Binary data should match exactly");

        // Clean up
        fs.deleteFile(testFile);
    }

    void testFileExistence()
    {
        FileSystem fs;
        juce::String nonExistentFile = "/tmp/non_existent_file_12345.txt";
        juce::String nonExistentDir = "/tmp/non_existent_dir_12345";

        // Test non-existent file and directory
        expect(!fs.fileExists(nonExistentFile), "Non-existent file should not exist");
        expect(!fs.directoryExists(nonExistentDir), "Non-existent directory should not exist");

        // Test empty path handling
        expect(!fs.fileExists(""), "Empty path should not exist");
        expect(!fs.directoryExists(""), "Empty directory path should not exist");
    }

    void testDirectoryListing()
    {
        FileSystem fs;
        juce::String testDir = "/tmp/analogiq_listing_test";
        juce::String subDir = testDir + "/subdir";
        juce::String testFile1 = testDir + "/file1.txt";
        juce::String testFile2 = testDir + "/file2.txt";

        // Create test structure
        fs.createDirectory(testDir);
        fs.createDirectory(subDir);
        fs.writeFile(testFile1, "content1");
        fs.writeFile(testFile2, "content2");

        // Test file listing
        juce::StringArray files = fs.getFiles(testDir);
        expect(files.size() == 2, "Should find 2 files");
        expect(files.contains("file1.txt"), "Should contain file1.txt");
        expect(files.contains("file2.txt"), "Should contain file2.txt");

        // Test directory listing
        juce::StringArray dirs = fs.getDirectories(testDir);
        expect(dirs.size() == 1, "Should find 1 directory");
        expect(dirs.contains("subdir"), "Should contain subdir");

        // Clean up
        fs.deleteDirectory(testDir);
    }

    void testFileProperties()
    {
        FileSystem fs;
        juce::String testFile = "/tmp/analogiq_properties_test.txt";
        juce::String testContent = "Test content for properties";

        // Write file
        fs.writeFile(testFile, testContent);

        // Test file size
        juce::int64 fileSize = fs.getFileSize(testFile);
        expect(fileSize == testContent.length(), "File size should match content length");

        // Test file time
        juce::Time fileTime = fs.getFileTime(testFile);
        expect(fileTime.toMilliseconds() > 0, "File time should be valid");

        // Test properties for non-existent file
        juce::int64 badSize = fs.getFileSize("/tmp/non_existent_file_properties.txt");
        expect(badSize == -1, "Non-existent file should return -1 for size");

        juce::Time badTime = fs.getFileTime("/tmp/non_existent_file_properties.txt");
        expect(badTime.toMilliseconds() == 0, "Non-existent file should return Time(0)");

        // Clean up
        fs.deleteFile(testFile);
    }

    void testDeletion()
    {
        FileSystem fs;
        juce::String testFile = "/tmp/analogiq_delete_test.txt";
        juce::String testDir = "/tmp/analogiq_delete_dir_test";
        juce::String subFile = testDir + "/subfile.txt";

        // Test file deletion
        fs.writeFile(testFile, "delete me");
        expect(fs.fileExists(testFile), "File should exist before deletion");
        expect(fs.deleteFile(testFile), "Should delete file successfully");
        expect(!fs.fileExists(testFile), "File should not exist after deletion");

        // Test directory deletion (recursive)
        fs.createDirectory(testDir);
        fs.writeFile(subFile, "subfile content");
        expect(fs.directoryExists(testDir), "Directory should exist before deletion");
        expect(fs.deleteDirectory(testDir), "Should delete directory recursively");
        expect(!fs.directoryExists(testDir), "Directory should not exist after deletion");

        // Test deleting non-existent file (should not crash)
        bool result = fs.deleteFile("/tmp/non_existent_delete_test.txt");
        // Result may be true or false depending on platform, just ensure it doesn't crash
    }

    void testFileMoving()
    {
        FileSystem fs;
        juce::String sourceFile = "/tmp/analogiq_move_source.txt";
        juce::String destFile = "/tmp/analogiq_move_dest.txt";
        juce::String testContent = "content to move";

        // Create source file
        fs.writeFile(sourceFile, testContent);
        expect(fs.fileExists(sourceFile), "Source file should exist");

        // Move file
        expect(fs.moveFile(sourceFile, destFile), "Should move file successfully");
        expect(!fs.fileExists(sourceFile), "Source file should not exist after move");
        expect(fs.fileExists(destFile), "Destination file should exist after move");

        // Verify content
        juce::String movedContent = fs.readFile(destFile);
        expectEquals(movedContent, testContent, "Moved file content should be preserved");

        // Clean up
        fs.deleteFile(destFile);
    }

    void testPathUtilities()
    {
        FileSystem fs;

        // Test getFileName
        expect(fs.getFileName("/path/to/file.txt") == "file.txt", "Should extract filename from absolute path");
        expect(fs.getFileName("relative/path/file.txt") == "file.txt", "Should extract filename from relative path");
        expect(fs.getFileName("file.txt") == "file.txt", "Should handle simple filename");
        expect(fs.getFileName("") == "", "Should handle empty path");

        // Test getParentDirectory
        juce::String parent = fs.getParentDirectory("/tmp/test_file.txt");
        expect(parent.isNotEmpty(), "Should return valid parent directory");

        // Test joinPath
        juce::String joined = fs.joinPath("/tmp", "subdir");
        expect(joined.contains("/tmp"), "Joined path should contain base path");
        expect(joined.contains("subdir"), "Joined path should contain sub path");
        
        // Test empty path handling
        expect(fs.joinPath("", "test") == "", "Should handle empty base path");
        expect(fs.joinPath("/tmp", "") == "/tmp", "Should handle empty sub path");

        // Test isAbsolutePath
        expect(fs.isAbsolutePath("/absolute/path"), "Should detect absolute path");
        expect(!fs.isAbsolutePath("relative/path"), "Should detect relative path");

        // Test normalizePath
        juce::String normalized = fs.normalizePath("/tmp/../tmp/./test");
        expect(normalized.isNotEmpty(), "Should normalize path successfully");
        expect(fs.normalizePath("") == "", "Should handle empty path normalization");
    }

    void testErrorHandling()
    {
        FileSystem fs;

        // Test createDirectory with empty path
        expect(!fs.createDirectory(""), "Should reject empty directory path");

        // Test readFile with empty path
        expect(fs.readFile("") == "", "Should return empty string for empty path");

        // Test readBinaryFile with empty path
        juce::MemoryBlock emptyResult = fs.readBinaryFile("");
        expect(emptyResult.getSize() == 0, "Should return empty MemoryBlock for empty path");

        // Test reading non-existent file
        expect(fs.readFile("/tmp/non_existent_read_test.txt") == "", "Should return empty string for non-existent file");
        
        juce::MemoryBlock nonExistentResult = fs.readBinaryFile("/tmp/non_existent_binary_test.dat");
        expect(nonExistentResult.getSize() == 0, "Should return empty MemoryBlock for non-existent file");
    }

    void testJpegFileHandling()
    {
        FileSystem fs;
        juce::String jpegFile = "/tmp/test_image.jpg";
        juce::String jpegFile2 = "/tmp/test_image.jpeg";

        // Create dummy JPEG files (just text files with .jpg extension for testing)
        fs.writeFile(jpegFile, "fake jpeg content");
        fs.writeFile(jpegFile2, "fake jpeg content");

        // Test JPEG file existence checking (which has special handling)
        bool jpegExists = fs.fileExists(jpegFile);
        bool jpeg2Exists = fs.fileExists(jpegFile2);

        // The method should handle JPEG files without crashing
        // (The actual result depends on whether JUCE can read the fake files)

        // Clean up
        fs.deleteFile(jpegFile);
        fs.deleteFile(jpegFile2);
    }

    void testCacheDirectory()
    {
        FileSystem fs;

        // Test getCacheRootDirectory
        juce::String cacheDir = fs.getCacheRootDirectory();
        expect(cacheDir.isNotEmpty(), "Cache directory path should not be empty");
        expect(cacheDir.contains("AnalogiqCache"), "Cache directory should contain AnalogiqCache");
    }
};

static FileSystemTests fileSystemTests;