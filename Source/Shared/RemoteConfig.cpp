#include "RemoteConfig.h"
#include <juce_core/juce_core.h>

// Base URLs - these can be easily updated without recompiling
const juce::String RemoteConfig::GEAR_LIBRARY_BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main";
const juce::String RemoteConfig::SCHEMAS_BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main";
const juce::String RemoteConfig::THUMBNAILS_BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main";
const juce::String RemoteConfig::FACEPLATES_BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main";
const juce::String RemoteConfig::CONTROL_IMAGES_BASE_URL = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main";

// Relative paths from base URLs
const juce::String RemoteConfig::GEAR_LIBRARY_INDEX_PATH = "/units/index.json";
const juce::String RemoteConfig::SCHEMAS_PATH = "/schemas";
const juce::String RemoteConfig::THUMBNAILS_PATH = "/assets/thumbnails";
const juce::String RemoteConfig::FACEPLATES_PATH = "/assets/faceplates";
const juce::String RemoteConfig::CONTROL_IMAGES_PATH = "/assets/controls";

juce::String RemoteConfig::getGearLibraryIndexUrl()
{
    return makeAbsoluteUrl(GEAR_LIBRARY_BASE_URL, GEAR_LIBRARY_INDEX_PATH);
}

juce::String RemoteConfig::getSchemaUrl(const juce::String &schemaPath)
{
    if (schemaPath.startsWith("/"))
        return makeAbsoluteUrl(SCHEMAS_BASE_URL, schemaPath);
    else
        return makeAbsoluteUrl(SCHEMAS_BASE_URL, "/" + schemaPath);
}

juce::String RemoteConfig::getThumbnailUrl(const juce::String &thumbnailPath)
{
    if (thumbnailPath.startsWith("/"))
        return makeAbsoluteUrl(THUMBNAILS_BASE_URL, thumbnailPath);
    else
        return makeAbsoluteUrl(THUMBNAILS_BASE_URL, "/" + thumbnailPath);
}

juce::String RemoteConfig::getFaceplateUrl(const juce::String &faceplatePath)
{
    if (faceplatePath.startsWith("/"))
        return makeAbsoluteUrl(FACEPLATES_BASE_URL, faceplatePath);
    else
        return makeAbsoluteUrl(FACEPLATES_BASE_URL, "/" + faceplatePath);
}

juce::String RemoteConfig::getControlImageUrl(const juce::String &controlImagePath)
{
    if (controlImagePath.startsWith("/"))
        return makeAbsoluteUrl(CONTROL_IMAGES_BASE_URL, controlImagePath);
    else
        return makeAbsoluteUrl(CONTROL_IMAGES_BASE_URL, "/" + controlImagePath);
}

bool RemoteConfig::isValidUrl(const juce::String &url)
{
    return url.startsWith("http://") || url.startsWith("https://");
}

juce::String RemoteConfig::makeAbsoluteUrl(const juce::String &baseUrl, const juce::String &relativePath)
{
    // Ensure base URL doesn't end with slash
    juce::String cleanBase = baseUrl;
    if (cleanBase.endsWith("/"))
        cleanBase = cleanBase.dropLastCharacters(1);

    // Ensure relative path starts with slash
    juce::String cleanPath = relativePath;
    if (!cleanPath.startsWith("/"))
        cleanPath = "/" + cleanPath;

    return cleanBase + cleanPath;
}
