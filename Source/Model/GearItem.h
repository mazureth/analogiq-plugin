#pragma once

#include "../Shared/GearControl.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/INetworkFetcher.h"
#include "../Shared/ICacheManager.h"
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
    GearItem(const juce::String &unitId, const juce::String &name, const juce::String &manufacturer);
    GearItem(const juce::String &unitId, const juce::String &name, const juce::String &manufacturer,
             IFileSystem *fileSystem, INetworkFetcher *networkFetcher, ICacheManager *cacheManager);
    GearItem(const GearItem &other);
    ~GearItem();

    // Assignment operator
    GearItem &operator=(const GearItem &other);

    // Comprehensive constructor matching legacy pattern
    GearItem(const juce::String &unitId,
             const juce::String &name,
             const juce::String &manufacturer,
             const juce::String &category,
             const juce::String &version,
             const juce::String &schemaPath,
             const juce::String &thumbnailImage,
             const juce::StringArray &tags,
             GearType gearType,
             GearCategory gearCategory,
             int slotSize,
             IFileSystem *fileSystem,
             INetworkFetcher *networkFetcher,
             ICacheManager *cacheManager);

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

    // Remote gear library support
    juce::String imageUrl;
    juce::String schemaUrl;
    juce::StringArray dependencies;
    juce::String compatibilityVersion;

    // Image assets
    juce::Image thumbnailImage;
    juce::String faceplateImagePath;
    juce::Image faceplateImage;

    // Control system
    juce::Array<GearControl> controls;
    juce::StringArray tags;

    // Instance management
    bool isInstance;
    juce::String instanceId;
    juce::String sourceUnitId;

    // Dependencies (injected)
    IFileSystem *fileSystem;
    INetworkFetcher *networkFetcher;
    ICacheManager *cacheManager;

    // Core methods
    void setFileSystem(IFileSystem *fs) { fileSystem = fs; }
    void setNetworkFetcher(INetworkFetcher *nf) { networkFetcher = nf; }
    void setCacheManager(ICacheManager *cm) { cacheManager = cm; }

    // Image loading and management
    bool loadImage();
    void createPlaceholderImage();
    juce::Colour getCategoryColor() const;

    // Instance management
    GearItem createInstance() const;
    void resetToSource();
    bool isInstanceOf(const GearItem &source) const;

    // JSON serialization
    bool saveToFile(const juce::String &filePath) const;
    bool loadFromFile(const juce::String &filePath);

    // Control management
    void addControl(const GearControl &control);
    GearControl *getControl(int index);
    int getNumControls() const;
    void resetAllControls();

    // Utility methods
    static GearItem getDummy();
    juce::String getTypeString() const;
    juce::String getCategoryString() const;
    void setTypeFromString(const juce::String &typeStr);
    void setCategoryFromString(const juce::String &categoryStr);

private:
    void initializeDefaults();
    void copyImages(const GearItem &other);
    void clearImages();
    juce::String generateInstanceId() const;
};
