/**
 * @file PluginProcessor.h
 * @brief Header file for the AnalogIQProcessor class, which implements the audio processing functionality.
 *
 * The AnalogIQProcessor class is responsible for:
 * - Audio and MIDI processing
 * - Plugin state management
 * - Instance state persistence
 * - Parameter automation
 * - Program management
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>

/**
 * @class AnalogIQProcessor
 * @brief The main audio processor class for the AnalogIQ plugin.
 *
 * This class handles all audio processing and state management for the plugin,
 * including parameter automation, program management, and instance state persistence.
 */
class AnalogIQProcessor : public juce::AudioProcessor
{
public:
    /**
     * @brief Constructs a new AnalogIQProcessor.
     */
    AnalogIQProcessor();

    /**
     * @brief Destructor for AnalogIQProcessor.
     */
    ~AnalogIQProcessor() override;

    /**
     * @brief Prepares the processor for playback.
     * @param sampleRate The sample rate to use
     * @param samplesPerBlock The maximum block size to expect
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Releases any resources used by the processor.
     */
    void releaseResources() override;

    /**
     * @brief Checks if the given bus layout is supported.
     * @param layouts The bus layout to check
     * @return true if the layout is supported, false otherwise
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    /**
     * @brief Processes a block of audio and MIDI data.
     * @param buffer The audio buffer to process
     * @param midiMessages The MIDI messages to process
     */
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    /**
     * @brief Creates the plugin's editor component.
     * @return A new instance of the plugin's editor
     */
    juce::AudioProcessorEditor *createEditor() override;

    /**
     * @brief Checks if the plugin has an editor.
     * @return true as this plugin has an editor
     */
    bool hasEditor() const override;

    /**
     * @brief Gets the name of the plugin.
     * @return The plugin name
     */
    const juce::String getName() const override;

    /**
     * @brief Checks if the plugin accepts MIDI input.
     * @return true as this plugin accepts MIDI
     */
    bool acceptsMidi() const override;

    /**
     * @brief Checks if the plugin produces MIDI output.
     * @return true as this plugin can produce MIDI
     */
    bool producesMidi() const override;

    /**
     * @brief Checks if the plugin is a MIDI effect.
     * @return false as this is not a MIDI-only effect
     */
    bool isMidiEffect() const override;

    /**
     * @brief Gets the tail length in seconds.
     * @return The tail length
     */
    double getTailLengthSeconds() const override;

    /**
     * @brief Gets the number of programs.
     * @return The number of programs
     */
    int getNumPrograms() override;

    /**
     * @brief Gets the current program index.
     * @return The current program index
     */
    int getCurrentProgram() override;

    /**
     * @brief Sets the current program.
     * @param index The program index to set
     */
    void setCurrentProgram(int index) override;

    /**
     * @brief Gets the name of a program.
     * @param index The program index
     * @return The program name
     */
    const juce::String getProgramName(int index) override;

    /**
     * @brief Changes the name of a program.
     * @param index The program index
     * @param newName The new program name
     */
    void changeProgramName(int index, const juce::String &newName) override;

    /**
     * @brief Saves the plugin state.
     * @param destData The memory block to save to
     */
    void getStateInformation(juce::MemoryBlock &destData) override;

    /**
     * @brief Restores the plugin state.
     * @param data The state data to restore from
     * @param sizeInBytes The size of the state data
     */
    void setStateInformation(const void *data, int sizeInBytes) override;

    /**
     * @brief Gets the plugin's state tree.
     * @return Reference to the state tree
     */
    juce::AudioProcessorValueTreeState &getState() { return state; }

    /**
     * @brief Saves the current instance state.
     */
    void saveInstanceState();

    /**
     * @brief Loads the saved instance state.
     */
    void loadInstanceState();

    /**
     * @brief Resets all instances to their default state.
     */
    void resetAllInstances();

private:
    juce::AudioProcessorValueTreeState state; ///< The plugin's state tree
    juce::UndoManager undoManager;            ///< Manager for undo/redo operations

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQProcessor)
};