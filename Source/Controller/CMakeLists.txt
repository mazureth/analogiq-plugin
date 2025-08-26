# Controller layer CMakeLists.txt

set(CONTROLLER_SOURCES
    MainController.h
    MainController.cpp
    GearLibraryController.h
    GearLibraryController.cpp
    # Additional controller sources will be added here
)

# Create controller library target
add_library(AnalogIQController STATIC ${CONTROLLER_SOURCES})

target_link_libraries(AnalogIQController
    PRIVATE
        juce::juce_core
        juce::juce_graphics
        juce::juce_gui_basics
        AnalogIQShared
        AnalogIQModel
        AnalogIQView
)

target_include_directories(AnalogIQController
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../Model
        ${CMAKE_CURRENT_SOURCE_DIR}/../View
        ${CMAKE_BINARY_DIR}/AnalogIQ_artefacts/JuceLibraryCode
)
