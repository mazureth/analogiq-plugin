/**
 * @file AnalogIQProcessor.cpp
 * @brief Implementation of the AnalogIQProcessor class.
 *
 * This file implements the audio processor for the AnalogIQ plugin,
 * handling audio processing, state management, and instance control.
 * The processor manages the plugin's state and coordinates between
 * the editor interface and the audio processing system.
 */

#include "AnalogIQProcessor.h"
#include "AnalogIQEditor.h"
#include "CacheManager.h"

/**
 * @brief Constructs a new AnalogIQProcessor.
 *
 * Initializes the processor with stereo input and output buses,
 * and sets up the state management system.
 *
 * @param networkFetcher Reference to the network fetcher to use
 * @param fileSystem Reference to the file system to use
 */
AnalogIQProcessor::AnalogIQProcessor(INetworkFetcher &networkFetcher, IFileSystem &fileSystem)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, &undoManager, "Parameters", {}),
      networkFetcher(networkFetcher),
      fileSystem(std::make_unique<FileSystem>()),
      cacheManager(std::make_unique<CacheManager>(*this->fileSystem)),
      presetManager(std::make_unique<PresetManager>(*this->fileSystem, *cacheManager)),
      gearLibrary(std::make_unique<GearLibrary>(networkFetcher, *this->fileSystem, *cacheManager, *presetManager))
{
    initializeLogging();
    juce::Logger::writeToLog("=== AnalogIQProcessor Constructor ===");
}

/**
 * @brief Destructor for AnalogIQProcessor.
 */
AnalogIQProcessor::~AnalogIQProcessor()
{
}

/**
 * @brief Initializes the logging system.
 */
void AnalogIQProcessor::initializeLogging()
{
    // Create log file in user's documents directory
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto analogiqDir = documentsDir.getChildFile("AnalogIQ");

#ifdef JUCE_DEBUG
    std::cout << "[AnalogIQ] Documents dir: " << documentsDir.getFullPathName().toStdString() << std::endl;
    std::cout << "[AnalogIQ] AnalogIQ dir: " << analogiqDir.getFullPathName().toStdString() << std::endl;
    std::cout << "[AnalogIQ] AnalogIQ dir exists: " << (analogiqDir.exists() ? "true" : "false") << std::endl;
#endif

    if (!analogiqDir.exists())
    {
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] Creating AnalogIQ directory..." << std::endl;
#endif
        auto result = analogiqDir.createDirectory();
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] Directory creation result: " << (result ? "success" : "failed") << std::endl;
#endif
    }

    logFile = analogiqDir.getChildFile("serialization.log");

#ifdef JUCE_DEBUG
    std::cout << "[AnalogIQ] Log file path: " << logFile.getFullPathName().toStdString() << std::endl;
    std::cout << "[AnalogIQ] Log file exists: " << (logFile.exists() ? "true" : "false") << std::endl;
#endif

    // Test logging immediately
    juce::Logger::writeToLog("=== Logging Initialized ===");
    juce::Logger::writeToLog("Log file: " + logFile.getFullPathName());

// Also log to console in debug mode
#ifdef JUCE_DEBUG
    std::cout << "[AnalogIQ] Logging initialized. Log file: " << logFile.getFullPathName().toStdString() << std::endl;
#endif
}

/**
 * @brief Writes a message to the log file.
 */
void AnalogIQProcessor::juce::Logger::writeToLog(const juce::String &message)
{
// Always log to console in debug mode
#ifdef JUCE_DEBUG
    std::cout << "[AnalogIQ] " << message.toStdString() << std::endl;
#endif

#ifdef JUCE_DEBUG
    std::cout << "[AnalogIQ] Attempting to write to log file..." << std::endl;
    std::cout << "[AnalogIQ] Log file path: " << logFile.getFullPathName().toStdString() << std::endl;
    std::cout << "[AnalogIQ] Log file exists: " << (logFile.exists() ? "true" : "false") << std::endl;
    std::cout << "[AnalogIQ] Log file is file: " << (logFile.existsAsFile() ? "true" : "false") << std::endl;
#endif

    if (logFile.existsAsFile())
    {
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] Log file exists, attempting to append..." << std::endl;
#endif

        auto timestamp = getLogTimestamp();
        auto logMessage = timestamp + ": " + message + "\n";

        auto result = logFile.appendText(logMessage);
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] Append result: " << (result ? "success" : "failed") << std::endl;
#endif
    }
    else
    {
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] WARNING: Log file not accessible: " << logFile.getFullPathName().toStdString() << std::endl;
        std::cout << "[AnalogIQ] Attempting to create log file..." << std::endl;
#endif

        // Try to create the file
        auto result = logFile.create();
#ifdef JUCE_DEBUG
        std::cout << "[AnalogIQ] File creation result: " << (result ? "success" : "failed") << std::endl;
#endif

        if (result)
        {
            // Try to write the message
            auto timestamp = getLogTimestamp();
            auto logMessage = timestamp + ": " + message + "\n";
            auto writeResult = logFile.appendText(logMessage);
#ifdef JUCE_DEBUG
            std::cout << "[AnalogIQ] Write result: " << (writeResult ? "success" : "failed") << std::endl;
#endif
        }
    }
}

/**
 * @brief Gets a formatted timestamp for logging.
 */
juce::String AnalogIQProcessor::getLogTimestamp()
{
    auto now = juce::Time::getCurrentTime();
    return now.toString(true, true, true, true);
}

/**
 * @brief Logs the structure of a ValueTree for debugging.
 */
void AnalogIQProcessor::logStateTreeStructure(const juce::ValueTree &tree, const juce::String &prefix)
{
    juce::Logger::writeToLog(prefix + " - Type: " + tree.getType().toString() + ", Properties: " + juce::String(tree.getNumProperties()) + ", Children: " + juce::String(tree.getNumChildren()));

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        auto child = tree.getChild(i);
        logStateTreeStructure(child, prefix + "  ");
    }
}

/**
 * @brief Logs the content of an XML element for debugging.
 */
void AnalogIQProcessor::logXmlContent(const juce::XmlElement &xml, const juce::String &prefix)
{
    juce::Logger::writeToLog(prefix + " - Tag: " + xml.getTagName() + ", Attributes: " + juce::String(xml.getNumAttributes()) + ", Children: " + juce::String(xml.getNumChildElements()));

    for (int i = 0; i < xml.getNumChildElements(); ++i)
    {
        auto child = xml.getChildElement(i);
        if (child != nullptr)
            logXmlContent(*child, prefix + "  ");
    }
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
    auto *editor = new AnalogIQEditor(*this, *fileSystem, *cacheManager, *presetManager, *gearLibrary);
    lastCreatedEditor = editor;

    // Store a reference to the rack for fallback operations when editor is not available
    if (auto *rackEditor = dynamic_cast<AnalogIQEditor *>(editor))
    {
        rack = rackEditor->getRack();
        juce::Logger::writeToLog("Stored rack reference for fallback operations");

        // Load instance state after the editor is created and gear library is loaded
        // We'll defer this to after the gear library is ready
        juce::MessageManager::callAsync([this, rackEditor]()
                                        {
            // Wait a bit more to ensure gear library is fully loaded
            juce::Timer::callAfterDelay(100, [this, rackEditor]()
            {
                if (auto *rack = rackEditor->getRack())
                {
                    loadInstanceState(rack);
                }
            }); });
    }
    return editor;
}

/**
 * @brief Gets the active editor.
 *
 * In a real plugin host, this is managed by JUCE. For testing purposes,
 * we return the last created editor.
 *
 * @return Pointer to the active editor, or nullptr if none exists
 */
juce::AudioProcessorEditor *AnalogIQProcessor::getActiveEditor()
{
    return lastCreatedEditor;
}

/**
 * @brief Saves the plugin's state.
 *
 * @param destData The memory block to save the state to
 */
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
        logStateTreeStructure(stateSnapshot, "Root");

        juce::Logger::writeToLog("Creating XML from state...");
        std::unique_ptr<juce::XmlElement> xml(stateSnapshot.createXml());

        if (xml != nullptr)
        {
            juce::Logger::writeToLog("XML created successfully");
            juce::Logger::writeToLog("XML children: " + juce::String(xml->getNumChildElements()));
            juce::Logger::writeToLog("XML tag name: " + xml->getTagName());

            // Log XML content for debugging
            logXmlContent(*xml, "Root");

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

    // Load instance state after restoring the main state
    // We need to wait for the editor to be created, so we'll do this in createEditor
}

/**
 * @brief Saves the current state of all gear instances.
 *
 * Saves the state of each gear instance in the rack, including
 * control values and settings, to the plugin's state tree.
 */
void AnalogIQProcessor::saveInstanceState()
{
    juce::Logger::writeToLog("=== saveInstanceState START ===");

    // Create a child tree for instance state
    juce::Logger::writeToLog("Creating/accessing instances tree...");
    auto instanceTree = state.state.getOrCreateChildWithName("instances", &undoManager);
    juce::Logger::writeToLog("Instances tree accessed successfully");
    juce::Logger::writeToLog("Instances tree children before clear: " + juce::String(instanceTree.getNumChildren()));

    // Clear existing instance data
    juce::Logger::writeToLog("Clearing existing instance data...");
    instanceTree.removeAllChildren(&undoManager);
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
            juce::Logger::writeToLog("Rack slots: " + juce::String(rack->getNumSlots()));
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

        // Check if we have a stored rack reference for fallback operations
        if (rack != nullptr)
        {
            juce::Logger::writeToLog("Using stored rack reference as fallback");
            juce::Logger::writeToLog("Stored rack slots: " + juce::String(rack->getNumSlots()));
            juce::Logger::writeToLog("WARNING: Using fallback rack reference - this may be stale if editor was destroyed");
            saveInstanceStateFromRack(rack, instanceTree);
        }
        else
        {
            juce::Logger::writeToLog("ERROR: No rack available - cannot save instance state");
            juce::Logger::writeToLog("This will result in an empty state being saved");
        }
    }

    juce::Logger::writeToLog("Final instances tree children: " + juce::String(instanceTree.getNumChildren()));
    juce::Logger::writeToLog("=== saveInstanceState END ===");
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
    juce::Logger::writeToLog("Rack slots: " + juce::String(rack->getNumSlots()));

    // Save instance data for each slot
    for (int i = 0; i < rack->getNumSlots(); ++i)
    {
        juce::Logger::writeToLog("Processing slot " + juce::String(i));

        if (auto *slot = rack->getSlot(i))
        {
            juce::Logger::writeToLog("Slot " + juce::String(i) + " obtained successfully");

            if (auto *item = slot->getGearItem())
            {
                juce::Logger::writeToLog("Slot " + juce::String(i) + " has gear item: " + item->name);
                juce::Logger::writeToLog("  Is instance: " + juce::String(item->isInstance ? "true" : "false"));
                juce::Logger::writeToLog("  Instance ID: " + item->instanceId);
                juce::Logger::writeToLog("  Unit ID: " + item->unitId);
                juce::Logger::writeToLog("  Controls count: " + juce::String(item->controls.size()));

                // Save state for instances only (all items in rack are now instances)
                if (item->isInstance && !item->instanceId.isEmpty() && !item->unitId.isEmpty())
                {
                    juce::Logger::writeToLog("Saving instance data for slot " + juce::String(i));

                    auto slotTree = instanceTree.getOrCreateChildWithName("slot_" + juce::String(i), &undoManager);
                    juce::Logger::writeToLog("Slot tree created for slot " + juce::String(i));

                    slotTree.setProperty("instanceId", item->instanceId, &undoManager);
                    slotTree.setProperty("sourceUnitId", item->sourceUnitId, &undoManager);
                    juce::Logger::writeToLog("Slot properties set for slot " + juce::String(i));

                    // Save control values
                    auto controlsTree = slotTree.getOrCreateChildWithName("controls", &undoManager);
                    juce::Logger::writeToLog("Controls tree created for slot " + juce::String(i));

                    for (int j = 0; j < item->controls.size(); ++j)
                    {
                        const auto &control = item->controls[j];
                        juce::Logger::writeToLog("Processing control " + juce::String(j) + " in slot " + juce::String(i));
                        juce::Logger::writeToLog("  Control name: " + control.name);
                        juce::Logger::writeToLog("  Control type: " + juce::String(static_cast<int>(control.type)));
                        juce::Logger::writeToLog("  Control value: " + juce::String(control.value));
                        juce::Logger::writeToLog("  Control initial value: " + juce::String(control.initialValue));

                        auto controlTree = controlsTree.getOrCreateChildWithName("control_" + juce::String(j), &undoManager);
                        controlTree.setProperty("value", control.value, &undoManager);
                        controlTree.setProperty("initialValue", control.initialValue, &undoManager);

                        if (control.type == GearControl::Type::Switch || control.type == GearControl::Type::Button)
                        {
                            controlTree.setProperty("currentIndex", control.currentIndex, &undoManager);
                            juce::Logger::writeToLog("  Control current index: " + juce::String(control.currentIndex));
                        }

                        juce::Logger::writeToLog("Control " + juce::String(j) + " saved successfully");
                    }

                    juce::Logger::writeToLog("Slot " + juce::String(i) + " completed successfully");
                }
                else
                {
                    juce::Logger::writeToLog("Slot " + juce::String(i) + " skipped - not a valid instance");
                    juce::Logger::writeToLog("  Is instance: " + juce::String(item->isInstance ? "true" : "false"));
                    juce::Logger::writeToLog("  Instance ID empty: " + juce::String(item->instanceId.isEmpty() ? "true" : "false"));
                    juce::Logger::writeToLog("  Unit ID empty: " + juce::String(item->unitId.isEmpty() ? "true" : "false"));
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
            auto notesTree = instanceTree.getOrCreateChildWithName("notes", &undoManager);
            auto notesContent = notesPanel->getText();
            notesTree.setProperty("content", notesContent, &undoManager);
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

/**
 * @brief Loads the state of all gear instances.
 *
 * Restores the state of each gear instance in the rack from
 * the plugin's state tree, including control values and settings.
 */
void AnalogIQProcessor::loadInstanceState(Rack *rack)
{
    // Get the instance state tree
    auto instanceTree = state.state.getChildWithName("instances");
    if (!instanceTree.isValid())
        return;

    if (rack != nullptr)
    {
        // Load instance data for each slot
        for (int i = 0; i < rack->getNumSlots(); ++i)
        {
            auto slotTree = instanceTree.getChildWithName("slot_" + juce::String(i));
            if (slotTree.isValid())
            {
                // Get the source unit ID from the saved state
                auto sourceUnitId = slotTree.getProperty("sourceUnitId").toString();
                if (!sourceUnitId.isEmpty())
                {
                    // Load the gear item from the gear library using the source unit ID
                    auto gearItem = gearLibrary->getGearItemByUnitId(sourceUnitId);
                    if (gearItem != nullptr)
                    {
                        // Create a new instance from the source gear using proper copy constructor
                        auto *item = new GearItem(*gearItem, networkFetcher, *fileSystem, *cacheManager);

                        // Set the gear item in the slot (this automatically creates an instance)
                        if (auto *slot = rack->getSlot(i))
                        {
                            slot->setGearItem(item);

                            // Validate that the instance was created successfully
                            if (item->isInstance && !item->instanceId.isEmpty())
                            {
                                // Get the gear item for this slot
                                if (auto *loadedItem = slot->getGearItem())
                                {
                                    // Capture control values before schema loading (following PresetManager pattern)
                                    struct SavedControlValues
                                    {
                                        int index;
                                        float value;
                                        float initialValue;
                                        int currentIndex;
                                    };
                                    juce::Array<SavedControlValues> savedControls;

                                    // Extract control values from saved state
                                    auto controlsTree = slotTree.getChildWithName("controls");
                                    if (controlsTree.isValid())
                                    {
                                        for (int j = 0; j < controlsTree.getNumChildren(); ++j)
                                        {
                                            auto controlTree = controlsTree.getChildWithName("control_" + juce::String(j));
                                            if (controlTree.isValid())
                                            {
                                                SavedControlValues saved;
                                                saved.index = j;
                                                saved.value = controlTree.getProperty("value", 0.0f);
                                                saved.initialValue = controlTree.getProperty("initialValue", 0.0f);
                                                saved.currentIndex = controlTree.getProperty("currentIndex", 0);
                                                savedControls.add(saved);
                                            }
                                        }
                                    }

                                    // Trigger faceplate and control image loading
                                    // We'll load the schema and then restore control values after
                                    rack->fetchSchemaForGearItem(loadedItem, [savedControls, loadedItem]()
                                                                 {
                                // Apply saved control values after schema parsing (following PresetManager pattern)
                                // Note: initialValue is preserved from schema, not restored from saved state
                                // Validate loadedItem is still valid
                                if (loadedItem != nullptr && savedControls.size() > 0)
                                {
                                    for (const auto& saved : savedControls)
                                    {
                                        if (saved.index >= 0 && saved.index < loadedItem->controls.size())
                                        {
                                            auto &control = loadedItem->controls.getReference(saved.index);
                                            control.value = saved.value;
                                            // Don't restore initialValue - keep the schema default

                                            if (control.type == GearControl::Type::Switch || control.type == GearControl::Type::Button)
                                            {
                                                control.currentIndex = saved.currentIndex;
                                            }
                                        }
                                    }
                                } });
                                }
                            }
                        }
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
                        notesPanel->setText(notesContent);
                    }
                }
            }
        }
    }
}

// Keep the original loadInstanceState for backward compatibility
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

/**
 * @brief Resets all gear instances to their default state.
 *
 * Calls resetAllInstances() on the rack to restore all gear
 * instances to their default settings.
 */
void AnalogIQProcessor::resetAllInstances()
{
    // First try to get the rack from the editor
    if (auto *editor = dynamic_cast<AnalogIQEditor *>(getActiveEditor()))
    {
        if (auto *rack = editor->getRack())
        {
            rack->resetAllInstances();
            return;
        }
    }

    // If no editor is available, try to use the stored rack reference as fallback
    if (rack != nullptr)
    {
        juce::Logger::writeToLog("Using stored rack reference for resetAllInstances fallback");
        juce::Logger::writeToLog("WARNING: Using fallback rack reference - this may be stale if editor was destroyed");
        rack->resetAllInstances();
    }
    else
    {
        juce::Logger::writeToLog("ERROR: No rack available for resetAllInstances - operation failed");
    }
}

void AnalogIQProcessor::clearRackReference()
{
    juce::Logger::writeToLog("Clearing stored rack reference to prevent dangling pointer usage");
    rack = nullptr;
}

/**
 * @brief Creates a new instance of the plugin.
 *
 * @return A new AnalogIQProcessor instance
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    static NetworkFetcher networkFetcher;
    static FileSystem fileSystem;
    return new AnalogIQProcessor(networkFetcher, fileSystem);
}