/**
 * @file PluginEditor.h
 * @brief Header file for the AnalogIQEditor class.
 *
 * This file defines the main editor interface for the AnalogIQ plugin,
 * providing a user interface for managing audio gear, racks, and session notes.
 * It includes a gear library, rack interface, and notes panel organized in a
 * tabbed layout.
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GearLibrary.h"
#include "Rack.h"
#include "NotesPanel.h"
#include "NetworkFetcher.h"

/**
 * @brief Main editor interface for the AnalogIQ plugin.
 *
 * The AnalogIQEditor class provides the user interface for the plugin,
 * managing the gear library, rack, and notes panel. It supports drag and drop
 * operations for gear items and organizes the interface using a tabbed layout.
 */
class AnalogIQEditor : public juce::AudioProcessorEditor,
                       public juce::DragAndDropContainer
{
public:
    /**
     * @brief Constructs a new AnalogIQEditor.
     *
     * @param processor Reference to the associated AudioProcessor
     */
    AnalogIQEditor(AnalogIQProcessor &);

    /**
     * @brief Destructor for AnalogIQEditor.
     */
    ~AnalogIQEditor() override;

    /**
     * @brief Paints the AnalogIQEditor component.
     *
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &) override;

    /**
     * @brief Handles resizing of the AnalogIQEditor component.
     */
    void resized() override;

    /**
     * @brief Gets a pointer to the rack component.
     *
     * @return Pointer to the Rack component
     */
    Rack *getRack() const { return rack.get(); }

private:
    AnalogIQProcessor &audioProcessor; ///< Reference to the associated AudioProcessor
    NetworkFetcher networkFetcher;     ///< Network fetcher for making HTTP requests

    // UI Components
    juce::TabbedComponent mainTabs{juce::TabbedButtonBar::TabsAtTop}; ///< Main tabbed interface
    std::unique_ptr<GearLibrary> gearLibrary;                         ///< Gear library component
    std::unique_ptr<Rack> rack;                                       ///< Rack component
    std::unique_ptr<NotesPanel> notesPanel;                           ///< Notes panel component

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQEditor)
};