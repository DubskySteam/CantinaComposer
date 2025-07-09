#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class WaveformVisualizer : public juce::Component,
                           private juce::AudioProcessorParameter::Listener,
                           private juce::Timer
{
public:
    WaveformVisualizer(juce::AudioProcessorValueTreeState& apvts);
    ~WaveformVisualizer() override;

    void paint(juce::Graphics& g) override;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void timerCallback() override;
    void parameterGestureChanged(int, bool) override {}

private:
    float previousWaveShape(float normalizedX);
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::AudioProcessorParameter* waveParam = nullptr;
    int currentWaveType = 1;
    juce::LinearSmoothedValue<float> transition { 1.f };
    juce::Array<float> previousWaveSnapshot;
};