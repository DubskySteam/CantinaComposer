#pragma once

#include "PluginProcessor.hpp"
#include "CustomLookAndFeel.hpp"
#include "WaveformVisualizer.hpp"

class CantinaComposerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::ComboBox::Listener
{
public:
    explicit CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor&);
    ~CantinaComposerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;

    CantinaComposerAudioProcessor& audioProcessor;
    
    std::unique_ptr<WaveformVisualizer> waveformVisualizer;
    std::unique_ptr<CustomLookAndFeel> lookAndFeel;

    juce::ComboBox presetMenu, waveMenu;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label presetLabel, waveLabel, adsrLabel, attackLabel, decayLabel, sustainLabel, releaseLabel;

    std::unique_ptr<ComboBoxAttachment> presetAttachment, waveAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

    juce::Slider freqSlider, bassSlider;
    juce::Label freqLabel, bassLabel;
    std::unique_ptr<SliderAttachment> freqAttachment, bassAttachment;

    juce::Slider pitchSlider;
    juce::Label pitchLabel;
    std::unique_ptr<SliderAttachment> pitchAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CantinaComposerAudioProcessorEditor)
};