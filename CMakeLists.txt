cmake_minimum_required(VERSION 3.15)

project(AnalogIQ VERSION 0.1.0)

# Enable testing
enable_testing()

# Find Doxygen
find_package(Doxygen REQUIRED)

# Set compiler flags for macOS 15.0 compatibility
if(APPLE)
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options(-Wno-unused-but-set-variable)
endif()

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use FetchContent to get JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG master  # Using master branch for latest macOS 15.0 support
)
FetchContent_MakeAvailable(JUCE)

# Add options for AAX, VST3, etc.
option(ANALOGIQ_BUILD_AAX "Build AAX plugin" ON)
option(ANALOGIQ_BUILD_VST3 "Build VST3 plugin" ON)
option(ANALOGIQ_BUILD_AU "Build Audio Unit plugin" ON)
option(ANALOGIQ_BUILD_STANDALONE "Build Standalone app" ON)
option(ANALOGIQ_BUILD_TESTS "Build and run tests" ON)
option(ANALOGIQ_COPY_PLUGINS "Copy plugins after build" ON)

# Build the list of formats based on options
set(PLUGIN_FORMATS "")
if(ANALOGIQ_BUILD_AAX)
    list(APPEND PLUGIN_FORMATS AAX)
endif()
if(ANALOGIQ_BUILD_VST3)
    list(APPEND PLUGIN_FORMATS VST3)
endif()
if(ANALOGIQ_BUILD_AU)
    list(APPEND PLUGIN_FORMATS AU)
endif()
if(ANALOGIQ_BUILD_STANDALONE)
    list(APPEND PLUGIN_FORMATS Standalone)
endif()

# Set up the plugin target with JUCE modules
juce_add_plugin(AnalogIQ
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "Mazureth"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD ${ANALOGIQ_COPY_PLUGINS}
    PLUGIN_MANUFACTURER_CODE Yoco
    PLUGIN_CODE Ana1
    FORMATS ${PLUGIN_FORMATS}
    VST3_CATEGORIES "Tools"
    AAX_CATEGORY "AAX_ePlugInCategory_Dynamics"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Set C++ standard to 17
target_compile_features(AnalogIQ PRIVATE cxx_std_17)

# Add source files
target_sources(AnalogIQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/GearLibrary.cpp
    Source/GearItem.cpp
    Source/Rack.cpp
    Source/RackSlot.cpp
    Source/NotesPanel.cpp
    Source/NotesPanel.h
    Source/NetworkFetcher.cpp
    Source/NetworkFetcher.h
    Source/INetworkFetcher.h
)

# Set up JUCE dependencies
target_compile_definitions(AnalogIQ
    PUBLIC
    JUCE_WEB_BROWSER=0
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

# Generate JuceHeader.h
juce_generate_juce_header(AnalogIQ)

# Add documentation target
if(DOXYGEN_FOUND)
    # Create docs directory
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add a custom target to run Doxygen
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    # Make sure the docs target depends on the main target
    add_dependencies(docs AnalogIQ)
endif()

# Add subdirectories
add_subdirectory(Source)

# Add tests if enabled
if(ANALOGIQ_BUILD_TESTS)
    add_subdirectory(tests)
endif()