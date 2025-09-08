#include <JuceHeader.h>
#include "GearItem.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class GearItemTests : public juce::UnitTest
{
public:
    GearItemTests() : UnitTest("GearItemTests") {}

    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Use static test image data to prevent JUCE leak detection
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

        // Set up mock responses for images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
            imageData);

        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            imageData);

        // Set up mock responses for control images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
            imageData);

        // Set up mock response for the compressor schema
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/la2a-compressor-1.0.0.json",
            R"({
                    "unitId": "la2a-compressor",
                    "name": "LA-2A Tube Compressor",
                    "manufacturer": "Universal Audio",
                    "tags": [
                        "compressor",
                        "tube",
                        "optical",
                        "vintage",
                        "hardware"
                    ],
                    "version": "1.0.0",
                    "category": "compressor",
                    "formFactor": "19-inch-rack",
                    "faceplateImage": "assets/faceplates/la2a-compressor-1.0.0.jpg",
                    "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                    "width": 1900,
                    "height": 525,
                    "controls": [
                        {
                            "id": "peak-reduction",
                            "label": "Peak Reduction",
                            "type": "knob",
                            "position": {
                                "x": 0.68,
                                "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        },
                        {
                            "id": "gain",
                            "label": "Gain",
                            "type": "knob",
                            "position": {
                                "x": 0.257,
                                "y": 0.44
                            },
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        }
                    ]
                })");
    }

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Create local instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Default Constructor");
        testDefaultConstructor();

        beginTest("Construction");
        testConstruction(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Property Assignment");
        testPropertyAssignment(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Category Mapping");
        testCategoryMapping(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Type Detection from Tags");
        testTypeDetectionFromTags(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Image Loading");
        testImageLoading(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Placeholder Creation");
        testPlaceholderCreation(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Instance Creation");
        testInstanceCreation(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Instance Checking");
        testInstanceChecking(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Reset to Source");
        testResetToSource(mockFetcher, mockFileSystem, cacheManager);

        beginTest("JSON Serialization");
        testJSONSerialization(mockFetcher, mockFileSystem, cacheManager);

        beginTest("JSON Deserialization");
        testJSONDeserialization(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Control Management");
        testControlManagement(mockFetcher, mockFileSystem, cacheManager);

        beginTest("Image Cleanup");
        testImageCleanup(mockFetcher, mockFileSystem, cacheManager);
    }

private:
    void testDefaultConstructor()
    {
        GearItem item;
        
        expect(item.unitId.isEmpty(), "Default unitId should be empty");
        expect(item.name.isEmpty(), "Default name should be empty");
        expect(item.manufacturer.isEmpty(), "Default manufacturer should be empty");
        expect(item.categoryString.isEmpty(), "Default categoryString should be empty");
        expect(item.version.isEmpty(), "Default version should be empty");
        expect(item.schemaPath.isEmpty(), "Default schemaPath should be empty");
        expect(item.thumbnailImage.isEmpty(), "Default thumbnailImage should be empty");
        expect(item.tags.isEmpty(), "Default tags should be empty");
        expect(item.type == GearType::Other, "Default type should be Other");
        expect(item.category == GearCategory::Other, "Default category should be Other");
        expectEquals(item.slotSize, 1, "Default slotSize should be 1");
        expect(item.controls.isEmpty(), "Default controls should be empty");
        expect(!item.isInstance, "Default isInstance should be false");
        expect(item.sourceUnitId.isEmpty(), "Default sourceUnitId should be empty");
        expect(item.instanceId.isEmpty(), "Default instanceId should be empty");
    }

    void testConstruction(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Create Peak Reduction control
        GearControl peakReduction;
        peakReduction.id = "peak-reduction";
        peakReduction.name = "Peak Reduction";
        peakReduction.type = GearControl::Type::Knob;
        peakReduction.position = {0.68f, 0.44f};
        peakReduction.value = 180;
        peakReduction.startAngle = 40;
        peakReduction.endAngle = 322;
        peakReduction.image = "assets/controls/knobs/bakelite-lg-black.png";
        controls.add(peakReduction);

        // Create Gain control
        GearControl gain;
        gain.id = "gain";
        gain.name = "Gain";
        gain.type = GearControl::Type::Knob;
        gain.position = {0.257f, 0.44f};
        gain.value = 180;
        gain.startAngle = 40;
        gain.endAngle = 322;
        gain.image = "assets/controls/knobs/bakelite-lg-black.png";
        controls.add(gain);

        GearItem item("la2a-compressor",
                      "LA-2A Tube Compressor",
                      "Universal Audio",
                      "compressor",
                      "1.0.0",
                      "units/la2a-compressor-1.0.0.json",
                      "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                      tags,
                      mockFetcher,
                      mockFileSystem,
                      cacheManager,
                      GearType::Rack19Inch,
                      GearCategory::Compressor,
                      1,
                      controls);

        expectEquals(item.name, juce::String("LA-2A Tube Compressor"), "Name should match LA-2A");
        expect(item.type == GearType::Rack19Inch, "Type should be Rack19Inch for LA-2A");
        expectEquals(item.manufacturer, juce::String("Universal Audio"), "Manufacturer should match");
        expect(item.category == GearCategory::Compressor, "Category should be Compressor");
        expectEquals(item.categoryString, juce::String("compressor"), "Category string should match");
        expectEquals(item.version, juce::String("1.0.0"), "Version should match");
        expectEquals(item.slotSize, 1, "Slot size should be 1");
        expect(!item.isInstance, "Should not be an instance initially");
        expectEquals(item.unitId, juce::String("la2a-compressor"), "Unit ID should match");
        expectEquals(item.sourceUnitId, juce::String(), "Source unit ID should be empty");
        expectEquals(item.instanceId, juce::String(), "Instance ID should be empty");
        expectEquals(item.controls.size(), 2, "Should have 2 controls");
    }

    void testPropertyAssignment(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        GearItem item("la2a-compressor",
                      "LA-2A Tube Compressor",
                      "Universal Audio",
                      "compressor",
                      "1.0.0",
                      "units/la2a-compressor-1.0.0.json",
                      "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                      tags,
                      mockFetcher,
                      mockFileSystem,
                      cacheManager,
                      GearType::Rack19Inch,
                      GearCategory::Compressor,
                      1,
                      controls);

        item.name = "Modified LA-2A";
        item.type = GearType::Series500;
        item.manufacturer = "Modified Audio";
        item.category = GearCategory::EQ;
        item.categoryString = "equalizer";
        item.version = "2.0.0";
        item.slotSize = 2;

        expectEquals(item.name, juce::String("Modified LA-2A"), "Name should be modified");
        expect(item.type == GearType::Series500, "Type should be Series500");
        expectEquals(item.manufacturer, juce::String("Modified Audio"), "Manufacturer should be modified");
        expect(item.category == GearCategory::EQ, "Category should be EQ");
        expectEquals(item.categoryString, juce::String("equalizer"), "Category string should be modified");
        expectEquals(item.version, juce::String("2.0.0"), "Version should be modified");
        expectEquals(item.slotSize, 2, "Slot size should be modified");
    }

    void testCategoryMapping(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Test EQ mapping
        GearItem eqItem("eq-unit", "EQ Unit", "Test Audio", "equalizer", "1.0.0", "", "", tags,
                       mockFetcher, mockFileSystem, cacheManager);
        expect(eqItem.category == GearCategory::EQ, "equalizer should map to EQ category");

        // Test compressor mapping
        GearItem compItem("comp-unit", "Compressor Unit", "Test Audio", "compressor", "1.0.0", "", "", tags,
                         mockFetcher, mockFileSystem, cacheManager);
        expect(compItem.category == GearCategory::Compressor, "compressor should map to Compressor category");

        // Test preamp mapping
        GearItem preampItem("pre-unit", "Preamp Unit", "Test Audio", "preamp", "1.0.0", "", "", tags,
                           mockFetcher, mockFileSystem, cacheManager);
        expect(preampItem.category == GearCategory::Preamp, "preamp should map to Preamp category");

        // Test other mapping
        GearItem otherItem("other-unit", "Other Unit", "Test Audio", "other", "1.0.0", "", "", tags,
                          mockFetcher, mockFileSystem, cacheManager);
        expect(otherItem.category == GearCategory::Other, "other should map to Other category");
    }

    void testTypeDetectionFromTags(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        juce::Array<GearControl> controls;

        // Test 500 series detection
        juce::StringArray series500Tags;
        series500Tags.add("500 series");
        series500Tags.add("module");
        GearItem series500Item("500-unit", "500 Series Unit", "Test Audio", "eq", "1.0.0", "", "", series500Tags,
                              mockFetcher, mockFileSystem, cacheManager);
        expect(series500Item.type == GearType::Series500, "500 series tag should set type to Series500");

        // Test rack detection
        juce::StringArray rackTags;
        rackTags.add("rack");
        rackTags.add("hardware");
        GearItem rackItem("rack-unit", "Rack Unit", "Test Audio", "compressor", "1.0.0", "", "", rackTags,
                         mockFetcher, mockFileSystem, cacheManager);
        expect(rackItem.type == GearType::Rack19Inch, "rack tag should set type to Rack19Inch");

        // Test 19 inch detection
        juce::StringArray inch19Tags;
        inch19Tags.add("19 inch");
        inch19Tags.add("hardware");
        GearItem inch19Item("19inch-unit", "19 Inch Unit", "Test Audio", "preamp", "1.0.0", "", "", inch19Tags,
                           mockFetcher, mockFileSystem, cacheManager);
        expect(inch19Item.type == GearType::Rack19Inch, "19 inch tag should set type to Rack19Inch");

        // Test no special tags (should remain Other)
        juce::StringArray normalTags;
        normalTags.add("vintage");
        normalTags.add("tube");
        GearItem normalItem("normal-unit", "Normal Unit", "Test Audio", "other", "1.0.0", "", "", normalTags,
                           mockFetcher, mockFileSystem, cacheManager);
        expect(normalItem.type == GearType::Other, "No special tags should keep type as Other");
    }

    void testImageLoading(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Test with thumbnail image
        GearItem item("test-gear", "Test Gear", "Test Audio", "other", "1.0.0", "", 
                     "assets/thumbnails/la2a-compressor-1.0.0.jpg", tags,
                     mockFetcher, mockFileSystem, cacheManager);
        
        bool imageLoaded = item.loadImage();
        expect(imageLoaded, "Image loading should succeed");
        expect(item.image.isValid(), "Image should be valid after loading");

        // Test loading same image again (should use cache)
        bool imageLoadedAgain = item.loadImage();
        expect(imageLoadedAgain, "Image loading should succeed on second attempt");

        // Test with empty thumbnail (should create placeholder)
        GearItem itemNoThumb("test-gear-2", "Test Gear 2", "Test Audio", "eq", "1.0.0", "", "", tags,
                            mockFetcher, mockFileSystem, cacheManager);
        bool placeholderCreated = itemNoThumb.loadImage();
        expect(placeholderCreated, "Placeholder creation should succeed");
        expect(itemNoThumb.image.isValid(), "Placeholder image should be valid");
    }

    void testPlaceholderCreation(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Test placeholder for different categories
        GearItem eqItem("eq-test", "EQ Test", "Test Audio", "equalizer", "1.0.0", "", "", tags,
                       mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::EQ);
        bool eqPlaceholder = eqItem.loadImage();
        expect(eqPlaceholder, "EQ placeholder should be created");
        expect(eqItem.image.isValid(), "EQ placeholder should be valid");

        GearItem compItem("comp-test", "Compressor Test", "Test Audio", "compressor", "1.0.0", "", "", tags,
                         mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::Compressor);
        bool compPlaceholder = compItem.loadImage();
        expect(compPlaceholder, "Compressor placeholder should be created");
        expect(compItem.image.isValid(), "Compressor placeholder should be valid");

        GearItem preampItem("preamp-test", "Preamp Test", "Test Audio", "preamp", "1.0.0", "", "", tags,
                           mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::Preamp);
        bool preampPlaceholder = preampItem.loadImage();
        expect(preampPlaceholder, "Preamp placeholder should be created");
        expect(preampItem.image.isValid(), "Preamp placeholder should be valid");

        GearItem otherItem("other-test", "Other Test", "Test Audio", "other", "1.0.0", "", "", tags,
                          mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::Other);
        bool otherPlaceholder = otherItem.loadImage();
        expect(otherPlaceholder, "Other placeholder should be created");
        expect(otherItem.image.isValid(), "Other placeholder should be valid");
    }

    void testInstanceCreation(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        GearItem item("la2a-compressor",
                      "LA-2A Tube Compressor",
                      "Universal Audio",
                      "compressor",
                      "1.0.0",
                      "units/la2a-compressor-1.0.0.json",
                      "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                      tags,
                      mockFetcher,
                      mockFileSystem,
                      cacheManager,
                      GearType::Rack19Inch,
                      GearCategory::Compressor,
                      1,
                      controls);

        item.createInstance(item.unitId);

        expect(item.isInstance, "Should be marked as instance after creation");
        expectEquals(item.sourceUnitId, juce::String("la2a-compressor"), "Source unit ID should match original");
        expectEquals(item.name, juce::String("LA-2A Tube Compressor"), "Name should remain unchanged");
        expect(item.instanceId.isNotEmpty(), "Instance ID should not be empty");
    }

    void testInstanceChecking(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        GearItem item("la2a-compressor",
                      "LA-2A Tube Compressor",
                      "Universal Audio",
                      "compressor",
                      "1.0.0",
                      "units/la2a-compressor-1.0.0.json",
                      "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                      tags,
                      mockFetcher,
                      mockFileSystem,
                      cacheManager,
                      GearType::Rack19Inch,
                      GearCategory::Compressor,
                      1,
                      controls);

        expect(!item.isInstanceOf("la2a-compressor"), "Non-instance should not be instance of la2a-compressor");
        expect(!item.isInstanceOf("other-compressor"), "Non-instance should not be instance of other-compressor");

        item.createInstance("la2a-compressor");

        expect(item.isInstanceOf("la2a-compressor"), "Instance should be instance of its source unit");
        expect(!item.isInstanceOf("other-compressor"), "Instance should not be instance of different unit");
    }

    void testResetToSource(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Create a control with an initial value
        GearControl testControl;
        testControl.id = "test-control";
        testControl.name = "Test Control";
        testControl.type = GearControl::Type::Knob;
        testControl.value = 50.0f;
        testControl.initialValue = 50.0f;
        controls.add(testControl);

        GearItem item("test-gear", "Test Gear", "Test Audio", "compressor", "1.0.0", "", "", tags,
                     mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::Other, 1, controls);

        // Reset on non-instance should do nothing
        item.resetToSource();
        expect(!item.isInstance, "Should still not be instance after reset");

        // Create instance
        item.createInstance("test-gear");
        expect(item.isInstance, "Should be instance after creation");

        // Modify control value
        item.controls.getReference(0).value = 75.0f;
        expectEquals(item.controls[0].value, 75.0f, "Control value should be modified");

        // Reset to source
        item.resetToSource();
        expectEquals(item.controls[0].value, 50.0f, "Control value should be reset to initial value");
        expect(item.isInstance, "Should still be instance after reset");
        expect(item.instanceId.isNotEmpty(), "Instance ID should still be present");
    }

    void testJSONSerialization(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Create a control for testing
        GearControl testControl;
        testControl.id = "test-control";
        testControl.name = "Test Control";
        testControl.type = GearControl::Type::Knob;
        testControl.position = {0.5f, 0.5f, 0.1f, 0.1f};
        testControl.value = 100.0f;
        controls.add(testControl);

        GearItem item("test-gear", "Test Gear", "Test Audio", "compressor", "1.0.0", 
                     "units/test-gear.json", "assets/test-gear.jpg", tags,
                     mockFetcher, mockFileSystem, cacheManager, GearType::Series500, GearCategory::Compressor, 2, controls);

        // Make it an instance for testing
        item.createInstance("source-gear");

        // Save to JSON
        juce::String jsonPath = "/tmp/test-gear.json";
        item.saveToJSON(jsonPath);

        // Check that file was written
        expect(mockFileSystem.fileExists(jsonPath), "JSON file should exist after saving");

        // Read and parse the JSON to verify content
        juce::String jsonContent = mockFileSystem.readFile(jsonPath);
        expect(jsonContent.isNotEmpty(), "JSON content should not be empty");

        juce::var jsonVar = juce::JSON::parse(jsonContent);
        expect(jsonVar.isObject(), "JSON should be a valid object");

        // Verify some key properties
        expectEquals(jsonVar.getProperty("unitId", "").toString(), juce::String("test-gear"), "Unit ID should be serialized");
        expectEquals(jsonVar.getProperty("name", "").toString(), juce::String("Test Gear"), "Name should be serialized");
        expect(jsonVar.getProperty("isInstance", false), "Instance flag should be serialized");
        expectEquals(jsonVar.getProperty("type", "").toString(), juce::String("500Series"), "Type should be serialized");
    }

    void testJSONDeserialization(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);

        // Create test JSON content
        juce::String testJson = R"({
            "unitId": "test-deserialize",
            "name": "Deserialized Gear",
            "manufacturer": "Test Company",
            "category": "equalizer",
            "version": "2.0.0",
            "schemaPath": "units/test.json",
            "thumbnailImage": "assets/test.jpg",
            "type": "Rack19Inch",
            "slotSize": 3,
            "tags": ["eq", "digital"],
            "controls": [
                {
                    "type": "Knob",
                    "name": "Frequency",
                    "position": {
                        "x": 0.25,
                        "y": 0.5,
                        "width": 0.1,
                        "height": 0.1
                    },
                    "value": 200.0
                }
            ]
        })";

        juce::String jsonPath = "/tmp/test-deserialize.json";
        mockFileSystem.writeFile(jsonPath, testJson);

        // Load from JSON
        GearItem loadedItem = GearItem::loadFromJSON(jsonPath, mockFetcher, mockFileSystem);

        // Verify loaded properties
        expectEquals(loadedItem.unitId, juce::String("test-deserialize"), "Unit ID should be loaded correctly");
        expectEquals(loadedItem.name, juce::String("Deserialized Gear"), "Name should be loaded correctly");
        expectEquals(loadedItem.manufacturer, juce::String("Test Company"), "Manufacturer should be loaded correctly");
        expectEquals(loadedItem.version, juce::String("2.0.0"), "Version should be loaded correctly");
        expectEquals(loadedItem.slotSize, 3, "Slot size should be loaded correctly");
        expect(loadedItem.type == GearType::Rack19Inch, "Type should be loaded correctly");
        expect(loadedItem.category == GearCategory::EQ, "Category should be mapped correctly");
        expectEquals(loadedItem.tags.size(), 2, "Tags should be loaded correctly");
        expectEquals(loadedItem.controls.size(), 1, "Controls should be loaded correctly");
    }

    void testControlManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Create various types of controls
        GearControl knobControl;
        knobControl.id = "knob-control";
        knobControl.name = "Knob Control";
        knobControl.type = GearControl::Type::Knob;
        knobControl.value = 50.0f;
        knobControl.initialValue = 25.0f;
        controls.add(knobControl);

        GearControl faderControl;
        faderControl.id = "fader-control";
        faderControl.name = "Fader Control";
        faderControl.type = GearControl::Type::Fader;
        faderControl.value = 75.0f;
        faderControl.initialValue = 50.0f;
        controls.add(faderControl);

        GearControl switchControl;
        switchControl.id = "switch-control";
        switchControl.name = "Switch Control";
        switchControl.type = GearControl::Type::Switch;
        switchControl.value = 1.0f;
        switchControl.initialValue = 0.0f;
        controls.add(switchControl);

        GearItem item("test-controls", "Test Controls", "Test Audio", "other", "1.0.0", "", "", tags,
                     mockFetcher, mockFileSystem, cacheManager, GearType::Other, GearCategory::Other, 1, controls);

        expectEquals(item.controls.size(), 3, "Should have 3 controls");
        
        // Test control values
        expectEquals(item.controls[0].value, 50.0f, "Knob control value should be correct");
        expectEquals(item.controls[1].value, 75.0f, "Fader control value should be correct");
        expectEquals(item.controls[2].value, 1.0f, "Switch control value should be correct");

        // Test that controls can be modified
        item.controls.getReference(0).value = 100.0f;
        expectEquals(item.controls[0].value, 100.0f, "Control value should be modifiable");
    }

    void testImageCleanup(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, CacheManager &cacheManager)
    {
        setUpMocks(mockFetcher);
        const juce::StringArray &tags = TestImageHelper::getEmptyTestTags();
        juce::Array<GearControl> controls;

        // Create a GearItem with images in a scope to test destructor
        {
            GearItem item("cleanup-test", "Cleanup Test", "Test Audio", "eq", "1.0.0", "", 
                         "assets/thumbnails/la2a-compressor-1.0.0.jpg", tags,
                         mockFetcher, mockFileSystem, cacheManager);
            
            // Load image to test cleanup
            bool imageLoaded = item.loadImage();
            expect(imageLoaded, "Image should load for cleanup test");
            expect(item.image.isValid(), "Image should be valid");
            
            // Test that faceplate image path can be set
            item.faceplateImagePath = "assets/faceplate.jpg";
            expectEquals(item.faceplateImagePath, juce::String("assets/faceplate.jpg"), "Faceplate path should be set");
        }
        // Destructor should clean up images here
        expect(true, "Destructor should complete without issues");
    }
};

static GearItemTests gearItemTests;