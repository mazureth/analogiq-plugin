#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogIQProcessor::AnalogIQProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, &undoManager, "Parameters", {})
{
}

AnalogIQProcessor::~AnalogIQProcessor()
{
}

const juce::String AnalogIQProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnalogIQProcessor::acceptsMidi() const
{
    return false;
}

bool AnalogIQProcessor::producesMidi() const
{
    return false;
}

bool AnalogIQProcessor::isMidiEffect() const
{
    return false;
}

double AnalogIQProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnalogIQProcessor::getNumPrograms()
{
    return 1;
}

int AnalogIQProcessor::getCurrentProgram()
{
    return 0;
}

void AnalogIQProcessor::setCurrentProgram(int /*index*/)
{
    // No program support
}

const juce::String AnalogIQProcessor::getProgramName(int /*index*/)
{
    return {};
}

void AnalogIQProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
    // No program support
}

void AnalogIQProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // No prep needed, we're not processing audio
}

void AnalogIQProcessor::releaseResources()
{
}

bool AnalogIQProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void AnalogIQProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    // We're not doing audio processing in this plugin as it's for settings/documentation only
    // Just pass audio through
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

bool AnalogIQProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AnalogIQProcessor::createEditor()
{
    return static_cast<juce::AudioProcessorEditor*>(new AnalogIQEditor(*this));
}

void AnalogIQProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto stateSnapshot = getState().copyState();
    std::unique_ptr<juce::XmlElement> xml(stateSnapshot.createXml());
    copyXmlToBinary(*xml, destData);
}

void AnalogIQProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(getState().state.getType()))
            getState().replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogIQProcessor();
} 