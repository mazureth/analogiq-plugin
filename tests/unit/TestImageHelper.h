#pragma once

#include <JuceHeader.h>

/**
 * @brief Helper class for creating test images with minimal memory footprint.
 *
 * This class provides methods to create test images for mocking while
 * minimizing the risk of triggering JUCE's leak detector.
 */
class TestImageHelper
{
public:
    /**
     * @brief Create a minimal test image and convert it to JPEG data.
     *
     * This method creates the smallest possible image and immediately
     * converts it to binary data to minimize memory usage and leak detection.
     *
     * @return MemoryBlock containing JPEG image data
     */
    static juce::MemoryBlock createMinimalTestImageData()
    {
        // Create a 24x24 image to match test expectations
        juce::Image testImage(juce::Image::RGB, 24, 24, true);

        {
            juce::Graphics g(testImage);
            g.fillAll(juce::Colours::darkgrey);
            g.setColour(juce::Colours::white);
            g.drawText("Test", testImage.getBounds(), juce::Justification::centred, true);
        }

        // Convert to JPEG with minimal quality to reduce size
        juce::MemoryOutputStream stream;
        juce::JPEGImageFormat jpegFormat;
        jpegFormat.setQuality(0.1f); // Minimal quality

        if (jpegFormat.writeImageToStream(testImage, stream))
        {
            return juce::MemoryBlock(stream.getData(), stream.getDataSize());
        }

        // Fallback: return empty block if encoding fails
        return juce::MemoryBlock();
    }

    /**
     * @brief Get a static test image data that's reused across tests.
     *
     * This returns the same image data every time to avoid creating
     * multiple Image objects during test execution.
     *
     * @return Reference to static MemoryBlock containing test image data
     */
    static const juce::MemoryBlock &getStaticTestImageData()
    {
        static juce::MemoryBlock staticImageData = createMinimalTestImageData();
        return staticImageData;
    }

    /**
     * @brief Get static test tags to avoid creating multiple StringArray objects.
     *
     * This returns the same StringArray every time to prevent JUCE leak detection
     * from seeing multiple temporary StringArray creations during tests.
     *
     * @return Reference to static StringArray containing test tags
     */
    static const juce::StringArray &getStaticTestTags()
    {
        static juce::StringArray tags = {"compressor", "tube", "optical", "vintage", "hardware"};
        return tags;
    }
};