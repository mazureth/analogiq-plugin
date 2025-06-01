/**
 * @file PluginEditor.h
 * @brief Header file for the AnalogIQEditor class, which provides the main user interface for the AnalogIQ plugin.
 *
 * The AnalogIQEditor class is responsible for:
 * - Managing the main plugin window and layout
 * - Coordinating between the gear library, rack, and notes panel
 * - Handling drag and drop operations
 * - Managing UI state and user interactions
 *
 * @author AnalogIQ Team
 * @version 1.0
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GearLibrary.h"
#include "Rack.h"
#include "NotesPanel.h"

/**
 * @class AnalogIQEditor
 * @brief The main editor component for the AnalogIQ plugin.
 *
 * AnalogIQEditor provides the user interface for the plugin, including:
 * - A tabbed interface for organizing different views
 * - The gear library for browsing available gear
 * - The rack for managing gear instances
 * - A notes panel for user annotations
 */
class AnalogIQEditor : public juce::AudioProcessorEditor,
                       public juce::DragAndDropContainer
{
public:
    /**
     * @brief Constructs a new AnalogIQEditor.
     *
     * Initializes the editor with:
     * - A reference to the audio processor
     * - A tabbed interface for organizing views
     * - The gear library component
     * - The rack component
     * - The notes panel component
     *
     * @param p Reference to the audio processor
     */
    AnalogIQEditor(AnalogIQProcessor &p);

    /**
     * @brief Destructor for AnalogIQEditor.
     */
    ~AnalogIQEditor() override;

    /**
     * @brief Paints the editor background.
     * @param g The graphics context to paint with
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Called when the component is resized.
     * Updates the layout of all child components.
     */
    void resized() override;

    /**
     * @brief Gets a pointer to the rack component.
     * @return Pointer to the rack component
     */
    Rack *getRack() const { return rack.get(); }

private:
    AnalogIQProcessor &audioProcessor; ///< Reference to the audio processor

    // UI Components
    juce::TabbedComponent mainTabs{juce::TabbedButtonBar::TabsAtTop}; ///< Main tabbed interface
    std::unique_ptr<GearLibrary> gearLibrary;                         ///< Gear library component
    std::unique_ptr<Rack> rack;                                       ///< Rack component
    std::unique_ptr<NotesPanel> notesPanel;                           ///< Notes panel component

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogIQEditor)
};