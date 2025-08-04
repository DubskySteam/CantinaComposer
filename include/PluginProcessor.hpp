#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SynthVoice.hpp"

class CantinaComposerAudioProcessor  : public juce::AudioProcessor
{
public:
    CantinaComposerAudioProcessor();
    ~CantinaComposerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const override { return JucePlugin_ProducesMidiOutput; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState apvts;
    void setPreset(int presetIndex);

private:

    juce::Synthesiser synth;

    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterChain = juce::dsp::ProcessorChain<Filter, Filter>;
    FilterChain filterChain;
    
    juce::LinearSmoothedValue<float> smoothedFilterFreq;
    void updateFilters();

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CantinaComposerAudioProcessor)
};