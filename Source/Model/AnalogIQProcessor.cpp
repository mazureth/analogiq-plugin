#include "AnalogIQProcessor.h"
#include "NetworkFetcher.h"
#include "FileSystem.h"
#include "CacheManager.h"
#include "PresetManager.h"
#include "GearLibrary.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

// Forward declarations for components that will be implemented
class CacheManager;
class PresetManager;
class GearLibrary;
class Rack;

AnalogIQProcessor::AnalogIQProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters", createParameterLayout()), undoManager(std::make_unique<juce::UndoManager>()), lastCreatedEditor(nullptr), storedRackReference(nullptr), networkFetcher(nullptr), fileSystem(nullptr)
{
    // Initialize logging
    initializeLogging();

    // Log initialization
    logToFile("AnalogIQProcessor initialized");
}

AnalogIQProcessor::AnalogIQProcessor(INetworkFetcher &nf, IFileSystem &fs)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters", createParameterLayout()), undoManager(std::make_unique<juce::UndoManager>()), lastCreatedEditor(nullptr), storedRackReference(nullptr), networkFetcher(&nf), fileSystem(&fs), cacheManager(std::make_unique<CacheManager>(fs)), presetManager(std::make_unique<PresetManager>(fs)), gearLibrary(std::make_unique<GearLibrary>(fs, *cacheManager))
{
    // Initialize logging
    initializeLogging();

    // Log initialization
    logToFile("AnalogIQProcessor initialized with dependencies");
}

AnalogIQProcessor::~AnalogIQProcessor()
{
    logToFile("AnalogIQProcessor destructor called");

    // Clear stored references
    storedRackReference = nullptr;
    lastCreatedEditor = nullptr;
}

// JUCE AudioProcessor methods
void AnalogIQProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    logToFile("prepareToPlay called - Sample Rate: " + juce::String(sampleRate) +
              ", Block Size: " + juce::String(samplesPerBlock));
}

void AnalogIQProcessor::releaseResources()
{
    logToFile("releaseResources called");
}

void AnalogIQProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    // This is a pass-through plugin - no audio processing
    // Just copy input to output
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float *inputChannel = buffer.getReadPointer(channel);
        float *outputChannel = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            outputChannel[sample] = inputChannel[sample];
        }
    }
}

bool AnalogIQProcessor::isBusesLayoutSupported(const BusesLayout &busesLayout) const
{
    // Support mono and stereo configurations
    if (busesLayout.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        busesLayout.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (busesLayout.getMainInputChannelSet() != busesLayout.getMainOutputChannelSet())
        return false;

    return true;
}

juce::AudioProcessorEditor *AnalogIQProcessor::createEditor()
{
    logToFile("createEditor called");

    // For now, return nullptr until we implement the editor
    // This will be implemented in Phase 2
    return nullptr;
}

bool AnalogIQProcessor::hasEditor() const
{
    return false; // Will be true when editor is implemented
}

const juce::String AnalogIQProcessor::getName() const
{
    return "AnalogIQ";
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

void AnalogIQProcessor::setCurrentProgram(int index)
{
    // No program support
}

const juce::String AnalogIQProcessor::getProgramName(int index)
{
    return "Default";
}

void AnalogIQProcessor::changeProgramName(int index, const juce::String &newName)
{
    // No program support
}

void AnalogIQProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    logToFile("getStateInformation called");

    try
    {
        auto xml = state.copyState().createXml();
        if (xml != nullptr)
        {
            copyXmlToBinary(*xml, destData);
            logXmlContent(*xml);
        }
    }
    catch (const std::exception &e)
    {
        logToFile("Error in getStateInformation: " + juce::String(e.what()));
    }
}

void AnalogIQProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    logToFile("setStateInformation called - Size: " + juce::String(sizeInBytes) + " bytes");

    try
    {
        auto xml = getXmlFromBinary(data, sizeInBytes);
        if (xml != nullptr)
        {
            state.replaceState(juce::ValueTree::fromXml(*xml));
            logXmlContent(*xml);
        }
    }
    catch (const std::exception &e)
    {
        logToFile("Error in setStateInformation: " + juce::String(e.what()));
    }
}

// State management interface
juce::AudioProcessorValueTreeState &AnalogIQProcessor::getState()
{
    return state;
}

void AnalogIQProcessor::saveInstanceState()
{
    logToFile("saveInstanceState called");
    // Will be implemented when Rack is available
}

void AnalogIQProcessor::loadInstanceState()
{
    logToFile("loadInstanceState called");
    // Will be implemented when Rack is available
}

void AnalogIQProcessor::saveInstanceStateFromRack(Rack *rack)
{
    logToFile("saveInstanceStateFromRack called");
    // Will be implemented when Rack is available
}

void AnalogIQProcessor::loadInstanceState(Rack *rack)
{
    logToFile("loadInstanceState called with rack");
    // Will be implemented when Rack is available
}

void AnalogIQProcessor::resetAllInstances()
{
    logToFile("resetAllInstances called");
    // Will be implemented when Rack is available
}

void AnalogIQProcessor::clearRackReference()
{
    logToFile("clearRackReference called");
    storedRackReference = nullptr;
}

// Private methods
juce::AudioProcessorValueTreeState::ParameterLayout AnalogIQProcessor::createParameterLayout()
{
    // Create a basic parameter layout
    // This will be expanded as we implement more features
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Add any parameters here as needed

    return {params.begin(), params.end()};
}

void AnalogIQProcessor::initializeLogging()
{
    try
    {
        // Create AnalogIQ directory in user documents
        auto userDocsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        auto analogiqDir = userDocsDir.getChildFile("AnalogIQ");

        if (!analogiqDir.exists())
            analogiqDir.createDirectory();

        logToFile("Logging initialized - Directory: " + analogiqDir.getFullPathName());
    }
    catch (const std::exception &e)
    {
        // Log to console if file logging fails
        juce::Logger::writeToLog("Failed to initialize logging: " + juce::String(e.what()));
    }
}

void AnalogIQProcessor::logToFile(const juce::String &message)
{
    try
    {
        auto userDocsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        auto analogiqDir = userDocsDir.getChildFile("AnalogIQ");
        auto logFile = analogiqDir.getChildFile("serialization.log");

        auto timestamp = getLogTimestamp();
        auto logEntry = timestamp + " - " + message + "\n";

        if (logFile.existsAsFile())
            logFile.appendText(logEntry);
        else
            logFile.replaceWithText(logEntry);
    }
    catch (...)
    {
        // Silently fail if logging fails
    }
}

juce::String AnalogIQProcessor::getLogTimestamp()
{
    auto now = juce::Time::getCurrentTime();
    return now.toString(true, true, true, true);
}

void AnalogIQProcessor::logStateTreeStructure(const juce::ValueTree &tree, int depth)
{
    auto indent = juce::String::repeatedString("  ", depth);
    auto nodeInfo = indent + "Node: " + tree.getType().toString();

    if (tree.getNumProperties() > 0)
    {
        nodeInfo += " Properties:";
        for (auto i = 0; i < tree.getNumProperties(); ++i)
        {
            auto name = tree.getPropertyName(i);
            auto value = tree.getProperty(name);
            nodeInfo += " " + name.toString() + "=" + value.toString();
        }
    }

    logToFile(nodeInfo);

    for (auto i = 0; i < tree.getNumChildren(); ++i)
    {
        logStateTreeStructure(tree.getChild(i), depth + 1);
    }
}

void AnalogIQProcessor::logXmlContent(const juce::XmlElement &xml)
{
    logToFile("XML Content: " + xml.toString());
}

// Plugin factory function
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    // Create static instances for dependency injection
    static auto networkFetcher = std::make_unique<NetworkFetcher>();
    static auto fileSystem = std::make_unique<FileSystem>();

    return new AnalogIQProcessor(*networkFetcher, *fileSystem);
}
