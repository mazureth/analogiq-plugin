#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"

class RackSlotTests : public juce::UnitTest
{
public:
    RackSlotTests() : UnitTest("RackSlotTests") {}

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

        // Set up mock response for the LA-2A schema
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

        beginTest("Initial State");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(0);
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Gear Item Management");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(0);

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

            auto item = std::make_unique<GearItem>(
                "la2a-compressor",
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

            slot.setGearItem(item.get());
            expect(!slot.isAvailable(), "Slot should not be available");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Gear item name should match");
            expect(slot.getGearItem()->manufacturer == "Universal Audio", "Manufacturer should match");
            expect(slot.getGearItem()->category == GearCategory::Compressor, "Category should be Compressor");
            expect(slot.getGearItem()->type == GearType::Rack19Inch, "Type should be Rack19Inch");
        }

        beginTest("Clear Gear Item");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(0);

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

            auto item = std::make_unique<GearItem>(
                "la2a-compressor",
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

            slot.setGearItem(item.get());
            slot.clearGearItem();
            expect(slot.isAvailable(), "Slot should be available");
        }

        beginTest("Instance Management");
        {
            setUpMocks(mockFetcher);
            RackSlot slot(0);

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

            auto gearItem = std::make_unique<GearItem>(
                "la2a-compressor",
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

            slot.setGearItem(gearItem.get());
            slot.createInstance();
            expect(slot.isInstance(), "Slot should be an instance");
            expect(!slot.getInstanceId().isEmpty(), "Instance ID should not be empty");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged");
            expect(slot.getGearItem()->manufacturer == "Universal Audio", "Manufacturer should remain unchanged");

            // Store the instance ID before reset
            auto instanceId = slot.getInstanceId();

            // Reset the instance
            slot.resetToSource();

            // Verify instance state is preserved
            expect(slot.isInstance(), "Slot should still be an instance after reset");
            expect(slot.getInstanceId() == instanceId, "Instance ID should be preserved after reset");
            expect(slot.getGearItem()->name == "LA-2A Tube Compressor", "Name should remain unchanged after reset");
        }
    }
};

static RackSlotTests rackSlotTestsInstance;