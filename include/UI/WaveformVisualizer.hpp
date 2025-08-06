#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "AudioBufferQueue.hpp"

/**
 * @class WaveformVisualizer
 * @brief A UI component that draws a live representation of an audio signal.
 *
 * This class inherits from juce::Component to be a drawable element in the UI,
 * and from juce::Timer to periodically refresh its display. It reads audio data
 * from an AudioBufferQueue, which is safely fed by the real-time audio thread.
 * @defgroup UI User Interface
 */

class WaveformVisualizer : public juce::Component,
                           private juce::Timer
{
public:
    WaveformVisualizer(AudioBufferQueue& queue);
    ~WaveformVisualizer() override;

    /**
     * @brief The paint callback where all drawing occurs.
     * @param g The graphics context to draw into.
     */
    void paint(juce::Graphics& g) override;    
    /**
     * @brief The callback for the juce::Timer. This is called periodically.
     * Its sole purpose is to trigger a repaint of the component.
     */
    void timerCallback() override;
private:
    /// @brief A reference to the queue that safely transfers audio data from the audio thread.
    AudioBufferQueue& audioBufferQueue;
};