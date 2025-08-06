#-------------------------------------------------------------------
# Repository: https://github.com/dubskysteam/cantinacomposer
#-------------------------------------------------------------------

cmake_minimum_required(VERSION 3.25)

project(CantinaComposer
    VERSION 1.0.0
)

#-------------------------------------------------------------------
# Core
#-------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Neovim clangd LSP

#-------------------------------------------------------------------
# Output
#-------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_DEBUG_POSTFIX "_d")

#-------------------------------------------------------------------
# CPM
#-------------------------------------------------------------------
include(cmake/CPM.cmake)

cpmaddpackage(
  NAME JUCE
  GIT_TAG 8.0.8
  VERSION 8.0.8
  GITHUB_REPOSITORY juce-framework/JUCE
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/juce
)

#-------------------------------------------------------------------
# JUCE Plugin Target
#-------------------------------------------------------------------
juce_add_plugin(
  ${PROJECT_NAME}
  COMPANY_NAME "CantinaComposer"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  PLUGIN_MANUFACTURER_CODE Nnja
  PLUGIN_CODE Nnja
  FORMATS VST3 Standalone
  PRODUCT_NAME "CantinaComposerVST3"
)

#-------------------------------------------------------------------
# Sources
#-------------------------------------------------------------------
target_sources(${PROJECT_NAME}
    PRIVATE
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
        src/SynthVoice.cpp
        src/UI/CustomLookAndFeel.cpp
        src/UI/WaveformVisualizer.cpp
        src/UI/StaticWaveformVisualizer.cpp
)

#-------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/UI
)

#-------------------------------------------------------------------
# JUCE / Build Flags
#-------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)


#-------------------------------------------------------------------
# Plugin install
#-------------------------------------------------------------------
include(cmake/InstallVST3.cmake)
install_plug(${PROJECT_NAME})