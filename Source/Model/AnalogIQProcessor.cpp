#include "AnalogIQProcessor.h"
#include "NetworkFetcher.h"
#include "FileSystem.h"
#include "CacheManager.h"
#include "PresetManager.h"
#include "GearLibrary.h"
#include "../View/AnalogIQEditor.h"
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
      state(*this, nullptr, "Parameters", createParameterLayout()), undoManager(std::make_unique<juce::UndoManager>()), lastCreatedEditor(nullptr), storedRackReference(nullptr), networkFetcher(nullptr), fileSystem(nullptr), cacheManager(nullptr), presetManager(nullptr), gearLibrary(nullptr), rackModel(nullptr)
{
    // Initialize logging
    initializeLogging();

    // Create default implementations for dependencies
    fileSystem = new FileSystem();
    networkFetcher = new NetworkFetcher();
    cacheManager = std::make_unique<CacheManager>(*fileSystem);
    presetManager = std::make_unique<PresetManager>(*fileSystem);
    gearLibrary = std::make_unique<GearLibrary>(*fileSystem, *cacheManager, *networkFetcher);
    rackModel = std::make_unique<RackModel>(*gearLibrary, *presetManager, *cacheManager);
}

AnalogIQProcessor::AnalogIQProcessor(INetworkFetcher &nf, IFileSystem &fs)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters", createParameterLayout()), undoManager(std::make_unique<juce::UndoManager>()), lastCreatedEditor(nullptr), storedRackReference(nullptr), networkFetcher(&nf), fileSystem(&fs), cacheManager(std::make_unique<CacheManager>(fs)), presetManager(std::make_unique<PresetManager>(fs)), gearLibrary(std::make_unique<GearLibrary>(fs, *cacheManager, nf)), rackModel(std::make_unique<RackModel>(*gearLibrary, *presetManager, *cacheManager))
{
    // Initialize logging
    initializeLogging();
}

AnalogIQProcessor::~AnalogIQProcessor()
{
    // Clear stored references
    storedRackReference = nullptr;
    lastCreatedEditor = nullptr;
}

// JUCE AudioProcessor methods
void AnalogIQProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::Logger::writeToLog("prepareToPlay called - Sample Rate: " + juce::String(sampleRate) +
                             ", Block Size: " + juce::String(samplesPerBlock));
}

void AnalogIQProcessor::releaseResources()
{
    juce::Logger::writeToLog("releaseResources called");
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
    // Create the editor with all necessary dependencies
    auto editor = new AnalogIQEditor(*this,
                                     fileSystem,
                                     cacheManager.get(),
                                     presetManager.get(),
                                     gearLibrary.get());

    // Store reference to the editor for state management
    lastCreatedEditor = editor;

    // Load instance state after the editor is created and gear library is loaded
    // We'll defer this to after the gear library is ready
    juce::MessageManager::callAsync([this, editor]()
                                    {
        // Wait a bit more to ensure gear library is fully loaded
        juce::Timer::callAfterDelay(100, [this, editor]()
        {
            if (auto *rack = editor->getRack())
            {
                loadInstanceState(rack);
            }
        }); });

    return editor;
}

bool AnalogIQProcessor::hasEditor() const
{
    return true; // Editor is now implemented
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
    juce::Logger::writeToLog("=== getStateInformation START ===");
    juce::Logger::writeToLog("DestData initial size: " + juce::String(destData.getSize()));

    try
    {
        // Save instance state before saving the main state
        juce::Logger::writeToLog("Calling saveInstanceState()...");
        saveInstanceState();
        juce::Logger::writeToLog("saveInstanceState() completed");

        juce::Logger::writeToLog("Creating state snapshot...");
        auto stateSnapshot = getState().copyState();
        juce::Logger::writeToLog("State snapshot created successfully");
        juce::Logger::writeToLog("State snapshot children: " + juce::String(stateSnapshot.getNumChildren()));

        // Log the state tree structure
        logStateTreeStructure(stateSnapshot, 0);

        juce::Logger::writeToLog("Creating XML from state...");
        std::unique_ptr<juce::XmlElement> xml(stateSnapshot.createXml());

        if (xml != nullptr)
        {
            juce::Logger::writeToLog("XML created successfully");
            juce::Logger::writeToLog("XML children: " + juce::String(xml->getNumChildElements()));
            juce::Logger::writeToLog("XML tag name: " + xml->getTagName());

            // Log XML content for debugging
            logXmlContent(*xml);

            juce::Logger::writeToLog("Converting XML to binary...");
            copyXmlToBinary(*xml, destData);
            juce::Logger::writeToLog("Binary conversion completed");
            juce::Logger::writeToLog("Final destData size: " + juce::String(destData.getSize()));
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Failed to create XML from state");
        }
    }
    catch (const std::exception &e)
    {
        juce::Logger::writeToLog("ERROR: Exception during serialization: " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("ERROR: Unknown exception during serialization");
    }

    juce::Logger::writeToLog("=== getStateInformation END ===");
}

void AnalogIQProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::Logger::writeToLog("setStateInformation called - Size: " + juce::String(sizeInBytes) + " bytes");

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
        juce::Logger::writeToLog("Error in setStateInformation: " + juce::String(e.what()));
    }

    // Load instance state after restoring the main state
    // We need to wait for the editor to be created, so we'll do this in createEditor
}

// State management interface
juce::AudioProcessorValueTreeState &AnalogIQProcessor::getState()
{
    return state;
}

void AnalogIQProcessor::saveInstanceState()
{
    juce::Logger::writeToLog("=== saveInstanceState START ===");

    // Create a child tree for instance state
    juce::Logger::writeToLog("Creating/accessing instances tree...");
    auto instanceTree = state.state.getOrCreateChildWithName("instances", undoManager.get());
    juce::Logger::writeToLog("Instances tree accessed successfully");
    juce::Logger::writeToLog("Instances tree children before clear: " + juce::String(instanceTree.getNumChildren()));

    // Clear existing instance data
    juce::Logger::writeToLog("Clearing existing instance data...");
    instanceTree.removeAllChildren(undoManager.get());
    juce::Logger::writeToLog("Instance data cleared");
    juce::Logger::writeToLog("Instances tree children after clear: " + juce::String(instanceTree.getNumChildren()));

    // Get the rack from the editor
    juce::Logger::writeToLog("Checking editor availability...");
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        juce::Logger::writeToLog("Editor is available");
        juce::Logger::writeToLog("Editor type: " + juce::String(typeid(*editor).name()));

        if (auto *rack = editor->getRack())
        {
            juce::Logger::writeToLog("Rack obtained from editor successfully");
            juce::Logger::writeToLog("Rack slots: " + juce::String(rackModel->getSlotCount()));
            saveInstanceStateFromRack(rack, instanceTree);
        }
        else
        {
            juce::Logger::writeToLog("WARNING: Editor available but getRack() returned null");
        }
    }
    else
    {
        juce::Logger::writeToLog("Editor is NOT available (getActiveEditor() returned null)");
        juce::Logger::writeToLog("ERROR: No rack available - cannot save instance state");
        juce::Logger::writeToLog("This will result in an empty state being saved");
    }

    juce::Logger::writeToLog("Final instances tree children: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("=== saveInstanceState END ===");
}

void AnalogIQProcessor::loadInstanceState()
{
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            loadInstanceState(rack);
        }
    }
}

void AnalogIQProcessor::saveInstanceStateFromRack(Rack *rack, juce::ValueTree &instanceTree)
{
    juce::Logger::writeToLog("=== saveInstanceStateFromRack START ===");

    if (rack == nullptr)
    {
        juce::Logger::writeToLog("ERROR: Rack pointer is null");
        return;
    }

    juce::Logger::writeToLog("Rack validation: rack pointer valid");
    juce::Logger::writeToLog("Rack slots: " + juce::String(rackModel->getSlotCount()));

    // Save instance data for each slot
    for (int i = 0; i < rackModel->getSlotCount(); ++i)
    {
        juce::Logger::writeToLog("Processing slot " + juce::String(i));

        if (auto *slotData = rackModel->getSlotData(i))
        {
            juce::Logger::writeToLog("Slot " + juce::String(i) + " obtained successfully");

            if (slotData->isOccupied && !slotData->gearId.isEmpty())
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " has gear item: " + slotData->gearName);
                juce::Logger::writeToLog("  Instance ID: " + slotData->instanceId);
                juce::Logger::writeToLog("  Unit ID: " + slotData->gearId);
                juce::Logger::writeToLog("  Controls count: " + juce::String(slotData->controls.size()));

                // Save state for instances only (all items in rack are now instances)
                if (!slotData->instanceId.isEmpty() && !slotData->gearId.isEmpty())
                {
                    juce::Logger::writeToLog("Saving instance data for slot " + juce::String(i));

                    auto slotTree = instanceTree.getOrCreateChildWithName("slot_" + juce::String(i), undoManager.get());
                    juce::Logger::writeToLog("Slot tree created for slot " + juce::String(i));

                    slotTree.setProperty("instanceId", slotData->instanceId, undoManager.get());
                    slotTree.setProperty("sourceUnitId", slotData->gearId, undoManager.get());
                    juce::Logger::writeToLog("Slot properties set for slot " + juce::String(i));

                    // Save control values
                    auto controlsTree = slotTree.getOrCreateChildWithName("controls", undoManager.get());
                    juce::Logger::writeToLog("Controls tree created for slot " + juce::String(i));

                    for (int j = 0; j < slotData->controls.size(); ++j)
                    {
                        const auto &control = slotData->controls[j];
                        juce::Logger::writeToLog("Processing control " + juce::String(j) + " in slot " + juce::String(i));
                        juce::Logger::writeToLog("  Control name: " + control.name);
                        juce::Logger::writeToLog("  Control type: " + juce::String(static_cast<int>(control.type)));
                        juce::Logger::writeToLog("  Control value: " + juce::String(control.currentValue));
                        juce::Logger::writeToLog("  Control initial value: " + juce::String(control.initialValue));

                        auto controlTree = controlsTree.getOrCreateChildWithName("control_" + juce::String(j), undoManager.get());
                        controlTree.setProperty("value", control.currentValue, undoManager.get());
                        controlTree.setProperty("initialValue", control.initialValue, undoManager.get());

                        if (control.type == GearControl::ControlType::Switch || control.type == GearControl::ControlType::Button)
                        {
                            controlTree.setProperty("currentIndex", control.currentIndex, undoManager.get());
                            juce::Logger::writeToLog("  Control current index: " + juce::String(control.currentIndex));
                        }

                        juce::Logger::writeToLog("Control " + juce::String(j) + " saved successfully");
                    }

                    juce::Logger::writeToLog("Slot " + juce::String(i) + " completed successfully");
                }
                else
                {
                    juce::Logger::writeToLog("Slot " + juce::String(i) + " skipped - not a valid instance");
                    juce::Logger::writeToLog("  Instance ID empty: " + juce::String(slotData->instanceId.isEmpty() ? "true" : "false"));
                    juce::Logger::writeToLog("  Unit ID empty: " + juce::String(slotData->gearId.isEmpty() ? "true" : "false"));
                }
            }
            else
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " has no gear item");
            }
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Failed to get slot " + juce::String(i));
        }
    }

    // Save notes panel content
    juce::Logger::writeToLog("Attempting to save notes panel content...");
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        juce::Logger::writeToLog("Editor available for notes panel");
        if (auto *notesPanel = editor->getNotesPanel())
        {
            juce::Logger::writeToLog("Notes panel obtained successfully");
            auto notesTree = instanceTree.getOrCreateChildWithName("notes", undoManager.get());
            auto notesContent = notesPanel->getNotes();
            notesTree.setProperty("content", notesContent, undoManager.get());
            juce::Logger::writeToLog("Notes content saved: " + juce::String(notesContent.length()) + " characters");
        }
        else
        {
            juce::Logger::writeToLog("WARNING: Notes panel is null");
        }
    }
    else
    {
        juce::Logger::writeToLog("Editor not available for notes panel");
    }

    juce::Logger::writeToLog("Final instance tree children: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("=== saveInstanceStateFromRack END ===");
}

void AnalogIQProcessor::loadInstanceState(Rack *rack)
{
    juce::Logger::writeToLog("=== loadInstanceState START ===");

    // Get the instance state tree
    auto instanceTree = state.state.getChildWithName("instances");
    if (!instanceTree.isValid())
    {
        juce::Logger::writeToLog("No instance tree found in state");
        return;
    }

    juce::Logger::writeToLog("Instance tree found, loading gear items...");

    if (rack != nullptr)
    {
        // Clear existing rack state first
        rackModel->clearAllSlots();
        juce::Logger::writeToLog("Cleared existing rack state");

        // Load instance data for each slot
        for (int i = 0; i < rackModel->getSlotCount(); ++i)
        {
            auto slotTree = instanceTree.getChildWithName("slot_" + juce::String(i));
            if (slotTree.isValid())
            {
                // Get the source unit ID and instance ID from the saved state
                auto sourceUnitId = slotTree.getProperty("sourceUnitId").toString();
                auto instanceId = slotTree.getProperty("instanceId").toString();

                if (!sourceUnitId.isEmpty() && !instanceId.isEmpty())
                {
                    juce::Logger::writeToLog("Loading gear item for slot " + juce::String(i) + " with unit ID: " + sourceUnitId + ", instance ID: " + instanceId);

                    // Get the template gear item from the library
                    auto *gearItemTemplate = gearLibrary->getGearItem(sourceUnitId);

                    if (gearItemTemplate)
                    {
                        juce::Logger::writeToLog("Found gear template: " + gearItemTemplate->name);

                        // Load the schema for the template first (like in normal drag-and-drop flow)
                        bool schemaLoaded = gearLibrary->loadGearSchema(gearItemTemplate);
                        juce::Logger::writeToLog("Schema loaded: " + juce::String(schemaLoaded ? "YES" : "NO"));

                        if (schemaLoaded)
                        {
                            // Use the public addGearToSlot method to add the gear item
                            // This will handle all the SlotData creation and async loading
                            bool gearAdded = rackModel->addGearToSlot(i, sourceUnitId);

                            if (gearAdded)
                            {
                                juce::Logger::writeToLog("Successfully added gear to slot " + juce::String(i));

                                // Now we need to restore the control values from the saved state
                                auto controlsTree = slotTree.getChildWithName("controls");
                                if (controlsTree.isValid())
                                {
                                    juce::Logger::writeToLog("Loading control values from ValueTree...");

                                    // Get the slot data to update control values
                                    if (auto *slotData = rackModel->getSlotData(i))
                                    {
                                        for (int j = 0; j < controlsTree.getNumChildren() && j < slotData->controls.size(); ++j)
                                        {
                                            auto controlTree = controlsTree.getChild(j);
                                            if (controlTree.isValid())
                                            {
                                                // Update the slot's control with saved values
                                                slotData->controls.getReference(j).currentValue = (float)controlTree.getProperty("value");
                                                slotData->controls.getReference(j).initialValue = (float)controlTree.getProperty("initialValue");

                                                // Handle switch/button specific properties
                                                if (controlTree.hasProperty("currentIndex"))
                                                {
                                                    slotData->controls.getReference(j).currentIndex = (int)controlTree.getProperty("currentIndex");
                                                }

                                                juce::Logger::writeToLog("  Control " + juce::String(j) + " (" + slotData->controls[j].name + "): value=" + juce::String(slotData->controls[j].currentValue));
                                            }
                                        }

                                        // Update the instance ID to match the saved state
                                        slotData->instanceId = instanceId;

                                        juce::Logger::writeToLog("Successfully loaded gear item for slot " + juce::String(i));
                                    }
                                }
                            }
                            else
                            {
                                juce::Logger::writeToLog("Failed to add gear to slot " + juce::String(i));
                            }
                        }
                        else
                        {
                            juce::Logger::writeToLog("Failed to load schema for gear template: " + sourceUnitId);
                        }
                    }
                    else
                    {
                        juce::Logger::writeToLog("Gear template not found: " + sourceUnitId);
                    }
                }
                else
                {
                    juce::Logger::writeToLog("Slot " + juce::String(i) + " has empty sourceUnitId or instanceId, skipping");
                }
            }
        }

        // Load notes panel content after all gear items are processed
        if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
        {
            if (auto *notesPanel = editor->getNotesPanel())
            {
                auto notesTree = instanceTree.getChildWithName("notes");
                if (notesTree.isValid())
                {
                    auto notesContent = notesTree.getProperty("content").toString();
                    if (notesContent.isNotEmpty())
                    {
                        notesPanel->setNotes(notesContent);
                        juce::Logger::writeToLog("Loaded notes panel content");
                    }
                }
            }
        }

        juce::Logger::writeToLog("=== loadInstanceState COMPLETE ===");
    }
    else
    {
        juce::Logger::writeToLog("Rack pointer is null, cannot load state");
    }
}

void AnalogIQProcessor::resetAllInstances()
{
    // First try to get the rack from the editor
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            // TODO: Implement resetAllInstances method in Rack
            juce::Logger::writeToLog("Reset all instances requested");
            return;
        }
    }

    juce::Logger::writeToLog("ERROR: No rack available for resetAllInstances - operation failed");
}

void AnalogIQProcessor::clearRackReference()
{
    juce::Logger::writeToLog("clearRackReference called");
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
    // Temporarily disable file logging to avoid permission issues
    // TODO: Re-enable with proper error handling when file system is stable
}

void AnalogIQProcessor::writeToLog(const juce::String &message)
{
    // Temporarily disable file logging to avoid permission issues
    // TODO: Re-enable with proper error handling when file system is stable
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

    juce::Logger::writeToLog(nodeInfo);

    for (auto i = 0; i < tree.getNumChildren(); ++i)
    {
        logStateTreeStructure(tree.getChild(i), depth + 1);
    }
}

void AnalogIQProcessor::logXmlContent(const juce::XmlElement &xml)
{
    juce::Logger::writeToLog("XML Content: " + xml.toString());
}

// Plugin factory function
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    // Create static instances for dependency injection
    static auto networkFetcher = std::make_unique<NetworkFetcher>();
    static auto fileSystem = std::make_unique<FileSystem>();

    return new AnalogIQProcessor(*networkFetcher, *fileSystem);
}
