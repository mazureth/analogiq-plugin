cmake_minimum_required(VERSION 3.15)

project(AnalogIQ VERSION 0.1.0)

# Find the JUCE library
add_subdirectory(JUCE)

# Add options for AAX, VST3, etc.
option(ANALOGIQ_BUILD_AAX "Build AAX plugin" ON)
option(ANALOGIQ_BUILD_VST3 "Build VST3 plugin" ON)
option(ANALOGIQ_BUILD_AU "Build Audio Unit plugin" ON)
option(ANALOGIQ_BUILD_STANDALONE "Build Standalone app" ON)

# Set up the plugin target with JUCE modules
juce_add_plugin(AnalogIQ
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "Mazureth"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Yoco
    PLUGIN_CODE Ana1
    FORMATS
        ${ANALOGIQ_BUILD_AAX}
        ${ANALOGIQ_BUILD_VST3}
        ${ANALOGIQ_BUILD_AU}
        ${ANALOGIQ_BUILD_STANDALONE}
    VST3_CATEGORIES "Tools"
    AAX_CATEGORY "AAX_ePlugInCategory_Dynamics"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Set C++ standard to 17
target_compile_features(AnalogIQ PRIVATE cxx_std_17)

# Add source files
target_sources(AnalogIQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GearLibrary.cpp
    Source/GearItem.cpp
    Source/Rack.cpp
    Source/RackSlot.cpp
    Source/NotesPanel.cpp
)

# Set up JUCE dependencies
target_compile_definitions(AnalogIQ
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link with JUCE modules
target_link_libraries(AnalogIQ
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
) 