#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessor::CantinaComposerAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.state.addListener(this);
    synth.addSound(new SynthSound());

    for (int i = 0; i < 8; ++i)
    {
        synth.addVoice(new SynthVoice(apvts));
    }
}

CantinaComposerAudioProcessor::~CantinaComposerAudioProcessor()
{
    apvts.state.removeListener(this);
}

juce::AudioProcessorValueTreeState::ParameterLayout CantinaComposerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::StringArray waveChoices = { "Sine", "Saw", "Square" };
    juce::StringArray presetChoices = { "Kloo Horn (Flute)", "Fanfar (Steel Drum)", "Gasan String-drum", "Ommni Box (Clarinet)" };

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("PRESET", "Preset", presetChoices, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("WAVE", "Waveform", waveChoices, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float>(0.01f, 1.0f, 0.001f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float>(0.01f, 1.0f, 0.001f, 0.3f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float>(0.01f, 3.0f, 0.001f, 0.3f), 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FILTER_FREQ", "Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("BASS_GAIN", "Bass", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f)); 
    return { params.begin(), params.end() };
}

void CantinaComposerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    filterChain.prepare(spec);
    smoothedFilterFreq.reset(sampleRate, 0.05);
    
    updateFilters();

    if (auto* presetParam = apvts.getRawParameterValue("PRESET"))
    {
        setPreset(static_cast<int>(presetParam->load()));
    }
}

void CantinaComposerAudioProcessor::releaseResources() {}

void CantinaComposerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    updateFilters();
    
    juce::dsp::AudioBlock<float> block (buffer);
    filterChain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void CantinaComposerAudioProcessor::updateFilters()
{
    auto freq = apvts.getRawParameterValue("FILTER_FREQ")->load();
    smoothedFilterFreq.setTargetValue(freq);
    auto bassGain = apvts.getRawParameterValue("BASS_GAIN")->load();

    *filterChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), smoothedFilterFreq.getNextValue());
    
    *filterChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 150.0f, 1.0f, juce::Decibels::decibelsToGain(bassGain));
}

void CantinaComposerAudioProcessor::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == juce::Identifier("PRESET"))
    {
        setPreset(static_cast<int>(tree.getProperty(property)));
    }
}

void CantinaComposerAudioProcessor::setPreset(int presetIndex)
{
    auto* waveParam = apvts.getParameter("WAVE");
    auto* attackParam = apvts.getParameter("ATTACK");
    auto* decayParam = apvts.getParameter("DECAY");
    auto* sustainParam = apvts.getParameter("SUSTAIN");
    auto* releaseParam = apvts.getParameter("RELEASE");
    auto* freqParam = apvts.getParameter("FILTER_FREQ");
    auto* bassParam = apvts.getParameter("BASS_GAIN");

    if (!waveParam || !attackParam || !decayParam || !sustainParam || !releaseParam || !freqParam || !bassParam)
    {
        jassertfalse;
        return;
    }

    auto setParam = [](juce::RangedAudioParameter* param, float value) {
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(value));
    };

    switch (presetIndex)
    {
        case 0:
            waveParam->setValueNotifyingHost(0);
            setParam(attackParam, 0.08f);
            setParam(decayParam, 0.3f);
            setParam(sustainParam, 0.8f);
            setParam(releaseParam, 0.4f);
            setParam(freqParam, 8000.0f);
            setParam(bassParam, -6.0f);
            break;

        case 1:
            waveParam->setValueNotifyingHost(0);
            setParam(attackParam, 0.01f);
            setParam(decayParam, 0.5f);
            setParam(sustainParam, 0.0f);
            setParam(releaseParam, 0.3f);
            setParam(freqParam, 12000.0f); 
            setParam(bassParam, 0.0f);
            break;

        case 2:
            waveParam->setValueNotifyingHost(1);
            setParam(attackParam, 0.02f);
            setParam(decayParam, 0.6f);
            setParam(sustainParam, 0.5f);
            setParam(releaseParam, 0.8f);
            setParam(freqParam, 6500.0f);
            setParam(bassParam, 3.0f);
            break;

        case 3:
            waveParam->setValueNotifyingHost(2);
            setParam(attackParam, 0.12f);
            setParam(decayParam, 0.1f);
            setParam(sustainParam, 1.0f);
            setParam(releaseParam, 0.2f);
            setParam(freqParam, 4000.0f);
            setParam(bassParam, -2.0f);
            break;
    }
}

juce::AudioProcessorEditor* CantinaComposerAudioProcessor::createEditor()
{
    return new CantinaComposerAudioProcessorEditor (*this);
}

void CantinaComposerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CantinaComposerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CantinaComposerAudioProcessor();
}