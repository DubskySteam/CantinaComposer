#include "SynthVoice.hpp"

SynthVoice::SynthVoice(juce::AudioProcessorValueTreeState& inApvts) : apvts(inApvts)
{
    osc.initialise([](float x) { return std::sin(x); }, 128);
    tempBlock.setSize(2, 512);
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int /*numOutputChannels*/)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    
    osc.prepare(spec);
    adsr.setSampleRate(sampleRate);
    tempBlock.setSize(2, samplesPerBlock);
    isPrepared = true;
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    if (!isPrepared) return;
    
    updateADSR();
    osc.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber), true);
    level = velocity * 0.15f;
    adsr.noteOn();
}

void SynthVoice::stopNote(float, bool allowTailOff)
{
    adsr.noteOff();
    if (!allowTailOff || !adsr.isActive())
    {
        clearCurrentNote();
    }
}
    
void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isPrepared || !isVoiceActive()) return;
            
    updateWaveform();

    tempBlock.clear();
    juce::dsp::AudioBlock<float> block(tempBlock);
    juce::dsp::ProcessContextReplacing<float> context(block);
    osc.process(context);
    
    adsr.applyEnvelopeToBuffer(tempBlock, 0, numSamples);

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, tempBlock, channel, 0, numSamples, level);
    }
    
    if (!adsr.isActive())
    {
        clearCurrentNote();
    }
}

void SynthVoice::updateADSR()
{
    adsrParams.attack = apvts.getRawParameterValue("ATTACK")->load();
    adsrParams.decay = apvts.getRawParameterValue("DECAY")->load();
    adsrParams.sustain = apvts.getRawParameterValue("SUSTAIN")->load();
    adsrParams.release = apvts.getRawParameterValue("RELEASE")->load();
    adsr.setParameters(adsrParams);
}
    
void SynthVoice::updateWaveform()
{
    auto waveType = static_cast<int>(apvts.getRawParameterValue("WAVE")->load());
    if (waveType == lastWaveType) return;

    switch (waveType)
    {
        case 0: osc.initialise([](float x) { return std::sin(x); }); break;
        case 1: osc.initialise([](float x) { return juce::jmap(x, 0.0f, juce::MathConstants<float>::twoPi, -1.0f, 1.0f); }); break;
        case 2: osc.initialise([](float x) { return x < juce::MathConstants<float>::pi ? 1.0f : -1.0f; }); break;
        default: osc.initialise([](float x) { return 0.0f; }); break;
    }
    lastWaveType = waveType;
}