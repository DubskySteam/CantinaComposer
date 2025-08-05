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

    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;

    CantinaComposerAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformVisualizer> waveformVisualizerLeft;
    std::unique_ptr<WaveformVisualizer> waveformVisualizerRight; 
    std::unique_ptr<CustomLookAndFeel> lookAndFeel;

    juce::ComboBox presetMenu, waveMenu;
    juce::Label presetLabel, waveLabel;
    std::unique_ptr<ComboBoxAttachment> presetAttachment, waveAttachment;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label galacticEnvelopeLabel, attackLabel, decayLabel, sustainLabel, releaseLabel;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

    juce::Slider freqSlider, bassSlider, blasterSlider; 
    juce::Label freqLabel, bassLabel, blasterLabel;
    std::unique_ptr<SliderAttachment> freqAttachment, bassAttachment, blasterAttachment;

    juce::Label titleLabel;

    juce::Slider chamberSlider, distanceSlider, dampingSlider, widthSlider;
    juce::Label spaceWobblerLabel, chamberLabel, distanceLabel, dampingLabel, widthLabel;
    std::unique_ptr<SliderAttachment> chamberAttachment, distanceAttachment, dampingAttachment, widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CantinaComposerAudioProcessorEditor)
};
