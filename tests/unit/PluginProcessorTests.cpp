#include <JuceHeader.h>
#include "PluginProcessor.h"

class PluginProcessorTests : public juce::UnitTest
{
public:
    PluginProcessorTests() : UnitTest("PluginProcessor Tests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            AnalogIQProcessor processor;
            expect(processor.getName() == "AnalogIQ");
        }

        beginTest("Default Parameters");
        {
            AnalogIQProcessor processor;
            const juce::Array<juce::AudioProcessorParameter *> &parameters = processor.getParameters();
            expect(parameters.size() > 0);
        }

        beginTest("State Management");
        {
            AnalogIQProcessor processor;

            // Test state saving
            juce::MemoryBlock state;
            processor.getStateInformation(state);
            expect(state.getSize() > 0);

            // Test state loading
            processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        }

        beginTest("Audio Processing");
        {
            AnalogIQProcessor processor;

            // Prepare audio buffer
            juce::AudioBuffer<float> buffer(2, 512); // 2 channels, 512 samples
            buffer.clear();

            // Create MIDI buffer
            juce::MidiBuffer midiBuffer;

            // Process audio
            processor.processBlock(buffer, midiBuffer);

            // Verify buffer wasn't modified (since we're not processing audio yet)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    expect(buffer.getSample(channel, sample) == 0.0f);
                }
            }
        }

        beginTest("Buses Configuration");
        {
            AnalogIQProcessor processor;

            // Check input/output configuration
            expect(processor.getBusCount(true) > 0);  // Input buses
            expect(processor.getBusCount(false) > 0); // Output buses

            // Check default bus arrangement
            expect(processor.getBus(true, 0)->isEnabled());  // Main input
            expect(processor.getBus(false, 0)->isEnabled()); // Main output
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static PluginProcessorTests pluginProcessorTests;