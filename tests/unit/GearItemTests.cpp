#include <JuceHeader.h>
#include "GearItem.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class GearItemTests : public juce::UnitTest
{
public:
    GearItemTests() : UnitTest("GearItemTests") {}

    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Create a test image for mocks
        juce::Image testImage(juce::Image::RGB, 24, 24, true);
        {
            juce::Graphics g(testImage);
            g.fillAll(juce::Colours::darkgrey);
            g.setColour(juce::Colours::white);
            g.drawText("Test", testImage.getBounds(), juce::Justification::centred, true);
        }

        // Convert to JPEG format
        juce::MemoryOutputStream stream;
        juce::JPEGImageFormat jpegFormat;
        jpegFormat.setQuality(0.8f);
        jpegFormat.writeImageToStream(testImage, stream);
        juce::MemoryBlock imageData(stream.getData(), stream.getDataSize());

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
        mockFetcher.reset();

        beginTest("Construction");
        {
            setUpMocks(mockFetcher);
            juce::StringArray tags = {"compressor", "tube", "optical", "vintage", "hardware"};
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

        beginTest("Property Assignment");
        {
            setUpMocks(mockFetcher);
            juce::StringArray tags = {"compressor", "tube", "optical", "vintage", "hardware"};
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

        beginTest("Instance Creation");
        {
            setUpMocks(mockFetcher);
            juce::StringArray tags = {"compressor", "tube", "optical", "vintage", "hardware"};
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

        beginTest("Instance Checking");
        {
            setUpMocks(mockFetcher);
            juce::StringArray tags = {"compressor", "tube", "optical", "vintage", "hardware"};
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
    }
};

static GearItemTests gearItemTests;