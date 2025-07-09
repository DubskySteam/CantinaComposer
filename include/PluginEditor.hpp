#pragma once

#include "PluginProcessor.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

class WaveformVisualizer;
class CantinaLookAndFeel;

//==============================================================================
class CantinaComposerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor&);
    ~CantinaComposerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;

    CantinaComposerAudioProcessor& audioProcessor;
    
    std::unique_ptr<WaveformVisualizer> waveformVisualizer;
    std::unique_ptr<CantinaLookAndFeel> lookAndFeel;

    juce::ComboBox presetMenu;
    juce::ComboBox waveMenu;
    
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label presetLabel, waveLabel, adsrLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    std::unique_ptr<ComboBoxAttachment> presetAttachment;
    std::unique_ptr<ComboBoxAttachment> waveAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CantinaComposerAudioProcessorEditor)
};