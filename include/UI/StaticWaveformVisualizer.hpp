#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * @class StaticWaveformVisualizer
 * @brief A UI component that draws a static representation of the selected waveform.
 *
 * This class listens to parameter changes for the waveform type and the "Jizz Gobbler"
 * effect, and updates its display accordingly. It does not process live audio, but
 * rather simulates the waveshaping effects to provide a clean visual preview.
 * @ingroup UI
 */
class StaticWaveformVisualizer : public juce::Component,
                                 public juce::AudioProcessorValueTreeState::Listener
{
public:
    StaticWaveformVisualizer(juce::AudioProcessorValueTreeState& apvts);
    
    ~StaticWaveformVisualizer() override;

    /** @brief The core JUCE paint callback where the static waveform is drawn. */
    void paint(juce::Graphics& g) override;

    /** @brief Called when a listened-to parameter changes. */
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    /// @brief A reference to the main AudioProcessorValueTreeState.
    juce::AudioProcessorValueTreeState& valueTreeState;

    /// @brief Caches the last selected wave type.
    int currentWaveType = 0;
    /// @brief Caches the last "Jizz Gobbler" amount.
    float gobblerAmount = 0.0f;
};
