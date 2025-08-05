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
    
    smoothedFrequency.reset(sampleRate, 0.05);

    tempBlock.setSize(2, samplesPerBlock); 
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    if (!isPrepared) return;
    
    updateADSR();

    double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto* pitchParam = apvts.getRawParameterValue("PITCH");
    float pitchOffset = pitchParam ? pitchParam->load() : 0.0f;
    double finalFrequency = baseFrequency * std::pow(2.0, pitchOffset / 12.0);

    smoothedFrequency.setCurrentAndTargetValue(finalFrequency);

    level = velocity * 0.15f;
    adsr.noteOn();
}

void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
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

    updateADSR();
    updateWaveform();

    double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(getCurrentlyPlayingNote());
    auto* pitchParam = apvts.getRawParameterValue("PITCH");
    float pitchOffset = pitchParam ? pitchParam->load() : 0.0f;
    double targetFrequency = baseFrequency * std::pow(2.0, pitchOffset / 12.0);
    smoothedFrequency.setTargetValue(targetFrequency);

    osc.setFrequency(smoothedFrequency.getNextValue(), true);

    tempBlock.clear();
    juce::dsp::AudioBlock<float> block(tempBlock);
    auto blockToProcess = block.getSubBlock(0, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(blockToProcess);
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

    if (waveType == lastWaveType) return;

    switch (waveType)
    {
        case 0: osc.initialise([](float x) { return std::sin(x); }); break; 
        case 1: osc.initialise([](float x) { return juce::jmap(x, 0.0f, juce::MathConstants<float>::twoPi, -1.0f, 1.0f); }); break;
        case 2: osc.initialise([](float x) { return std::copysign(1.0f, std::sin(x)); }); break; 
        default: osc.initialise([](float x) { return 0.0f; }); break;
    }

    lastWaveType = waveType;
}

