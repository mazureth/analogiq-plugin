# Model layer CMakeLists.txt

set(MODEL_SOURCES
    FileSystem.h
    FileSystem.cpp
    NetworkFetcher.h
    NetworkFetcher.cpp
    CacheManager.h
    CacheManager.cpp
    PresetManager.h
    PresetManager.cpp
    GearItem.h
    GearItem.cpp
    GearLibrary.h
    GearLibrary.cpp
    RackModel.h
    RackModel.cpp
    # Additional model sources will be added here
)

# Create model library target
add_library(AnalogIQModel STATIC ${MODEL_SOURCES})

target_link_libraries(AnalogIQModel
    PRIVATE
        juce::juce_core
        juce::juce_graphics
        AnalogIQShared
)

target_include_directories(AnalogIQModel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
