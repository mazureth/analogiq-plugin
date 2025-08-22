# Controller layer CMakeLists.txt

set(CONTROLLER_SOURCES
    # Controller sources will be added here as they're implemented
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
)
