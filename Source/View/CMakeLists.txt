# View layer CMakeLists.txt
# This file manages the View layer components for the AnalogIQ plugin

# Define View layer sources
set(VIEW_SOURCES
    AnalogIQEditor.h
    AnalogIQEditor.cpp
    NotesPanel.h
    NotesPanel.cpp
    Rack.h
    Rack.cpp
    RackSlot.h
    RackSlot.cpp
    # Additional view sources will be added here
)

# Create View library target
add_library(AnalogIQView STATIC ${VIEW_SOURCES})

# Link with JUCE modules
target_link_libraries(AnalogIQView
    PRIVATE
        juce::juce_core
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_processors
        juce::juce_audio_utils
)

# Set include directories
target_include_directories(AnalogIQView
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../Model
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/AnalogIQ_artefacts/JuceLibraryCode
)
