#include "SynthVoice.hpp"

SynthVoice::SynthVoice(juce::AudioProcessorValueTreeState& inApvts) : apvts(inApvts)
{
    osc.initialise([](float x) { return std::sin(x); }, 128);
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    isPrepared = true;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<unsigned int>(samplesPerBlock);
    spec.numChannels = static_cast<unsigned int>(numOutputChannels);

    osc.prepare(spec);
    adsr.setSampleRate(sampleRate);
    
    // Reset the frequency smoother with the host's sample rate and a 50ms ramp time.
    smoothedFrequency.reset(sampleRate, 0.05);

    // The temp block must be large enough to hold a full buffer of audio data.
    tempBlock.setSize(2, samplesPerBlock); 
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    // This voice can play any sound that is a SynthSound.
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    if (!isPrepared) return;
    
    updateADSR(); // Load the latest ADSR settings from the UI.

    // Convert the incoming MIDI note number (e.g., 69) to a frequency in Hz (e.g., 440).
    double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    // Get the pitch offset in semitones from our "Blaster" slider.
    auto* pitchParam = apvts.getRawParameterValue("PITCH");
    float pitchOffset = pitchParam ? pitchParam->load() : 0.0f;
    // Calculate the final frequency, including the pitch offset.
    double finalFrequency = baseFrequency * std::pow(2.0, pitchOffset / 12.0);

    // Set the frequency immediately when a note starts to avoid an audible "slide up" effect.
    smoothedFrequency.setCurrentAndTargetValue(finalFrequency);

    // The note's volume is determined by its MIDI velocity.
    level = velocity * 0.15f;

    // Trigger the "note on" phase of the ADSR envelope.
    adsr.noteOn();
}

void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    // Trigger the "note off" (release) phase of the ADSR envelope.
    adsr.noteOff();

    // If tail-off is not allowed, or the note is already silent, deactivate the voice immediately.
    if (!allowTailOff || !adsr.isActive())
    {
        clearCurrentNote();
    }
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isPrepared || !isVoiceActive()) return;

    // Update sound-shaping parameters on every block.
    updateADSR();
    updateWaveform();

    // Continuously update the target frequency based on the pitch slider.
    double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(getCurrentlyPlayingNote());
    auto* pitchParam = apvts.getRawParameterValue("PITCH");
    float pitchOffset = pitchParam ? pitchParam->load() : 0.0f;
    double targetFrequency = baseFrequency * std::pow(2.0, pitchOffset / 12.0);
    smoothedFrequency.setTargetValue(targetFrequency);

    // Set the oscillator's frequency to the next smoothed value. This prevents clicks.
    osc.setFrequency(smoothedFrequency.getNextValue(), true);

    // Generate the raw tone into our temporary buffer.
    tempBlock.clear();
    juce::dsp::AudioBlock<float> block(tempBlock);
    auto blockToProcess = block.getSubBlock(0, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(blockToProcess);
    osc.process(context);

    // Apply the ADSR envelope to the raw tone, shaping its volume over time.
    adsr.applyEnvelopeToBuffer(tempBlock, 0, numSamples);

    // Add the voice's processed audio to the main output buffer.
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, tempBlock, channel, 0, numSamples, level);
    }

    // If the note has finished its release phase, this voice is now free to be reused.
    if (!adsr.isActive())
    {
        clearCurrentNote();
    }
}

// Was planned to implement for full midi support. But not required for the plugin functionality.
void SynthVoice::pitchWheelMoved(int) {}
void SynthVoice::controllerMoved(int, int) {}

void SynthVoice::updateADSR()
{
    adsrParams.attack  = apvts.getRawParameterValue("ATTACK")->load();
    adsrParams.decay   = apvts.getRawParameterValue("DECAY")->load();
    adsrParams.sustain = apvts.getRawParameterValue("SUSTAIN")->load();
    adsrParams.release = apvts.getRawParameterValue("RELEASE")->load();
    adsr.setParameters(adsrParams);
}

void SynthVoice::updateWaveform()
{
    auto waveType = static_cast<int>(apvts.getRawParameterValue("WAVE")->load());

    if (waveType == lastWaveType) return; // Saves time

    switch (waveType)
    {
        case 0: osc.initialise([](float x) { return std::sin(x); }); break; // Sine
        case 1: osc.initialise([](float x) { return juce::jmap(x, 0.0f, juce::MathConstants<float>::twoPi, -1.0f, 1.0f); }); break; // Saw
        case 2: osc.initialise([](float x) { return std::copysign(1.0f, std::sin(x)); }); break; // Square
        default: osc.initialise([](float x) { return 0.0f; }); break; // If we get here, I seriously fucked something up
    }

    lastWaveType = waveType;
}

