# View layer CMakeLists.txt

set(VIEW_SOURCES
    # View sources will be added here as they're implemented
)

# Create view library target
add_library(AnalogIQView STATIC ${VIEW_SOURCES})

target_link_libraries(AnalogIQView
    PRIVATE
        juce::juce_core
        juce::juce_graphics
        juce::juce_gui_basics
        AnalogIQShared
        AnalogIQModel
)

target_include_directories(AnalogIQView
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
