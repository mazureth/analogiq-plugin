/**
 * @file AnalogIQProcessor.h
 * @brief Header file for the AnalogIQProcessor class.
 *
 * This file defines the main audio processor for the AnalogIQ plugin,
 * handling audio processing, state management, and instance control.
 * The processor manages the plugin's state and coordinates between
 * the editor interface and the audio processing system.
 */

#pragma once

#include <JuceHeader.h>
#include "Rack.h"
#include "NetworkFetcher.h"
#include "CacheManager.h"
#include "PresetManager.h"
#include "FileSystem.h"

// Forward declare the test class
class AnalogIQProcessorTests;

/**
 * @brief Main audio processor for the AnalogIQ plugin.
 *
 * The AnalogIQProcessor class handles all audio processing and state management
 * for the plugin. It provides a non-destructive audio pass-through while managing
 * the state of gear instances and their settings.
 */
class AnalogIQProcessor : public juce::AudioProcessor
{
    friend class AnalogIQProcessorTests; // Allow test class to access private members

public:
    /**
     * @brief Constructs a new AnalogIQProcessor.
     *
     * @param networkFetcher Reference to the network fetcher to use
     * @param fileSystem Reference to the file system to use
     */
    AnalogIQProcessor(INetworkFetcher &networkFetcher, IFileSystem &fileSystem);

    /**
     * @brief Destructor for AnalogIQProcessor.
     */
    ~AnalogIQProcessor() override;

    /**
     * @brief Prepares the processor for playback.
     *
     * @param sampleRate The sample rate
     * @param samplesPerBlock The block size
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Releases resources used by the processor.
     */
    void releaseResources() override;

    /**
     * @brief Checks if a bus layout is supported.
     *
     * @param layouts The bus layout to check
     * @return true if the layout is supported
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    /**
     * @brief Processes a block of audio data.
     *
     * @param buffer The audio buffer to process
     * @param midiMessages The MIDI messages
     */
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    /**
     * @brief Creates the plugin's editor.
     *
     * @return A new AnalogIQEditor instance
     */
    juce::AudioProcessorEditor *createEditor() override;

    /**
     * @brief Checks if the plugin has an editor.
     *
     * @return true as this plugin has an editor
     */
    bool hasEditor() const override;

    /**
     * @brief Gets the name of the plugin.
     *
     * @return The plugin name
     */
    const juce::String getName() const override;

    /**
     * @brief Checks if the plugin accepts MIDI input.
     *
     * @return false as this plugin does not process MIDI
     */
    bool acceptsMidi() const override;

    /**
     * @brief Checks if the plugin produces MIDI output.
     *
     * @return false as this plugin does not produce MIDI
     */
    bool producesMidi() const override;

    /**
     * @brief Checks if the plugin is a MIDI effect.
     *
     * @return false as this is not a MIDI effect
     */
    bool isMidiEffect() const override;

    /**
     * @brief Gets the tail length in seconds.
     *
     * @return 0.0 as this plugin has no tail
     */
    double getTailLengthSeconds() const override;

    /**
     * @brief Gets the number of programs.
     *
     * @return 1 as this plugin has no program support
     */
    int getNumPrograms() override;

    /**
     * @brief Gets the current program index.
     *
     * @return 0 as this plugin has no program support
     */
    int getCurrentProgram() override;

    /**
     * @brief Sets the current program.
     *
     * @param index The program index
     */
    void setCurrentProgram(int index) override;

    /**
     * @brief Gets the name of a program.
     *
     * @param index The program index
     * @return Empty string as this plugin has no program support
     */
    const juce::String getProgramName(int index) override;

    /**
     * @brief Changes the name of a program.
     *
     * @param index The program index
     * @param newName The new program name
     */
    void changeProgramName(int index, const juce::String &newName) override;

    /**
     * @brief Saves the plugin's state.
     *
     * @param destData The memory block to save the state to
     */
    void getStateInformation(juce::MemoryBlock &destData) override;

    /**
     * @brief Restores the plugin's state.
     *
     * @param data Pointer to the state data
     * @param sizeInBytes Size of the state data
     */
    void setStateInformation(const void *data, int sizeInBytes) override;

    /**
     * @brief Gets the active editor.
     *
     * In a real plugin host, this is managed by JUCE. For testing purposes,
     * we return the last created editor.
     *
     * @return Pointer to the active editor, or nullptr if none exists
     */
    juce::AudioProcessorEditor *getActiveEditor();

    /**
     * @brief Gets the processor's state tree.
     *
     * @return Reference to the AudioProcessorValueTreeState
     */
    juce::AudioProcessorValueTreeState &getState() { return state; }

    /**
     * @brief Gets the processor's network fetcher.
     *
     * @return Reference to the network fetcher
     */
    INetworkFetcher &getNetworkFetcher() { return networkFetcher; }

    /**
     * @brief Gets the processor's file system.
     *
     * @return Reference to the file system
     */
    IFileSystem &getFileSystem() { return *fileSystem; }

    /**
     * @brief Gets the processor's cache manager.
     *
     * @return Reference to the cache manager
     */
    CacheManager &getCacheManager() { return *cacheManager; }

    /**
     * @brief Gets the processor's preset manager.
     *
     * @return Reference to the preset manager
     */
    PresetManager &getPresetManager() { return *presetManager; }

    /**
     * @brief Gets the processor's gear library.
     *
     * @return Reference to the gear library
     */
    GearLibrary &getGearLibrary() { return *gearLibrary; }

    /**
     * @brief Saves the current state of all gear instances.
     */
    void saveInstanceState();

    /**
     * @brief Saves the state of all gear instances from a rack.
     *
     * @param rack The rack containing the gear instances
     * @param instanceTree The value tree to save the state to
     */
    void saveInstanceStateFromRack(Rack *rack, juce::ValueTree &instanceTree);

    /**
     * @brief Loads the state of all gear instances.
     */
    void loadInstanceState();

    /**
     * @brief Loads the state of all gear instances from a rack.
     *
     * @param rack The rack containing the gear instances
     */
    void loadInstanceState(Rack *rack);

    /**
     * @brief Resets all gear instances to their default state.
     */
    void resetAllInstances();

private:
    juce::AudioProcessorValueTreeState state;                ///< The processor's state tree
    juce::UndoManager undoManager;                           ///< Undo manager for state changes
    juce::AudioProcessorEditor *lastCreatedEditor = nullptr; ///< Pointer to the last created editor (for testing)
    Rack *rack = nullptr;                                    ///< Pointer to the rack (for testing)
    INetworkFetcher &networkFetcher;                         ///< Reference to the network fetcher for making HTTP requests
    std::unique_ptr<IFileSystem> fileSystem;
    std::unique_ptr<CacheManager> cacheManager;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<GearLibrary> gearLibrary;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQProcessor)
};