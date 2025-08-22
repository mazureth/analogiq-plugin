#pragma once

#include "../Shared/GearControl.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/INetworkFetcher.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

// Forward declarations
class CacheManager;

/**
 * Comprehensive individual gear instances with sophisticated control systems.
 * Provides complete gear metadata management, instance management, and advanced image handling.
 */
class GearItem
{
public:
    enum class GearType
    {
        Series500,
        Rack19Inch,
        UserCreated,
        Other
    };

    enum class GearCategory
    {
        EQ,
        Compressor,
        Preamp,
        Other
    };

    // Constructor
    GearItem();
    GearItem(const juce::String& unitId, const juce::String& name, const juce::String& manufacturer);
    GearItem(const GearItem& other);
    ~GearItem();

    // Assignment operator
    GearItem& operator=(const GearItem& other);

    // Core properties
    juce::String unitId;
    juce::String name;
    juce::String manufacturer;
    juce::String version;
    juce::String description;
    GearType type;
    GearCategory category;
    juce::String categoryString;
    juce::String schemaPath;

    // Image assets
    juce::Image thumbnailImage;
    juce::Image faceplateImage;

    // Control system
    juce::Array<GearControl> controls;
    juce::StringArray tags;

    // Instance management
    bool isInstance;
    juce::String instanceId;
    juce::String sourceUnitId;

    // Dependencies (injected)
    IFileSystem* fileSystem;
    INetworkFetcher* networkFetcher;
    CacheManager* cacheManager;

    // Core methods
    void setFileSystem(IFileSystem* fs) { fileSystem = fs; }
    void setNetworkFetcher(INetworkFetcher* nf) { networkFetcher = nf; }
    void setCacheManager(CacheManager* cm) { cacheManager = cm; }

    // Image loading and management
    bool loadImage();
    void createPlaceholderImage();
    juce::Colour getCategoryColor() const;

    // Instance management
    GearItem createInstance() const;
    void resetToSource();
    bool isInstanceOf(const GearItem& source) const;

    // JSON serialization
    bool saveToFile(const juce::String& filePath) const;
    bool loadFromFile(const juce::String& filePath);

    // Control management
    void addControl(const GearControl& control);
    GearControl* getControl(int index);
    int getNumControls() const;
    void resetAllControls();

    // Utility methods
    static GearItem getDummy();
    juce::String getTypeString() const;
    juce::String getCategoryString() const;
    void setTypeFromString(const juce::String& typeStr);
    void setCategoryFromString(const juce::String& categoryStr);

private:
    void initializeDefaults();
    void copyImages(const GearItem& other);
    void clearImages();
    juce::String generateInstanceId() const;
};
