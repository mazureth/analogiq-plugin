/**
 * @file PluginProcessor.cpp
 * @brief Implementation of the AnalogIQProcessor class.
 *
 * This file implements the audio processor for the AnalogIQ plugin,
 * handling audio processing, state management, and instance control.
 * The processor manages the plugin's state and coordinates between
 * the editor interface and the audio processing system.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

/**
 * @brief Constructs a new AnalogIQProcessor.
 *
 * Initializes the processor with stereo input and output buses,
 * and sets up the state management system.
 */
AnalogIQProcessor::AnalogIQProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, &undoManager, "Parameters", {})
{
}

/**
 * @brief Destructor for AnalogIQProcessor.
 */
AnalogIQProcessor::~AnalogIQProcessor()
{
}

/**
 * @brief Gets the name of the plugin.
 *
 * @return The plugin name as defined in JucePlugin_Name
 */
const juce::String AnalogIQProcessor::getName() const
{
    return JucePlugin_Name;
}

/**
 * @brief Checks if the plugin accepts MIDI input.
 *
 * @return false as this plugin does not process MIDI
 */
bool AnalogIQProcessor::acceptsMidi() const
{
    return false;
}

/**
 * @brief Checks if the plugin produces MIDI output.
 *
 * @return false as this plugin does not produce MIDI
 */
bool AnalogIQProcessor::producesMidi() const
{
    return false;
}

/**
 * @brief Checks if the plugin is a MIDI effect.
 *
 * @return false as this is not a MIDI effect
 */
bool AnalogIQProcessor::isMidiEffect() const
{
    return false;
}

/**
 * @brief Gets the tail length in seconds.
 *
 * @return 0.0 as this plugin has no tail
 */
double AnalogIQProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

/**
 * @brief Gets the number of programs.
 *
 * @return 1 as this plugin has no program support
 */
int AnalogIQProcessor::getNumPrograms()
{
    return 1;
}

/**
 * @brief Gets the current program index.
 *
 * @return 0 as this plugin has no program support
 */
int AnalogIQProcessor::getCurrentProgram()
{
    return 0;
}

/**
 * @brief Sets the current program.
 *
 * @param index The program index (unused)
 */
void AnalogIQProcessor::setCurrentProgram(int /*index*/)
{
    // No program support
}

/**
 * @brief Gets the name of a program.
 *
 * @param index The program index (unused)
 * @return Empty string as this plugin has no program support
 */
const juce::String AnalogIQProcessor::getProgramName(int /*index*/)
{
    return {};
}

/**
 * @brief Changes the name of a program.
 *
 * @param index The program index (unused)
 * @param newName The new program name (unused)
 */
void AnalogIQProcessor::changeProgramName(int /*index*/, const juce::String & /*newName*/)
{
    // No program support
}

/**
 * @brief Prepares the processor for playback.
 *
 * @param sampleRate The sample rate (unused)
 * @param samplesPerBlock The block size (unused)
 */
void AnalogIQProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // No prep needed, we're not processing audio
}

/**
 * @brief Releases resources used by the processor.
 */
void AnalogIQProcessor::releaseResources()
{
}

/**
 * @brief Checks if a bus layout is supported.
 *
 * @param layouts The bus layout to check
 * @return true if the layout is supported (mono or stereo)
 */
bool AnalogIQProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

/**
 * @brief Processes a block of audio data.
 *
 * @param buffer The audio buffer to process
 * @param midiMessages The MIDI messages (unused)
 */
void AnalogIQProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer & /*midiMessages*/)
{
    // We're not doing audio processing in this plugin as it's for settings/documentation only
    // Just pass audio through
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

/**
 * @brief Checks if the plugin has an editor.
 *
 * @return true as this plugin has an editor
 */
bool AnalogIQProcessor::hasEditor() const
{
    return true;
}

/**
 * @brief Creates the plugin's editor.
 *
 * @return A new AnalogIQEditor instance
 */
juce::AudioProcessorEditor *AnalogIQProcessor::createEditor()
{
    return static_cast<juce::AudioProcessorEditor *>(new AnalogIQEditor(*this));
}

/**
 * @brief Saves the plugin's state.
 *
 * @param destData The memory block to save the state to
 */
void AnalogIQProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto stateSnapshot = getState().copyState();
    std::unique_ptr<juce::XmlElement> xml(stateSnapshot.createXml());
    copyXmlToBinary(*xml, destData);
}

/**
 * @brief Restores the plugin's state.
 *
 * @param data Pointer to the state data
 * @param sizeInBytes Size of the state data
 */
void AnalogIQProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(getState().state.getType()))
            getState().replaceState(juce::ValueTree::fromXml(*xmlState));
}

/**
 * @brief Saves the current state of all gear instances.
 *
 * Saves the state of each gear instance in the rack, including
 * control values and settings, to the plugin's state tree.
 */
void AnalogIQProcessor::saveInstanceState()
{
    // Create a child tree for instance state
    auto instanceTree = state.state.getOrCreateChildWithName("instances", nullptr);

    // Clear existing instance data
    instanceTree.removeAllChildren(nullptr);

    // Get the rack from the editor
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            // Save instance data for each slot
            for (int i = 0; i < rack->getNumSlots(); ++i)
            {
                if (rack->isInstance(i))
                {
                    auto slotTree = instanceTree.getOrCreateChildWithName("slot_" + juce::String(i), nullptr);
                    slotTree.setProperty("instanceId", rack->getInstanceId(i), nullptr);

                    // Get the gear item for this slot
                    if (auto *slot = rack->getSlot(i))
                    {
                        if (auto *item = slot->getGearItem())
                        {
                            // Save control values
                            auto controlsTree = slotTree.getOrCreateChildWithName("controls", nullptr);
                            for (int j = 0; j < item->controls.size(); ++j)
                            {
                                const auto &control = item->controls[j];
                                auto controlTree = controlsTree.getOrCreateChildWithName("control_" + juce::String(j), nullptr);
                                controlTree.setProperty("value", control.value, nullptr);
                                controlTree.setProperty("initialValue", control.initialValue, nullptr);
                                if (control.type == GearControl::Type::Switch)
                                {
                                    controlTree.setProperty("currentIndex", control.currentIndex, nullptr);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Loads the state of all gear instances.
 *
 * Restores the state of each gear instance in the rack from
 * the plugin's state tree, including control values and settings.
 */
void AnalogIQProcessor::loadInstanceState()
{
    // Get the instance state tree
    auto instanceTree = state.state.getChildWithName("instances");
    if (!instanceTree.isValid())
        return;

    // Get the rack from the editor
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            // Load instance data for each slot
            for (int i = 0; i < rack->getNumSlots(); ++i)
            {
                auto slotTree = instanceTree.getChildWithName("slot_" + juce::String(i));
                if (slotTree.isValid())
                {
                    // Create instance if we have saved state
                    rack->createInstance(i);

                    // Get the gear item for this slot
                    if (auto *slot = rack->getSlot(i))
                    {
                        if (auto *item = slot->getGearItem())
                        {
                            // Load control values
                            auto controlsTree = slotTree.getChildWithName("controls");
                            if (controlsTree.isValid())
                            {
                                for (int j = 0; j < item->controls.size(); ++j)
                                {
                                    auto controlTree = controlsTree.getChildWithName("control_" + juce::String(j));
                                    if (controlTree.isValid())
                                    {
                                        auto &control = item->controls.getReference(j);
                                        control.value = controlTree.getProperty("value", control.value);
                                        control.initialValue = controlTree.getProperty("initialValue", control.initialValue);
                                        if (control.type == GearControl::Type::Switch)
                                        {
                                            control.currentIndex = controlTree.getProperty("currentIndex", control.currentIndex);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Resets all gear instances to their default state.
 *
 * Calls resetAllInstances() on the rack to restore all gear
 * instances to their default settings.
 */
void AnalogIQProcessor::resetAllInstances()
{
    // Get the rack from the editor
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            rack->resetAllInstances();
        }
    }
}

/**
 * @brief Creates a new instance of the plugin.
 *
 * @return A new AnalogIQProcessor instance
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogIQProcessor();
}