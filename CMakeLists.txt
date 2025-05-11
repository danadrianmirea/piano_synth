cmake_minimum_required(VERSION 3.14)
project(piano_synth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set JUCE path to the submodule
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/JUCE")

# Add JUCE as a subdirectory to build it
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)

# Add JUCE modules path
list(APPEND CMAKE_MODULE_PATH "${JUCE_PATH}/extras/Build/CMake")

# Include JUCE
include("${JUCE_PATH}/extras/Build/CMake/JUCEUtils.cmake")
include("${JUCE_PATH}/extras/Build/CMake/JUCEModuleSupport.cmake")

# Set JUCE modules path
set(JUCE_MODULES_PATH "${JUCE_PATH}/modules")

# Add source files
set(SOURCES
    src/main.cpp
)

# Create JUCE console application
juce_add_console_app(${PROJECT_NAME}
    PRODUCT_NAME "Piano Synth"
    COMPANY_NAME "Your Company"
    BUNDLE_ID "com.yourcompany.pianosynth"
)

# Add source files to the target
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Set JUCE project properties
juce_generate_juce_header(${PROJECT_NAME})

# Add JUCE modules
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_MODULE_AVAILABLE_juce_core=1
    JUCE_MODULE_AVAILABLE_juce_audio_basics=1
    JUCE_MODULE_AVAILABLE_juce_audio_devices=1
    JUCE_MODULE_AVAILABLE_juce_audio_formats=1
    JUCE_MODULE_AVAILABLE_juce_audio_processors=1
    JUCE_MODULE_AVAILABLE_juce_audio_utils=1
    JUCE_MODULE_AVAILABLE_juce_data_structures=1
    JUCE_MODULE_AVAILABLE_juce_events=1
    JUCE_MODULE_AVAILABLE_juce_graphics=1
    JUCE_MODULE_AVAILABLE_juce_gui_basics=1
)

# Link against JUCE modules
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        juce_core
        juce_audio_basics
        juce_audio_devices
        juce_audio_formats
        juce_audio_processors
        juce_audio_utils
        juce_data_structures
        juce_events
        juce_graphics
        juce_gui_basics
)

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JUCE_MODULES_PATH}
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Set JUCE module search paths
set_target_properties(${PROJECT_NAME} PROPERTIES
    JUCE_MODULE_SEARCH_PATHS "${JUCE_MODULES_PATH}"
)