/**
 * @file GearItem.h
 * @brief Core data structures for representing gear items and their controls in the AnalogIQ system.
 *
 * This file defines the fundamental data structures used throughout the application:
 * - GearType and GearCategory enums for classifying gear items
 * - GearControl class for representing individual controls (knobs, faders, switches, buttons)
 * - GearItem class for representing complete gear units with their controls and metadata
 *
 * The implementation supports:
 * - Multiple control types with type-specific properties
 * - Instance management for creating variations of gear items
 * - Image and sprite sheet handling for visual representation
 * - JSON serialization for saving and loading gear configurations
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>

/**
 * @brief Enumeration of supported gear types.
 *
 * Defines the physical form factor or mounting type of gear items.
 */
enum class GearType
{
    Series500,   ///< 500 series format modules
    Rack19Inch,  ///< Standard 19-inch rack mount units
    UserCreated, ///< User-defined custom gear items
    Other        ///< Other or unspecified gear types
};

/**
 * @brief Enumeration of gear categories.
 *
 * Defines the functional category or purpose of gear items.
 */
enum class GearCategory
{
    EQ,         ///< Equalizers and tone shaping devices
    Compressor, ///< Dynamic range processors
    Preamp,     ///< Preamplifiers and gain stages
    Other       ///< Other or unspecified categories
};

/**
 * @brief Represents a single control on a gear item.
 *
 * The GearControl class encapsulates all properties and behaviors of a control,
 * including its type, position, value, and visual representation. It supports
 * multiple control types (knobs, faders, switches, buttons) with type-specific
 * properties and behaviors.
 */
class GearControl
{
public:
    /**
     * @brief Structure for defining frames in sprite sheets.
     *
     * Used for both switch and button controls to define the position and
     * dimensions of each state's frame in the sprite sheet image.
     */
    struct SwitchOptionFrame
    {
        int x = 0;          ///< X coordinate of the frame in the sprite sheet
        int y = 0;          ///< Y coordinate of the frame in the sprite sheet
        int width = 0;      ///< Width of the frame
        int height = 0;     ///< Height of the frame
        juce::String value; ///< Value associated with this frame
        juce::String label; ///< Display label for this frame
    };

    /**
     * @brief Enumeration of supported control types.
     */
    enum class Type
    {
        Button, ///< Push button control
        Fader,  ///< Linear fader control
        Switch, ///< Multi-position switch control
        Knob    ///< Rotary knob control
    };

    // Add default constructor
    GearControl()
        : type(Type::Button),
          name(""),
          id(""),
          position(0.0f, 0.0f, 0.0f, 0.0f),
          value(0.0f),
          initialValue(0.0f),
          currentIndex(0),
          orientation("vertical"),
          startAngle(0.0f),
          endAngle(360.0f),
          currentStepIndex(0),
          length(100),
          momentary(false)
    {
    }

    GearControl(Type typeParam, const juce::String &nameParam, const juce::Rectangle<float> &positionParam)
        : type(typeParam),
          name(nameParam),
          position(positionParam),
          value(0.0f),
          initialValue(0.0f),
          momentary(false) {}

    // Add copy constructor
    GearControl(const GearControl &other)
        : type(other.type),
          name(other.name),
          id(other.id),
          position(other.position),
          value(other.value),
          initialValue(other.value),
          options(other.options),
          currentIndex(other.currentIndex),
          orientation(other.orientation),
          image(other.image),
          startAngle(other.startAngle),
          endAngle(other.endAngle),
          steps(other.steps),
          currentStepIndex(other.currentStepIndex),
          loadedImage(other.loadedImage),
          switchFrames(other.switchFrames),
          switchSpriteSheet(other.switchSpriteSheet),
          length(other.length),
          faderImage(other.faderImage),
          momentary(other.momentary),
          buttonFrames(other.buttonFrames),
          buttonSpriteSheet(other.buttonSpriteSheet)
    {
        DBG("GearControl copy constructor called for control: " + name + " with ID: " + id);
    }

    // Add assignment operator
    GearControl &operator=(const GearControl &other)
    {
        if (this != &other)
        {
            type = other.type;
            name = other.name;
            id = other.id;
            position = other.position;
            value = other.value;
            initialValue = other.initialValue;
            options = other.options;
            currentIndex = other.currentIndex;
            orientation = other.orientation;
            image = other.image;
            startAngle = other.startAngle;
            endAngle = other.endAngle;
            steps = other.steps;
            currentStepIndex = other.currentStepIndex;
            loadedImage = other.loadedImage;
            switchFrames = other.switchFrames;
            switchSpriteSheet = other.switchSpriteSheet;
            length = other.length;
            faderImage = other.faderImage;
            momentary = other.momentary;
            buttonFrames = other.buttonFrames;
            buttonSpriteSheet = other.buttonSpriteSheet;
            DBG("GearControl assignment operator called for control: " + name + " with ID: " + id);
        }
        return *this;
    }

    Type type;
    juce::String name;
    juce::String id; // Unique identifier for the control
    juce::Rectangle<float> position;
    float value;
    float initialValue; // Store the original value from schema

    // Additional properties for switches
    juce::StringArray options;
    int currentIndex = 0;
    juce::String orientation = "vertical";       // "horizontal" or "vertical"
    juce::String image;                          // URI to the sprite sheet image
    juce::Array<SwitchOptionFrame> switchFrames; // Store frame data for each switch position
    juce::Image switchSpriteSheet;               // The loaded sprite sheet image for switches

    // Additional properties for knobs
    float startAngle = 0.0f;  // Starting angle in degrees (0-360)
    float endAngle = 360.0f;  // Ending angle in degrees (0-360)
    juce::Array<float> steps; // Array of rotation degrees for stepped knobs
    int currentStepIndex = 0; // Index of current step (only used with steps)
    juce::Image loadedImage;  // The actual loaded knob image

    // Additional properties for faders
    int length = 100;       // Length of the fader track in pixels
    juce::Image faderImage; // The loaded fader image

    // Additional properties for buttons
    bool momentary = false;                      // Whether the button is momentary or latching
    juce::Array<SwitchOptionFrame> buttonFrames; // Store frame data for button states
    juce::Image buttonSpriteSheet;               // The loaded sprite sheet image for buttons
};

/**
 * @brief Represents a complete gear item with its controls and metadata.
 *
 * The GearItem class encapsulates all properties and behaviors of a gear unit,
 * including its metadata (name, manufacturer, type), controls, and visual
 * representation. It supports instance management for creating variations
 * of gear items and JSON serialization for saving/loading configurations.
 */
class GearItem
{
public:
    GearItem() = default;

    GearItem(const juce::String &unitIdParam,
             const juce::String &nameParam,
             const juce::String &manufacturerParam,
             const juce::String &categoryParam,
             const juce::String &versionParam,
             const juce::String &schemaPathParam,
             const juce::String &thumbnailImageParam,
             const juce::StringArray &tagsParam,
             GearType typeParam = GearType::Other,
             GearCategory gearCategoryParam = GearCategory::Other,
             int slotSizeParam = 1,
             const juce::Array<GearControl> &controlsParam = {})
        : unitId(unitIdParam),
          name(nameParam),
          manufacturer(manufacturerParam),
          type(typeParam),
          category(gearCategoryParam),
          slotSize(slotSizeParam),
          version(versionParam),
          schemaPath(schemaPathParam),
          thumbnailImage(thumbnailImageParam),
          categoryString(categoryParam),
          tags(tagsParam),
          controls(controlsParam)
    {
        // Map category string to enum (for backward compatibility)
        if (categoryString == "equalizer" || categoryString == "eq")
            category = GearCategory::EQ;
        else if (categoryString == "compressor")
            category = GearCategory::Compressor;
        else if (categoryString == "preamp")
            category = GearCategory::Preamp;
        else
            category = GearCategory::Other;

        // Try to determine type from tags
        if (tags.contains("500 series"))
            type = GearType::Series500;
        else if (tags.contains("rack") || tags.contains("19 inch"))
            type = GearType::Rack19Inch;
    }

    // Original constructor for backward compatibility
    GearItem(const juce::String &nameParam,
             const juce::String &manufacturerParam,
             GearType typeParam,
             GearCategory categoryParam,
             int slotSizeParam,
             const juce::String &imageUrlParam,
             const juce::Array<GearControl> &controlsParam)
        : unitId(nameParam.toLowerCase().replaceCharacter(' ', '-')),
          name(nameParam),
          manufacturer(manufacturerParam),
          type(typeParam),
          category(categoryParam),
          slotSize(slotSizeParam),
          version("1.0.0"),
          schemaPath(""),
          thumbnailImage(imageUrlParam),
          categoryString(""),
          tags(),
          controls(controlsParam)
    {
        // Map category enum to string
        switch (category)
        {
        case GearCategory::EQ:
            categoryString = "equalizer";
            break;
        case GearCategory::Compressor:
            categoryString = "compressor";
            break;
        case GearCategory::Preamp:
            categoryString = "preamp";
            break;
        case GearCategory::Other:
            categoryString = "other";
            break;
        }
    }

    // Add instance management fields
    juce::String instanceId;   // Unique identifier for this instance
    bool isInstance = false;   // Whether this is an instance of another item
    juce::String sourceUnitId; // The unitId of the source item if this is an instance

    // Add instance management methods
    /**
     * @brief Creates a new instance of this gear item.
     *
     * Creates a copy of this gear item that shares the same source data
     * but maintains its own state. The instance will have a unique ID
     * and will be linked to this item as its source.
     *
     * @param sourceUnitId The unit ID of the source gear item.
     */
    void createInstance(const juce::String &sourceUnitId);

    /**
     * @brief Resets this instance to match its source item's state.
     *
     * If this is an instance of another gear item, resets all control
     * values to match the source item's state.
     */
    void resetToSource();

    /**
     * @brief Checks if this item is an instance of a specific gear item.
     *
     * @param unitId The unit ID to check against.
     * @return true if this is an instance of the specified gear item.
     */
    bool isInstanceOf(const juce::String &unitId) const { return isInstance && sourceUnitId == unitId; }

    // New fields from updated schema
    juce::String unitId;
    juce::String name;
    juce::String manufacturer;
    GearType type;
    GearCategory category;
    int slotSize;
    juce::String version;
    juce::String schemaPath;
    juce::String thumbnailImage;
    juce::String categoryString;
    juce::StringArray tags;
    juce::Image image;
    juce::String faceplateImagePath;
    juce::Image faceplateImage;
    juce::Array<GearControl> controls;

    /**
     * @brief Attempts to load the gear item's image.
     *
     * Loads the faceplate or thumbnail image for the gear item.
     * The image path is determined by the schema or thumbnailImage property.
     *
     * @return true if the image was successfully loaded.
     */
    bool loadImage();

    /**
     * @brief Saves the gear item configuration to a JSON file.
     *
     * Serializes the gear item's properties and control states to JSON format
     * and saves them to the specified file.
     *
     * @param destinationFile The file to save the configuration to.
     */
    void saveToJSON(juce::File destinationFile);

    /**
     * @brief Loads a gear item configuration from a JSON file.
     *
     * Creates a new GearItem instance from a saved JSON configuration file.
     *
     * @param sourceFile The JSON file to load the configuration from.
     * @return A new GearItem instance with the loaded configuration.
     */
    static GearItem loadFromJSON(juce::File sourceFile);

    /**
     * @brief Creates a placeholder image for the gear item.
     *
     * Generates a simple placeholder image when the actual image
     * cannot be loaded. The placeholder includes the gear item's name
     * and basic visual elements.
     *
     * @return true if the placeholder was successfully created.
     */
    bool createPlaceholderImage();

    // Copy constructor
    GearItem(const GearItem &other)
        : unitId(other.unitId),
          name(other.name),
          manufacturer(other.manufacturer),
          categoryString(other.categoryString),
          version(other.version),
          schemaPath(other.schemaPath),
          thumbnailImage(other.thumbnailImage),
          tags(other.tags),
          type(other.type),
          category(other.category),
          slotSize(other.slotSize),
          controls(other.controls),
          image(other.image),
          faceplateImage(other.faceplateImage),
          isInstance(false),           // New instances start as non-instances
          instanceId(juce::String()),  // New instances get a new ID
          sourceUnitId(juce::String()) // New instances start with no source
    {
        // Load the image
        loadImage();
    }

private:
    // Helper method to create a placeholder image
    bool createPlaceholderImage();
};