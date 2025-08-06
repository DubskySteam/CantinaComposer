#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SynthVoice.hpp"
#include "AudioBufferQueue.hpp"

/**
 * @class CantinaComposerAudioProcessor
 * @brief The core audio processing class for the CantinaComposer plugin.
 *
 * This class manages the synthesizer, the effects chain (filter, reverb, distortion),
 * and all user-facing parameters via the AudioProcessorValueTreeState (APVTS).
 * It handles all interaction with the DAW/host.
 * @defgroup Processor Audio Processor
 */
class CantinaComposerAudioProcessor : public juce::AudioProcessor
{
public:
    CantinaComposerAudioProcessor();
    ~CantinaComposerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const override { return JucePlugin_ProducesMidiOutput; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String &) override {}

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    /** @brief Applies parameter values for a selected preset.
     *  @param presetIndex The zero-based index of the preset to load.
     */
    void setPreset(int presetIndex);

    /** @brief A queue to pass audio data safely from the audio thread to the UI thread for visualization. */
    AudioBufferQueue audioBufferQueue;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /** @brief Updates the filter parameters based on the current APVTS values. */
    void updateFilters();

    /// @brief The main synthesizer engine.
    juce::Synthesiser synth;

    /// @brief The audio processing chain for the filter section.
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterChain = juce::dsp::ProcessorChain<Filter, Filter>;
    FilterChain filterChain;
    /// @brief A smoothed value for the filter frequency to prevent audio clicks.
    juce::LinearSmoothedValue<float> smoothedFilterFreq;

    // --- Effects ---
    /// @brief The reverb module for the "Space Wobbler" effect.
    juce::dsp::Reverb reverb;
    /// @brief The parameter block for the reverb module.
    juce::dsp::Reverb::Parameters reverbParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CantinaComposerAudioProcessor)
};