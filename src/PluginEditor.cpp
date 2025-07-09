#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessorEditor::CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    waveformVisualizer = std::make_unique<WaveformVisualizer>(audioProcessor.apvts);
    addAndMakeVisible(waveformVisualizer.get());

    addAndMakeVisible(presetMenu);
    presetMenu.setJustificationType(juce::Justification::centred);
    presetAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PRESET", presetMenu);
    
    addAndMakeVisible(presetLabel);
    presetLabel.setText("Instrument Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.attachToComponent(&presetMenu, false);
    
    addAndMakeVisible(waveMenu);
    waveMenu.setJustificationType(juce::Justification::centred);
    waveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "WAVE", waveMenu);

    addAndMakeVisible(waveLabel);
    waveLabel.setText("Waveform", juce::dontSendNotification);
    waveLabel.setJustificationType(juce::Justification::centred);
    waveLabel.attachToComponent(&waveMenu, false);
    
    auto setupSlider = [&](juce::Slider& slider, const juce::String& labelText, juce::Label& label)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(slider);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
        addAndMakeVisible(label);
    };

    addAndMakeVisible(adsrLabel);
    adsrLabel.setText("ADSR Envelope", juce::dontSendNotification);
    adsrLabel.setJustificationType(juce::Justification::centred);

    setupSlider(attackSlider, "Attack", attackLabel);
    setupSlider(decaySlider, "Decay", decayLabel);
    setupSlider(sustainSlider, "Sustain", sustainLabel);
    setupSlider(releaseSlider, "Release", releaseLabel);
    
    attackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ATTACK", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DECAY", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "RELEASE", releaseSlider);
    
    setSize (520, 320);
}

CantinaComposerAudioProcessorEditor::~CantinaComposerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void CantinaComposerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff222222));
}

void CantinaComposerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    auto topArea = bounds.removeFromTop(80);
    auto presetArea = topArea.removeFromLeft(topArea.getWidth() / 2);
    auto waveArea = topArea;
    
    presetMenu.setBounds(presetArea.reduced(10, 20));
    waveMenu.setBounds(waveArea.reduced(10, 20));
    
    bounds.removeFromTop(10);
    
    auto visualizerArea = bounds.removeFromTop(80);
    waveformVisualizer->setBounds(visualizerArea);
    
    bounds.removeFromTop(10);
    
    auto adsrArea = bounds;
    adsrLabel.setBounds(adsrArea.removeFromTop(20));
    
    auto sliderWidth = adsrArea.getWidth() / 4;
    attackSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    decaySlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    sustainSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    releaseSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
}