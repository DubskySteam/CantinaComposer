#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessor::CantinaComposerAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    synth.addSound(new SynthSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new SynthVoice(apvts));
}

CantinaComposerAudioProcessor::~CantinaComposerAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CantinaComposerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // Available waves
    juce::StringArray waveChoices = { "Sine", "Saw", "Square" };
    // Available presets
    juce::StringArray presetChoices = { "Kloo Horn (Flute)", "Fanfar (Steel Drum)", "Gasan String-drum", "Ommni Box (Clarinet)" };

    // --- Main Synth Parameters ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("PRESET", "Preset", presetChoices, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("WAVE", "Waveform", waveChoices, 0));
    // --- Galactic Envelope (ADSR) ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float>(0.01f, 1.0f, 0.001f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float>(0.01f, 1.0f, 0.001f, 0.3f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float>(0.01f, 3.0f, 0.001f, 0.3f), 0.4f));
    // --- Filter & Tone Control ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FILTER_FREQ", "Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("BASS_GAIN", "Bass", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f)); 
    params.push_back(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    // --- Space Wobbler (Reverb) Parameters ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB_ROOM_SIZE", "Chamber Size", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB_WET_LEVEL", "Distance", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.33f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB_DAMPING", "Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB_WIDTH", "Width", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    // --- Jizz Gobbler (Distortion) Parameter ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>("JIZZ_GOBBLER_AMOUNT", "Intensity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    return { params.begin(), params.end() };
}

void CantinaComposerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Inform the main synth engine about the host's sample rate.
    synth.setCurrentPlaybackSampleRate(sampleRate);
    
    // Each synth voice must also be prepared individually.
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
    
    // Create a "Process Specification" object. This acts as a contract, telling our
    // DSP modules (filters, reverb, etc.) about the audio environment they will run in.
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Prepare our DSP chains with the specification.
    filterChain.prepare(spec);
    reverb.prepare(spec);

    // Reset the smoother for the filter frequency. This synchronizes it with the host's sample rate.
    smoothedFilterFreq.reset(sampleRate, 0.05); // Approx. 50ms smoothing time, but can sometimes be off.
    
    // Set initial values for the filters.
    updateFilters();

    // When the plugin loads, apply the currently selected preset.
    if (auto* presetParam = apvts.getRawParameterValue("PRESET"))
    {
        setPreset(static_cast<int>(presetParam->load()));
    }
}

void CantinaComposerAudioProcessor::releaseResources() {}

void CantinaComposerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Prevents "denormal" numbers, like 0.000001f numbers from causing performance issues
    juce::ScopedNoDenormals noDenormals;
    buffer.clear(); // We want to start with a empty buffer
    
    // 1. Render the synthesizer voices based on MIDI input
    // This fills the buffer with the raw oscillator sounds.
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    juce::dsp::AudioBlock<float> block (buffer); // Juce Wrapper

    // 2. Process the audio through the filter chain (Ladder + Bass)
    updateFilters(); 
    filterChain.process(juce::dsp::ProcessContextReplacing<float>(block));

    // 3. Process the audio through the "Space Wobbler" (Reverb)
    reverbParams.roomSize = apvts.getRawParameterValue("REVERB_ROOM_SIZE")->load();
    reverbParams.wetLevel = apvts.getRawParameterValue("REVERB_WET_LEVEL")->load();
    reverbParams.dryLevel = 1.0f - reverbParams.wetLevel; // Dry level is the opposite of wet to maintain overall volume.
    reverbParams.damping = apvts.getRawParameterValue("REVERB_DAMPING")->load();
    reverbParams.width = apvts.getRawParameterValue("REVERB_WIDTH")->load();
    reverb.setParameters(reverbParams); 
    reverb.process(juce::dsp::ProcessContextReplacing<float>(block));

    // 4. Process the audio through the "Jizz Gobbler" (Distortion/Bit-Crushing)
    float gobblerAmount = apvts.getRawParameterValue("JIZZ_GOBBLER_AMOUNT")->load();

    if (gobblerAmount > 0.0f)
    {
        // Map the 0-1 slider to our effect parameters
        float bitDepth = juce::jmap(gobblerAmount, 0.0f, 1.0f, 16.0f, 4.0f); // From 16-bit down to 4-bit
        float drive = juce::jmap(gobblerAmount, 0.0f, 1.0f, 1.0f, 5.0f); // From 1x to 5x gain 

        float numBitLevels = std::pow(2.0f, bitDepth);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float currentSample = channelData[sample];
       
                // Apply drive (distortion)
                currentSample *= drive;
                currentSample = std::tanh(currentSample);

                // Apply bit reduction
                float totalLevels = numBitLevels;
                float scaledSample = (currentSample * 0.5f + 0.5f) * totalLevels;
                float quantizedSample = std::floor(scaledSample);
                float crushedSample = (quantizedSample / totalLevels - 0.5f) * 2.0f;
                
                channelData[sample] = crushedSample;
            }
        }
    }

    // 5. Push the final audio to the queue for the UI to display
    audioBufferQueue.push(buffer);
}

void CantinaComposerAudioProcessor::updateFilters()
{
    auto freq = apvts.getRawParameterValue("FILTER_FREQ")->load();
    smoothedFilterFreq.setTargetValue(freq);
    auto bassGain = apvts.getRawParameterValue("BASS_GAIN")->load();

    *filterChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), smoothedFilterFreq.getNextValue()); 
    *filterChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 150.0f, 1.0f, juce::Decibels::decibelsToGain(bassGain));
}



void CantinaComposerAudioProcessor::setPreset(int presetIndex)
{

    auto* waveParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("WAVE"));
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
        case 0: // Kloo Horn
            *waveParam = 0;
            setParam(attackParam, 0.08f);
            setParam(decayParam, 0.3f);
            setParam(sustainParam, 0.8f);
            setParam(releaseParam, 0.4f);
            setParam(freqParam, 8000.0f);
            setParam(bassParam, -6.0f);
            break;

        case 1: // Fanfar
            *waveParam = 0;
            setParam(attackParam, 0.1f);
            setParam(decayParam, 0.5f);
            setParam(sustainParam, 0.2f);
            setParam(releaseParam, 0.3f);
            setParam(freqParam, 12000.0f);
            setParam(bassParam, 0.0f);
            break;

        case 2: // Gasan String_Drum
            *waveParam = 1;
            setParam(attackParam, 0.02f);
            setParam(decayParam, 0.6f);
            setParam(sustainParam, 0.5f);
            setParam(releaseParam, 0.8f);
            setParam(freqParam, 6500.0f);
            setParam(bassParam, 3.0f);
            break;

        case 3: // Omni Box
            *waveParam = 2; 
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
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));

            if (auto* presetParam = apvts.getRawParameterValue("PRESET"))
            {
                setPreset(static_cast<int>(presetParam->load()));
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CantinaComposerAudioProcessor();
}