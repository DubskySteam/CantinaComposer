#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "AudioBufferQueue.hpp"

class WaveformVisualizer : public juce::Component,
                           private juce::Timer
{
public:
    WaveformVisualizer(AudioBufferQueue& queue);
    ~WaveformVisualizer() override;

    void paint(juce::Graphics& g) override;    
    void timerCallback() override;
private:
    AudioBufferQueue& audioBufferQueue;
};