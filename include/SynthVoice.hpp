#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice(juce::AudioProcessorValueTreeState& inApvts);
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numOutputChannels);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

private:
    void updateADSR();
    void updateWaveform();

    bool isPrepared = false;
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::dsp::Oscillator<float> osc;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    juce::AudioBuffer<float> tempBlock;
    float level = 0.0f;
    int lastWaveType = -1;
};