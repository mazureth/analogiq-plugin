#include "GearItem.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

GearItem::GearItem()
    : unitId(""), name(""), manufacturer(""), version("1.0"), description(""), type(GearType::Other), category(GearCategory::Other), categoryString(""), schemaPath(""), thumbnailImage(), faceplateImage(), controls(), tags(), isInstance(false), instanceId(""), sourceUnitId(""), fileSystem(nullptr), networkFetcher(nullptr), cacheManager(nullptr)
{
    initializeDefaults();
}

GearItem::GearItem(const juce::String &id, const juce::String &gearName, const juce::String &maker)
    : unitId(id), name(gearName), manufacturer(maker), version("1.0"), description(""), type(GearType::Other), category(GearCategory::Other), categoryString(""), schemaPath(""), thumbnailImage(), faceplateImage(), controls(), tags(), isInstance(false), instanceId(""), sourceUnitId(""), fileSystem(nullptr), networkFetcher(nullptr), cacheManager(nullptr)
{
    initializeDefaults();
}

GearItem::GearItem(const juce::String &id, const juce::String &gearName, const juce::String &maker,
                   IFileSystem *fs, INetworkFetcher *nf, ICacheManager *cm)
    : unitId(id), name(gearName), manufacturer(maker), version("1.0"), description(""), type(GearType::Other), category(GearCategory::Other), categoryString(""), schemaPath(""), thumbnailImage(), faceplateImage(), controls(), tags(), isInstance(false), instanceId(""), sourceUnitId(""), fileSystem(fs), networkFetcher(nf), cacheManager(cm)
{
    initializeDefaults();
}

GearItem::GearItem(const GearItem &other)
    : unitId(other.unitId), name(other.name), manufacturer(other.manufacturer), version(other.version), description(other.description), type(other.type), category(other.category), categoryString(other.categoryString), schemaPath(other.schemaPath), thumbnailImage(other.thumbnailImage), faceplateImage(other.faceplateImage), controls(other.controls), tags(other.tags), isInstance(other.isInstance), instanceId(other.instanceId), sourceUnitId(other.sourceUnitId), fileSystem(other.fileSystem), networkFetcher(other.networkFetcher), cacheManager(other.cacheManager)
{
    copyImages(other);
}

GearItem::~GearItem()
{
    clearImages();
}

GearItem &GearItem::operator=(const GearItem &other)
{
    if (this != &other)
    {
        unitId = other.unitId;
        name = other.name;
        manufacturer = other.manufacturer;
        version = other.version;
        description = other.description;
        type = other.type;
        category = other.category;
        categoryString = other.categoryString;
        schemaPath = other.schemaPath;
        controls = other.controls;
        tags = other.tags;
        isInstance = other.isInstance;
        instanceId = other.instanceId;
        sourceUnitId = other.sourceUnitId;
        fileSystem = other.fileSystem;
        networkFetcher = other.networkFetcher;
        cacheManager = other.cacheManager;

        copyImages(other);
    }
    return *this;
}

void GearItem::initializeDefaults()
{
    if (categoryString.isEmpty())
    {
        categoryString = getCategoryString();
    }

    if (schemaPath.isEmpty())
    {
        schemaPath = "schemas/default.json";
    }
}

void GearItem::copyImages(const GearItem &other)
{
    thumbnailImage = other.thumbnailImage;
    faceplateImage = other.faceplateImage;
}

void GearItem::clearImages()
{
    thumbnailImage = juce::Image();
    faceplateImage = juce::Image();
}

bool GearItem::loadImage()
{
    if (!fileSystem)
        return false;

    // Try to load thumbnail first
    auto thumbnailPath = fileSystem->joinPath(fileSystem->getCacheRootDirectory(),
                                              "thumbnails/" + unitId + ".png");
    if (fileSystem->fileExists(thumbnailPath))
    {
        auto imageData = fileSystem->readBinaryFile(thumbnailPath);
        thumbnailImage = juce::ImageCache::getFromMemory(imageData.getData(), imageData.getSize());
        return true;
    }

    // If no thumbnail, create placeholder
    createPlaceholderImage();
    return false;
}

void GearItem::createPlaceholderImage()
{
    // Create a simple colored rectangle as placeholder
    thumbnailImage = juce::Image(juce::Image::RGB, 64, 64, true);
    juce::Graphics g(thumbnailImage);
    g.setColour(getCategoryColor());
    g.fillAll();

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText(name, 0, 0, 64, 64, juce::Justification::centred, true);
}

juce::Colour GearItem::getCategoryColor() const
{
    switch (category)
    {
    case GearCategory::EQ:
        return juce::Colours::blue;
    case GearCategory::Compressor:
        return juce::Colours::red;
    case GearCategory::Preamp:
        return juce::Colours::green;
    case GearCategory::Other:
    default:
        return juce::Colours::grey;
    }
}

GearItem GearItem::createInstance() const
{
    GearItem instance(*this);
    instance.isInstance = true;
    instance.instanceId = generateInstanceId();
    instance.sourceUnitId = unitId;
    instance.unitId = instance.instanceId;
    return instance;
}

void GearItem::resetToSource()
{
    if (!isInstance || sourceUnitId.isEmpty())
        return;

    // Reset to source values
    unitId = sourceUnitId;
    isInstance = false;
    instanceId = "";
    sourceUnitId = "";
}

bool GearItem::isInstanceOf(const GearItem &source) const
{
    return isInstance && sourceUnitId == source.unitId;
}

bool GearItem::saveToFile(const juce::String &filePath) const
{
    if (!fileSystem)
        return false;

    // Create a simple JSON-like format for now
    juce::String content = "{\n";
    content += "  \"unitId\": \"" + unitId + "\",\n";
    content += "  \"name\": \"" + name + "\",\n";
    content += "  \"manufacturer\": \"" + manufacturer + "\",\n";
    content += "  \"version\": \"" + version + "\",\n";
    content += "  \"description\": \"" + description + "\",\n";
    content += "  \"type\": " + juce::String(static_cast<int>(type)) + ",\n";
    content += "  \"category\": " + juce::String(static_cast<int>(category)) + ",\n";
    content += "  \"categoryString\": \"" + categoryString + "\",\n";
    content += "  \"schemaPath\": \"" + schemaPath + "\",\n";
    content += "  \"isInstance\": " + juce::String(isInstance ? "true" : "false") + ",\n";
    content += "  \"instanceId\": \"" + instanceId + "\",\n";
    content += "  \"sourceUnitId\": \"" + sourceUnitId + "\"\n";
    content += "}";

    return fileSystem->writeFile(filePath, content);
}

bool GearItem::loadFromFile(const juce::String &filePath)
{
    if (!fileSystem || !fileSystem->fileExists(filePath))
        return false;

    auto content = fileSystem->readFile(filePath);
    // Simple parsing - in production would use proper JSON parser
    auto lines = juce::StringArray::fromLines(content);

    for (auto &line : lines)
    {
        line = line.trim();
        if (line.startsWith("\"unitId\":"))
            unitId = line.substring(10).trim().removeCharacters("\",");
        else if (line.startsWith("\"name\":"))
            name = line.substring(8).trim().removeCharacters("\",");
        else if (line.startsWith("\"manufacturer\":"))
            manufacturer = line.substring(15).trim().removeCharacters("\",");
        else if (line.startsWith("\"version\":"))
            version = line.substring(11).trim().removeCharacters("\",");
        else if (line.startsWith("\"description\":"))
            description = line.substring(15).trim().removeCharacters("\",");
        else if (line.startsWith("\"type\":"))
            type = static_cast<GearType>(line.substring(8).trim().removeCharacters(",").getIntValue());
        else if (line.startsWith("\"category\":"))
            category = static_cast<GearCategory>(line.substring(12).trim().removeCharacters(",").getIntValue());
        else if (line.startsWith("\"categoryString\":"))
            categoryString = line.substring(18).trim().removeCharacters("\",");
        else if (line.startsWith("\"schemaPath\":"))
            schemaPath = line.substring(14).trim().removeCharacters("\",");
        else if (line.startsWith("\"isInstance\":"))
            isInstance = line.substring(14).trim().removeCharacters(",") == "true";
        else if (line.startsWith("\"instanceId\":"))
            instanceId = line.substring(14).trim().removeCharacters("\",");
        else if (line.startsWith("\"sourceUnitId\":"))
            sourceUnitId = line.substring(16).trim().removeCharacters("\",");
    }

    return true;
}

void GearItem::addControl(const GearControl &control)
{
    controls.add(control);
}

GearControl *GearItem::getControl(int index)
{
    if (index >= 0 && index < controls.size())
        return &controls.getReference(index);
    return nullptr;
}

int GearItem::getNumControls() const
{
    return controls.size();
}

void GearItem::resetAllControls()
{
    for (auto &control : controls)
    {
        control.resetToDefault();
    }
}

GearItem GearItem::getDummy()
{
    return GearItem("dummy", "Dummy Gear", "Dummy Manufacturer");
}

juce::String GearItem::getTypeString() const
{
    switch (type)
    {
    case GearType::Series500:
        return "Series 500";
    case GearType::Rack19Inch:
        return "19\" Rack";
    case GearType::UserCreated:
        return "User Created";
    case GearType::Other:
    default:
        return "Other";
    }
}

juce::String GearItem::getCategoryString() const
{
    switch (category)
    {
    case GearCategory::EQ:
        return "EQ";
    case GearCategory::Compressor:
        return "Compressor";
    case GearCategory::Preamp:
        return "Preamp";
    case GearCategory::Other:
    default:
        return "Other";
    }
}

void GearItem::setTypeFromString(const juce::String &typeStr)
{
    if (typeStr.containsIgnoreCase("500") || typeStr.containsIgnoreCase("series"))
        type = GearType::Series500;
    else if (typeStr.containsIgnoreCase("rack") || typeStr.containsIgnoreCase("19"))
        type = GearType::Rack19Inch;
    else if (typeStr.containsIgnoreCase("user") || typeStr.containsIgnoreCase("created"))
        type = GearType::UserCreated;
    else
        type = GearType::Other;
}

void GearItem::setCategoryFromString(const juce::String &categoryStr)
{
    if (categoryStr.containsIgnoreCase("eq") || categoryStr.containsIgnoreCase("equalizer"))
        category = GearCategory::EQ;
    else if (categoryStr.containsIgnoreCase("comp") || categoryStr.containsIgnoreCase("compressor"))
        category = GearCategory::Compressor;
    else if (categoryStr.containsIgnoreCase("pre") || categoryStr.containsIgnoreCase("preamp"))
        category = GearCategory::Preamp;
    else
        category = GearCategory::Other;
}

juce::String GearItem::generateInstanceId() const
{
    auto timestamp = juce::Time::getCurrentTime().toMilliseconds();
    return unitId + "_inst_" + juce::String(timestamp);
}

GearItem::GearItem(const juce::String &id,
                   const juce::String &gearName,
                   const juce::String &maker,
                   const juce::String &gearCategory,
                   const juce::String &gearVersion,
                   const juce::String &gearSchemaPath,
                   const juce::String &gearThumbnailImage,
                   const juce::StringArray &gearTags,
                   GearType gearType,
                   GearCategory gearCategoryEnum,
                   int gearSlotSize,
                   IFileSystem *fs,
                   INetworkFetcher *nf,
                   ICacheManager *cm)
    : unitId(id), name(gearName), manufacturer(maker), version(gearVersion), description(""), type(gearType), category(gearCategoryEnum), categoryString(gearCategory), schemaPath(gearSchemaPath), imageUrl(gearThumbnailImage), schemaUrl(""), dependencies(), compatibilityVersion(""), thumbnailImage(), faceplateImage(), controls(), tags(gearTags), isInstance(false), instanceId(""), sourceUnitId(""), fileSystem(fs), networkFetcher(nf), cacheManager(cm)
{
    // Create placeholder image if thumbnail is not available
    if (gearThumbnailImage.isEmpty())
    {
        createPlaceholderImage();
    }
}
