#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Shared/IFileSystem.h"
#include "../Shared/INetworkFetcher.h"
#include "../Shared/IRackStateListener.h"
#include "../Shared/ICacheManager.h"
#include "../Shared/IPresetManager.h"

// Forward declarations
class CacheManager;
class PresetManager;
class GearLibrary;

/**
 * Core audio processor implementation for the AnalogIQ plugin.
 * Handles audio processing, comprehensive state management, instance coordination, and plugin lifecycle.
 */
class AnalogIQProcessor : public juce::AudioProcessor
{
public:
    AnalogIQProcessor();
    explicit AnalogIQProcessor(INetworkFetcher &networkFetcher, IFileSystem &fileSystem);
    ~AnalogIQProcessor() override;

    // JUCE AudioProcessor methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    // Bus management
    bool isBusesLayoutSupported(const BusesLayout &busesLayout) const override;

    // Editor management
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    // Plugin information
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Program support
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    // State persistence
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    // State management interface
    juce::AudioProcessorValueTreeState &getState();
    void saveInstanceState();
    void loadInstanceState();
    void saveInstanceStateFromRack(Rack *rack);
    void loadInstanceState(Rack *rack);
    void resetAllInstances();
    void clearRackReference();

    // Component access methods
    INetworkFetcher *getNetworkFetcher() const { return networkFetcher; }
    IFileSystem *getFileSystem() const { return fileSystem; }
    CacheManager *getCacheManager() const { return cacheManager.get(); }
    PresetManager *getPresetManager() const { return presetManager.get(); }
    GearLibrary *getGearLibrary() const { return gearLibrary.get(); }

    // Testing support
    friend class AnalogIQProcessorTests;

private:
    // State management
    juce::AudioProcessorValueTreeState state;
    std::unique_ptr<juce::UndoManager> undoManager;

    // Editor tracking
    juce::AudioProcessorEditor *lastCreatedEditor;

    // Rack reference for fallback operations
    Rack *storedRackReference;

    // Dependencies
    INetworkFetcher *networkFetcher;
    IFileSystem *fileSystem;
    // These will be implemented as we add the other Model components
    std::unique_ptr<CacheManager> cacheManager;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<GearLibrary> gearLibrary;

    // Logging infrastructure
    void initializeLogging();
    void logToFile(const juce::String &message);
    juce::String getLogTimestamp();
    void logStateTreeStructure(const juce::ValueTree &tree, int depth = 0);
    void logXmlContent(const juce::XmlElement &xml);

private:
    // Private methods
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQProcessor)
};
