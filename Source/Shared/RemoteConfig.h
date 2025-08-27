#pragma once
#include <juce_core/juce_core.h>

/**
 * @brief Remote server configuration for AnalogIQ
 *
 * This class centralizes all remote server configuration including base URLs
 * and relative paths. It can be easily updated without recompiling the plugin.
 */
class RemoteConfig
{
public:
    // Base URLs for different remote services
    static const juce::String GEAR_LIBRARY_BASE_URL;
    static const juce::String SCHEMAS_BASE_URL;
    static const juce::String THUMBNAILS_BASE_URL;

    // Relative paths from base URLs
    static const juce::String GEAR_LIBRARY_INDEX_PATH;
    static const juce::String SCHEMAS_PATH;
    static const juce::String THUMBNAILS_PATH;

    // Full constructed URLs
    static juce::String getGearLibraryIndexUrl();
    static juce::String getSchemaUrl(const juce::String &schemaPath);
    static juce::String getThumbnailUrl(const juce::String &thumbnailPath);

    // URL validation and utilities
    static bool isValidUrl(const juce::String &url);
    static juce::String makeAbsoluteUrl(const juce::String &baseUrl, const juce::String &relativePath);

private:
    // Prevent instantiation
    RemoteConfig() = delete;
};
