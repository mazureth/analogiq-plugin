# Shared layer CMakeLists.txt

set(SHARED_SOURCES
    IFileSystem.h
    INetworkFetcher.h
    IRackStateListener.h
    ICacheManager.h
    IPresetManager.h
    GearControl.h
    GearControl.cpp
)

# Create shared library target
add_library(AnalogIQShared STATIC ${SHARED_SOURCES})

target_link_libraries(AnalogIQShared
    PRIVATE
        juce::juce_core
        juce::juce_graphics
)

target_include_directories(AnalogIQShared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
