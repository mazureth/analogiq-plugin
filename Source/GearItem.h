/**
 * @file GearItem.h
 * @brief Header file for the GearItem class and related components.
 *
 * This file defines the GearItem class and its supporting classes for managing
 * audio gear items in the plugin. It includes definitions for gear types,
 * categories, and controls.
 */

#pragma once

#include <JuceHeader.h>
#include "INetworkFetcher.h"

/**
 * @brief Enumeration of possible gear types.
 *
 * Defines the different types of audio gear that can be represented
 * in the system, such as 500 series modules, rack units, etc.
 */
enum class GearType
{
    Series500,   ///< 500 series module
    Rack19Inch,  ///< 19-inch rack unit
    UserCreated, ///< User-created custom gear
    Other        ///< Other type of gear
};

/**
 * @brief Enumeration of gear categories.
 *
 * Defines the different functional categories of audio gear,
 * such as equalizers, compressors, and preamps.
 */
enum class GearCategory
{
    EQ,         ///< Equalizer
    Compressor, ///< Compressor
    Preamp,     ///< Preamp
    Other       ///< Other category
};

/**
 * @brief Class representing a control on a piece of gear.
 *
 * This class defines the properties and behavior of controls
 * such as knobs, faders, switches, and buttons on audio gear.
 */
class GearControl
{
public:
    /**
     * @brief Structure defining a frame in a switch or button sprite sheet.
     *
     * Contains position and size information for a single frame
     * in a multi-state control's sprite sheet.
     */
    struct SwitchOptionFrame
    {
        int x = 0;          ///< X position in sprite sheet
        int y = 0;          ///< Y position in sprite sheet
        int width = 0;      ///< Frame width
        int height = 0;     ///< Frame height
        juce::String value; ///< Value associated with this frame
        juce::String label; ///< Display label for this frame
    };

    /**
     * @brief Enumeration of control types.
     *
     * Defines the different types of controls that can be
     * represented in the system.
     */
    enum class Type
    {
        Button, ///< Push button control
        Fader,  ///< Slider/fader control
        Switch, ///< Toggle switch control
        Knob    ///< Rotary knob control
    };

    /**
     * @brief Default constructor.
     *
     * Initializes a control with default values.
     */
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

    /**
     * @brief Destructor.
     *
     * Cleans up any loaded images to prevent memory leaks.
     */
    ~GearControl()
    {
        loadedImage = juce::Image();
        switchSpriteSheet = juce::Image();
        faderImage = juce::Image();
        buttonSpriteSheet = juce::Image();
    }

    /**
     * @brief Constructor with basic parameters.
     *
     * @param typeParam The type of control
     * @param nameParam The name of the control
     * @param positionParam The position and size of the control
     */
    GearControl(Type typeParam, const juce::String &nameParam, const juce::Rectangle<float> &positionParam)
        : type(typeParam),
          name(nameParam),
          position(positionParam),
          value(0.0f),
          initialValue(0.0f),
          momentary(false) {}

    /**
     * @brief Copy constructor.
     *
     * @param other The control to copy from
     */
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
    }

    /**
     * @brief Assignment operator.
     *
     * @param other The control to assign from
     * @return Reference to this control
     */
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
        }
        return *this;
    }

    Type type;                       ///< The type of control
    juce::String name;               ///< The name of the control
    juce::String id;                 ///< Unique identifier for the control
    juce::Rectangle<float> position; ///< Position and size of the control
    float value;                     ///< Current value of the control
    float initialValue;              ///< Original value from schema

    // Additional properties for switches
    juce::StringArray options;                   ///< Available options for switch
    int currentIndex = 0;                        ///< Current selected option index
    juce::String orientation = "vertical";       ///< Control orientation
    juce::String image;                          ///< URI to the sprite sheet image
    juce::Array<SwitchOptionFrame> switchFrames; ///< Frame data for switch positions
    juce::Image switchSpriteSheet;               ///< Loaded sprite sheet for switches

    // Additional properties for knobs
    float startAngle = 0.0f;  ///< Starting angle in degrees
    float endAngle = 360.0f;  ///< Ending angle in degrees
    juce::Array<float> steps; ///< Rotation degrees for stepped knobs
    int currentStepIndex = 0; ///< Current step index
    juce::Image loadedImage;  ///< Loaded knob image

    // Additional properties for faders
    int length = 100;       ///< Length of fader track in pixels
    juce::Image faderImage; ///< Loaded fader image

    // Additional properties for buttons
    bool momentary = false;                      ///< Whether button is momentary
    juce::Array<SwitchOptionFrame> buttonFrames; ///< Frame data for button states
    juce::Image buttonSpriteSheet;               ///< Loaded sprite sheet for buttons
};

/**
 * @brief Class representing a piece of audio gear.
 *
 * This class manages the properties and behavior of audio gear items,
 * including their controls, images, and instance management.
 */
class GearItem
{
public:
    /**
     * @brief Default constructor.
     *
     * Initializes a gear item with default values.
     */
    GearItem()
        : unitId(""),
          name(""),
          manufacturer(""),
          categoryString(""),
          version(""),
          schemaPath(""),
          thumbnailImage(""),
          tags(),
          type(GearType::Other),
          category(GearCategory::Other),
          slotSize(1),
          controls(),
          isInstance(false),
          sourceUnitId(""),
          networkFetcher(INetworkFetcher::getDummy())
    {
    }

    /**
     * @brief Destructor.
     *
     * Cleans up any loaded images to prevent memory leaks.
     */
    ~GearItem()
    {
        // Clear the main images
        image = juce::Image();
        faceplateImage = juce::Image();

        // Clear images in controls
        for (auto &control : controls)
        {
            control.loadedImage = juce::Image();
            control.switchSpriteSheet = juce::Image();
            control.faderImage = juce::Image();
            control.buttonSpriteSheet = juce::Image();
        }
    }

    /**
     * @brief Constructor with all parameters.
     *
     * @param unitIdParam The unique identifier for the gear item
     * @param nameParam The name of the gear item
     * @param manufacturerParam The manufacturer of the gear item
     * @param categoryParam The category of the gear item
     * @param versionParam The version of the gear item
     * @param schemaPathParam The path to the schema file
     * @param thumbnailImageParam The path to the thumbnail image
     * @param tagsParam The tags associated with the gear item
     * @param networkFetcherParam The network fetcher to use for loading resources
     * @param typeParam The type of gear
     * @param gearCategoryParam The category of gear
     * @param slotSizeParam The size of the slot required
     * @param controlsParam The controls available on the gear
     */
    GearItem(const juce::String &unitIdParam,
             const juce::String &nameParam,
             const juce::String &manufacturerParam,
             const juce::String &categoryParam,
             const juce::String &versionParam,
             const juce::String &schemaPathParam,
             const juce::String &thumbnailImageParam,
             const juce::StringArray &tagsParam,
             INetworkFetcher &networkFetcherParam,
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
          controls(controlsParam),
          networkFetcher(networkFetcherParam)
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

    /**
     * @brief Legacy constructor for backward compatibility.
     *
     * @param nameParam Name of the gear
     * @param manufacturerParam Manufacturer name
     * @param typeParam Type of gear
     * @param categoryParam Category of gear
     * @param slotSizeParam Size in rack slots
     * @param imageUrlParam Path to image
     * @param controlsParam Array of controls
     * @param networkFetcherParam Reference to network fetcher
     */
    GearItem(const juce::String &nameParam,
             const juce::String &manufacturerParam,
             GearType typeParam,
             GearCategory categoryParam,
             int slotSizeParam,
             const juce::String &imageUrlParam,
             const juce::Array<GearControl> &controlsParam,
             INetworkFetcher &networkFetcherParam)
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
          controls(controlsParam),
          networkFetcher(networkFetcherParam)
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

    // Instance management fields
    juce::String instanceId;   ///< Unique identifier for this instance
    bool isInstance = false;   ///< Whether this is an instance of another item
    juce::String sourceUnitId; ///< The unitId of the source item if this is an instance

    // Add instance management methods
    void createInstance(const juce::String &sourceUnitId);
    void resetToSource();
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

    bool loadImage();
    void saveToJSON(juce::File destinationFile);
    static GearItem loadFromJSON(juce::File sourceFile, INetworkFetcher &networkFetcher);

    /**
     * @brief Copy constructor.
     *
     * Creates a new instance of GearItem with the same properties as the source.
     * The new instance is not marked as an instance of the source.
     *
     * @param other The GearItem to copy from
     * @param networkFetcherParam Reference to network fetcher
     */
    GearItem(const GearItem &other, INetworkFetcher &networkFetcherParam)
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
          isInstance(false),            // New instances start as non-instances
          instanceId(juce::String()),   // New instances get a new ID
          sourceUnitId(juce::String()), // New instances start with no source
          networkFetcher(networkFetcherParam)
    {
        // Load the image
        loadImage();
    }

private:
    INetworkFetcher &networkFetcher;

    /**
     * @brief Creates a placeholder image for the gear item.
     *
     * Generates a colored placeholder image based on the gear category,
     * with the first letter of the gear name displayed.
     *
     * @return true if placeholder was successfully created
     */
    bool createPlaceholderImage();
};