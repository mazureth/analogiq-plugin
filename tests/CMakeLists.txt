# Enable testing
enable_testing()

# Create test executable
add_executable(analogiq_tests
    main.cpp
    unit/TestHelpers.h
    unit/TestFixture.h
    unit/MockNetworkFetcher.h
    unit/MockRackStateListener.h
    unit/MockFileSystem.h
    unit/MockStateVerifier.h
    unit/GearLibraryTests.cpp
    unit/GearItemTests.cpp
    unit/RackTests.cpp
    unit/RackSlotTests.cpp
    unit/DraggableListBoxTests.cpp
    unit/NotesPanelTests.cpp
    unit/PluginProcessorTests.cpp
    unit/PluginEditorTests.cpp
    unit/CacheManagerTests.cpp
    unit/PresetManagerTests.cpp
    unit/PresetIntegrationTests.cpp
)

# Set C++ standard
target_compile_features(analogiq_tests PRIVATE cxx_std_17)

# Link with JUCE modules
target_link_libraries(analogiq_tests
    PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    AnalogIQ
)

# Add include directories
target_include_directories(analogiq_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_BINARY_DIR}/AnalogIQ_artefacts/JuceLibraryCode # For JuceHeader.h
)

# Add test to CTest
add_test(NAME analogiq_tests COMMAND analogiq_tests)

# Set JUCE unit test specific definitions
target_compile_definitions(analogiq_tests
    PRIVATE
    JUCE_UNIT_TESTS=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_UNIT_TESTS_VERBOSE=1
    JUCE_DONT_ENABLE_LEAK_DETECTOR=1
    RUNNING_TESTS=1
) 

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling code coverage for test target")

    # Coverage for main library
    target_compile_options(AnalogIQ PRIVATE
        -O0 -g --coverage -fprofile-instr-generate -fcoverage-mapping
    )
    target_link_options(AnalogIQ PRIVATE
        --coverage -fprofile-instr-generate -fcoverage-mapping
    )

    # Coverage for tests
    target_compile_options(analogiq_tests PRIVATE
        -O0 -g --coverage -fprofile-instr-generate -fcoverage-mapping
    )

    target_link_options(analogiq_tests PRIVATE
        --coverage -fprofile-instr-generate -fcoverage-mapping
    )
endif()