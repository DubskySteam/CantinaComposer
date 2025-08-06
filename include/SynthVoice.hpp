#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
 * @class SynthSound
 * @brief A simple SynthesiserSound that can be played by any voice.
 * This class acts as a tag, letting the main Synthesiser know which voices
 * are compatible with which sounds. In our case, all voices can play this sound.
 * @ingroup Processor
 */
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

/**
 * @class SynthVoice
 * @brief Represents a single voice of the synthesizer.
 *
 * Each instance of this class can play one note at a time. It manages its own
 * oscillator, ADSR envelope, and pitch, pulling parameter values from the APVTS.
 * @ingroup Processor
 */
class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice(juce::AudioProcessorValueTreeState& inApvts);

    /** @brief Prepares the voice's internal DSP components for playback. */
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numOutputChannels);

    /** @brief Determines if this voice can play a given sound. */
    bool canPlaySound(juce::SynthesiserSound* sound) override;

    /** @brief Called by the synthesiser when a MIDI note-on is received. */
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;

    /** @brief Called by the synthesiser when a MIDI note-off is received. */
    void stopNote(float velocity, bool allowTailOff) override;

    /** @brief Renders the next block of audio for this voice. */
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    /** Not needed because I didn't implement the standard full midi */
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;

private:
    /** @brief Updates the ADSR parameters from the APVTS. */
    void updateADSR();
    /** @brief Updates the oscillator's waveform from the APVTS. */
    void updateWaveform();

    /// @brief Flag to ensure prepareToPlay has been called.
    bool isPrepared = false;

    /// @brief A reference to the main AudioProcessorValueTreeState.
    juce::AudioProcessorValueTreeState& apvts;
    /// @brief The oscillator that generates the basic tone.
    juce::dsp::Oscillator<float> osc;
    /// @brief The ADSR envelope generator.
    juce::ADSR adsr;
     /// @brief The parameter block for the ADSR.
    juce::ADSR::Parameters adsrParams;
    /// @brief A temporary buffer to render audio into before applying 
    juce::AudioBuffer<float> tempBlock;

    /// @brief A smoother to prevent audio clicks when the pitch changes.
    juce::LinearSmoothedValue<double> smoothedFrequency; 

    /// @brief The velocity-based level of the current note.
    float level = 0.0f;
    /// @brief Caches the last selected wave type to avoid unnecessary re-initialization.
    int lastWaveType = -1;
};