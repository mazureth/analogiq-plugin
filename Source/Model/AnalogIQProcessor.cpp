#include "AnalogIQProcessor.h"
#include "NetworkFetcher.h"
#include "FileSystem.h"
#include "CacheManager.h"
#include "PresetManager.h"
#include "GearLibrary.h"
#include "../View/AnalogIQEditor.h"
#include "../Shared/CrashLogger.h"
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
    juce::Logger::writeToLog("AnalogIQProcessor::createEditor - START - Creating new editor");
    CrashLogger::getInstance().log("CREATE_EDITOR", "AnalogIQProcessor::createEditor called");

    // Create the editor with all necessary dependencies
    juce::Logger::writeToLog("AnalogIQProcessor::createEditor - Creating AnalogIQEditor instance");
    CrashLogger::getInstance().log("CREATE_EDITOR", "Creating AnalogIQEditor instance");
    auto editor = new AnalogIQEditor(*this,
                                     fileSystem,
                                     cacheManager.get(),
                                     presetManager.get(),
                                     gearLibrary.get());
    juce::Logger::writeToLog("AnalogIQProcessor::createEditor - AnalogIQEditor created successfully");
    CrashLogger::getInstance().log("CREATE_EDITOR", "AnalogIQEditor created successfully");

    // Store reference to the editor for state management
    lastCreatedEditor = editor;

    // CRITICAL FIX: UI opening/closing is separate from state persistence
    // The RackModel data persists across UI open/close cycles
    // State loading only happens when DAW loads a project (setStateInformation)
    // State saving only happens when DAW saves a project (getStateInformation)
    CrashLogger::getInstance().log("CREATE_EDITOR", "UI created - RackModel data persists, no state loading needed");

    juce::Logger::writeToLog("AnalogIQProcessor::createEditor - END - Editor creation complete, returning editor");
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
    CrashLogger::getInstance().log("STATE_SAVE", "getStateInformation START - DestData initial size: " + juce::String(destData.getSize()));
    juce::Logger::writeToLog("DestData initial size: " + juce::String(destData.getSize()));

    try
    {
        // Save instance state before saving the main state
        juce::Logger::writeToLog("Calling saveInstanceState()...");
        CrashLogger::getInstance().log("STATE_SAVE", "Calling saveInstanceState()");
        saveInstanceState();
        juce::Logger::writeToLog("saveInstanceState() completed");
        CrashLogger::getInstance().log("STATE_SAVE", "saveInstanceState() completed");

        juce::Logger::writeToLog("Creating state snapshot...");
        CrashLogger::getInstance().log("STATE_SAVE", "Creating state snapshot");
        auto stateSnapshot = getState().copyState();
        juce::Logger::writeToLog("State snapshot created successfully");
        CrashLogger::getInstance().log("STATE_SAVE", "State snapshot created successfully - children: " + juce::String(stateSnapshot.getNumChildren()));
        juce::Logger::writeToLog("State snapshot children: " + juce::String(stateSnapshot.getNumChildren()));

        // Log the state tree structure
        logStateTreeStructure(stateSnapshot, 0);

        juce::Logger::writeToLog("Creating XML from state...");
        CrashLogger::getInstance().log("STATE_SAVE", "Creating XML from state");
        std::unique_ptr<juce::XmlElement> xml(stateSnapshot.createXml());

        if (xml != nullptr)
        {
            juce::Logger::writeToLog("XML created successfully");
            CrashLogger::getInstance().log("STATE_SAVE", "XML created successfully - children: " + juce::String(xml->getNumChildElements()) + ", tag: " + xml->getTagName());
            juce::Logger::writeToLog("XML children: " + juce::String(xml->getNumChildElements()));
            juce::Logger::writeToLog("XML tag name: " + xml->getTagName());

            // Log XML content for debugging
            logXmlContent(*xml);

            juce::Logger::writeToLog("Converting XML to binary...");
            CrashLogger::getInstance().log("STATE_SAVE", "Converting XML to binary");
            copyXmlToBinary(*xml, destData);
            juce::Logger::writeToLog("Binary conversion completed");
            CrashLogger::getInstance().log("STATE_SAVE", "Binary conversion completed - Final destData size: " + juce::String(destData.getSize()));
            juce::Logger::writeToLog("Final destData size: " + juce::String(destData.getSize()));
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Failed to create XML from state");
            CrashLogger::getInstance().log("STATE_SAVE_ERROR", "Failed to create XML from state");
        }
    }
    catch (const std::exception &e)
    {
        juce::Logger::writeToLog("ERROR: Exception during serialization: " + juce::String(e.what()));
        CrashLogger::getInstance().log("STATE_SAVE_ERROR", "Exception during serialization: " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("ERROR: Unknown exception during serialization");
        CrashLogger::getInstance().log("STATE_SAVE_ERROR", "Unknown exception during serialization");
    }

    juce::Logger::writeToLog("=== getStateInformation END ===");
    CrashLogger::getInstance().log("STATE_SAVE", "getStateInformation END");
}

void AnalogIQProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::Logger::writeToLog("setStateInformation called - Size: " + juce::String(sizeInBytes) + " bytes");
    CrashLogger::getInstance().log("STATE_LOAD", "setStateInformation called - Size: " + juce::String(sizeInBytes) + " bytes");

    try
    {
        CrashLogger::getInstance().log("STATE_LOAD", "Converting binary to XML");
        auto xml = getXmlFromBinary(data, sizeInBytes);
        if (xml != nullptr)
        {
            CrashLogger::getInstance().log("STATE_LOAD", "XML created successfully - children: " + juce::String(xml->getNumChildElements()) + ", tag: " + xml->getTagName());
            state.replaceState(juce::ValueTree::fromXml(*xml));
            CrashLogger::getInstance().log("STATE_LOAD", "ValueTree state replaced successfully");
            logXmlContent(*xml);
        }
        else
        {
            CrashLogger::getInstance().log("STATE_LOAD_ERROR", "Failed to create XML from binary data");
        }
    }
    catch (const std::exception &e)
    {
        juce::Logger::writeToLog("Error in setStateInformation: " + juce::String(e.what()));
        CrashLogger::getInstance().log("STATE_LOAD_ERROR", "Exception in setStateInformation: " + juce::String(e.what()));
    }

    // Load instance state after restoring the main state
    // We need to wait for the editor to be created, so we'll do this in createEditor
    CrashLogger::getInstance().log("STATE_LOAD", "setStateInformation completed - instance loading deferred to createEditor");
}

// State management interface
juce::AudioProcessorValueTreeState &AnalogIQProcessor::getState()
{
    return state;
}

void AnalogIQProcessor::saveInstanceState()
{
    juce::Logger::writeToLog("=== saveInstanceState START ===");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "saveInstanceState START");

    // Log RackModel instance ID
    if (rackModel)
    {
        juce::Logger::writeToLog("saveInstanceState - RackModel instance ID: " + juce::String(rackModel->getInstanceId()));
        CrashLogger::getInstance().log("INSTANCE_SAVE", "RackModel instance ID: " + juce::String(rackModel->getInstanceId()));
    }
    else
    {
        juce::Logger::writeToLog("saveInstanceState - ERROR: rackModel is null!");
        CrashLogger::getInstance().log("INSTANCE_SAVE_ERROR", "rackModel is null!");
        return;
    }

    // Create a child tree for instance state
    juce::Logger::writeToLog("Creating/accessing instances tree...");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Creating/accessing instances tree");
    auto instanceTree = state.state.getOrCreateChildWithName("instances", undoManager.get());
    juce::Logger::writeToLog("Instances tree accessed successfully");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Instances tree accessed successfully - children before clear: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("Instances tree children before clear: " + juce::String(instanceTree.getNumChildren()));

    // Clear existing instance data
    juce::Logger::writeToLog("Clearing existing instance data...");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Clearing existing instance data");
    instanceTree.removeAllChildren(undoManager.get());
    juce::Logger::writeToLog("Instance data cleared");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Instance data cleared - children after clear: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("Instances tree children after clear: " + juce::String(instanceTree.getNumChildren()));

    // Save instance data directly from RackModel - no Editor dependency!
    juce::Logger::writeToLog("Saving instance data directly from RackModel...");
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Saving instance data directly from RackModel - slots: " + juce::String(rackModel->getSlotCount()));
    juce::Logger::writeToLog("RackModel slots: " + juce::String(rackModel->getSlotCount()));

    // Count occupied slots first to avoid saving empty state
    int occupiedSlots = 0;
    for (int i = 0; i < rackModel->getSlotCount(); ++i)
    {
        if (auto *slotData = rackModel->getSlotData(i))
        {
            juce::Logger::writeToLog("Slot " + juce::String(i) + " - isOccupied: " + juce::String(slotData->isOccupied ? "YES" : "NO") + " - gearId: '" + slotData->gearId + "' - instanceId: '" + slotData->instanceId + "' - gearName: '" + slotData->gearName + "'");
            CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " - isOccupied: " + juce::String(slotData->isOccupied ? "YES" : "NO") + " - gearId: '" + slotData->gearId + "' - instanceId: '" + slotData->instanceId + "' - gearName: '" + slotData->gearName + "'");

            if (slotData->isOccupied && !slotData->gearId.isEmpty())
            {
                occupiedSlots++;
                juce::Logger::writeToLog("Slot " + juce::String(i) + " is occupied and will be saved");
                CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " is occupied and will be saved");
            }
            else
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " is empty or has no gearId - will not be saved");
                CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " is empty or has no gearId - will not be saved");
            }
        }
        else
        {
            juce::Logger::writeToLog("Slot " + juce::String(i) + " data is null");
            CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " data is null");
        }
    }

    juce::Logger::writeToLog("Found " + juce::String(occupiedSlots) + " occupied slots out of " + juce::String(rackModel->getSlotCount()));
    CrashLogger::getInstance().log("INSTANCE_SAVE", "Found " + juce::String(occupiedSlots) + " occupied slots out of " + juce::String(rackModel->getSlotCount()));

    // Only save state if there are occupied slots
    if (occupiedSlots == 0)
    {
        juce::Logger::writeToLog("No occupied slots found - skipping gear item saving");
        CrashLogger::getInstance().log("INSTANCE_SAVE", "No occupied slots found - skipping gear item saving");
    }
    else
    {
        juce::Logger::writeToLog("Saving " + juce::String(occupiedSlots) + " occupied slots");
        CrashLogger::getInstance().log("INSTANCE_SAVE", "Saving " + juce::String(occupiedSlots) + " occupied slots");
    }

    // Save instance data for each slot directly from RackModel (only if there are occupied slots)
    if (occupiedSlots > 0)
    {
        for (int i = 0; i < rackModel->getSlotCount(); ++i)
        {
            juce::Logger::writeToLog("Processing slot " + juce::String(i));
            CrashLogger::getInstance().log("INSTANCE_SAVE", "Processing slot " + juce::String(i));

            if (auto *slotData = rackModel->getSlotData(i))
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " obtained successfully");
                CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " obtained successfully");

                if (slotData->isOccupied && !slotData->gearId.isEmpty())
                {
                    juce::Logger::writeToLog("Slot " + juce::String(i) + " has gear item: " + slotData->gearName);
                    CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " has gear item: " + slotData->gearName + " - Instance ID: " + slotData->instanceId + " - Unit ID: " + slotData->gearId + " - Controls: " + juce::String(slotData->controls.size()));
                    juce::Logger::writeToLog("  Instance ID: " + slotData->instanceId);
                    juce::Logger::writeToLog("  Unit ID: " + slotData->gearId);
                    juce::Logger::writeToLog("  Controls count: " + juce::String(slotData->controls.size()));

                    // Save state for instances only (all items in rack are now instances)
                    if (!slotData->instanceId.isEmpty() && !slotData->gearId.isEmpty())
                    {
                        juce::Logger::writeToLog("Saving instance data for slot " + juce::String(i));
                        CrashLogger::getInstance().log("INSTANCE_SAVE", "Saving instance data for slot " + juce::String(i));

                        auto slotTree = instanceTree.getOrCreateChildWithName("slot_" + juce::String(i), undoManager.get());
                        juce::Logger::writeToLog("Slot tree created for slot " + juce::String(i));
                        CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot tree created for slot " + juce::String(i));

                        slotTree.setProperty("instanceId", slotData->instanceId, undoManager.get());
                        slotTree.setProperty("sourceUnitId", slotData->gearId, undoManager.get());
                        juce::Logger::writeToLog("Slot properties set for slot " + juce::String(i));
                        CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot properties set for slot " + juce::String(i) + " - instanceId: " + slotData->instanceId + " - sourceUnitId: " + slotData->gearId);

                        // Save control values
                        auto controlsTree = slotTree.getOrCreateChildWithName("controls", undoManager.get());
                        juce::Logger::writeToLog("Controls tree created for slot " + juce::String(i));
                        CrashLogger::getInstance().log("INSTANCE_SAVE", "Controls tree created for slot " + juce::String(i) + " - controls count: " + juce::String(slotData->controls.size()));

                        for (int j = 0; j < slotData->controls.size(); ++j)
                        {
                            const auto &control = slotData->controls[j];
                            juce::Logger::writeToLog("Processing control " + juce::String(j) + " in slot " + juce::String(i));
                            CrashLogger::getInstance().log("INSTANCE_SAVE", "Processing control " + juce::String(j) + " in slot " + juce::String(i) + " - name: " + control.name + " - type: " + juce::String(static_cast<int>(control.type)) + " - value: " + juce::String(control.currentValue));
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
                        juce::Logger::writeToLog("Slot " + juce::String(i) + " has gear item but missing instanceId or gearId - skipping");
                        CrashLogger::getInstance().log("INSTANCE_SAVE", "Slot " + juce::String(i) + " has gear item but missing instanceId or gearId - skipping");
                    }
                }
                else
                {
                    juce::Logger::writeToLog("Slot " + juce::String(i) + " is empty or has no gear ID - skipping");
                }
            }
            else
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " data is null - skipping");
                CrashLogger::getInstance().log("INSTANCE_SAVE_ERROR", "Slot " + juce::String(i) + " data is null - skipping");
            }
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

    juce::Logger::writeToLog("Final instances tree children: " + juce::String(instanceTree.getNumChildren()));
    CrashLogger::getInstance().log("INSTANCE_SAVE", "saveInstanceState END - Final instances tree children: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("=== saveInstanceState END ===");
}

void AnalogIQProcessor::loadInstanceState()
{
    juce::Logger::writeToLog("=== loadInstanceState START ===");
    CrashLogger::getInstance().log("INSTANCE_LOAD", "loadInstanceState START");

    // Get the instance state tree
    auto instanceTree = state.state.getChildWithName("instances");
    if (!instanceTree.isValid())
    {
        juce::Logger::writeToLog("No instance tree found in state");
        CrashLogger::getInstance().log("INSTANCE_LOAD_ERROR", "No instance tree found in state");
        return;
    }

    juce::Logger::writeToLog("Instance tree found, loading gear items...");
    CrashLogger::getInstance().log("INSTANCE_LOAD", "Instance tree found, loading gear items - children: " + juce::String(instanceTree.getNumChildren()));

    // Check if there are any gear items to load (children > 1 means gear items exist, 1 means only notes)
    int gearItemCount = 0;
    for (int i = 0; i < instanceTree.getNumChildren(); ++i)
    {
        auto child = instanceTree.getChild(i);
        if (child.getType().toString().startsWith("slot_"))
        {
            gearItemCount++;
        }
    }

    juce::Logger::writeToLog("Found " + juce::String(gearItemCount) + " gear items to load");
    CrashLogger::getInstance().log("INSTANCE_LOAD", "Found " + juce::String(gearItemCount) + " gear items to load");

    // Clear existing rack state first
    rackModel->clearAllSlots();
    juce::Logger::writeToLog("Cleared existing rack state");
    CrashLogger::getInstance().log("INSTANCE_LOAD", "Cleared existing rack state");

    // Only proceed with loading if there are gear items
    if (gearItemCount == 0)
    {
        juce::Logger::writeToLog("No gear items found in state - skipping gear loading");
        CrashLogger::getInstance().log("INSTANCE_LOAD", "No gear items found in state - skipping gear loading");
    }
    else
    {
        juce::Logger::writeToLog("Loading " + juce::String(gearItemCount) + " gear items");
        CrashLogger::getInstance().log("INSTANCE_LOAD", "Loading " + juce::String(gearItemCount) + " gear items");
    }

    // Load instance data for each slot (only if there are gear items)
    if (gearItemCount > 0)
    {
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
                    CrashLogger::getInstance().log("INSTANCE_LOAD", "Loading gear item for slot " + juce::String(i) + " with unit ID: " + sourceUnitId + ", instance ID: " + instanceId);

                    // Get the template gear item from the library
                    auto *gearItemTemplate = gearLibrary->getGearItem(sourceUnitId);

                    if (gearItemTemplate)
                    {
                        juce::Logger::writeToLog("Found gear template: " + gearItemTemplate->name);
                        CrashLogger::getInstance().log("INSTANCE_LOAD", "Found gear template: " + gearItemTemplate->name);

                        // Load the schema for the template first (like in normal drag-and-drop flow)
                        bool schemaLoaded = gearLibrary->loadGearSchema(gearItemTemplate);
                        juce::Logger::writeToLog("Schema loaded: " + juce::String(schemaLoaded ? "YES" : "NO"));
                        CrashLogger::getInstance().log("INSTANCE_LOAD", "Schema loaded: " + juce::String(schemaLoaded ? "YES" : "NO"));

                        if (schemaLoaded)
                        {
                            // Use the public addGearToSlot method to add the gear item
                            // This will handle all the SlotData creation and async loading
                            bool gearAdded = rackModel->addGearToSlot(i, sourceUnitId);

                            if (gearAdded)
                            {
                                juce::Logger::writeToLog("Successfully added gear to slot " + juce::String(i));
                                CrashLogger::getInstance().log("INSTANCE_LOAD", "Successfully added gear to slot " + juce::String(i));

                                // Now we need to restore the control values from the saved state
                                auto controlsTree = slotTree.getChildWithName("controls");
                                if (controlsTree.isValid())
                                {
                                    juce::Logger::writeToLog("Loading control values from ValueTree...");
                                    CrashLogger::getInstance().log("INSTANCE_LOAD", "Loading control values from ValueTree for slot " + juce::String(i) + " - controls count: " + juce::String(controlsTree.getNumChildren()));

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
                                                CrashLogger::getInstance().log("INSTANCE_LOAD", "Control " + juce::String(j) + " (" + slotData->controls[j].name + ") restored - value: " + juce::String(slotData->controls[j].currentValue));
                                            }
                                        }

                                        // Update the instance ID to match the saved state
                                        slotData->instanceId = instanceId;

                                        juce::Logger::writeToLog("Successfully loaded gear item for slot " + juce::String(i));
                                        CrashLogger::getInstance().log("INSTANCE_LOAD", "Successfully loaded gear item for slot " + juce::String(i) + " - instanceId: " + instanceId);
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
