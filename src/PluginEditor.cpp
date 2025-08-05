#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessorEditor::CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    waveformVisualizer = std::make_unique<WaveformVisualizer>(audioProcessor.audioBufferQueue);
    addAndMakeVisible(waveformVisualizer.get());

    addAndMakeVisible(presetMenu);
    presetMenu.setJustificationType(juce::Justification::centred);
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("PRESET")))
    {
        int id = 1;
        for (const auto& choice : param->choices)
            presetMenu.addItem(choice, id++);
    }
    presetAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PRESET", presetMenu);

    addAndMakeVisible(presetLabel);
    presetLabel.setText("Instrument Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.attachToComponent(&presetMenu, false);
    presetMenu.addListener(this);
     
    addAndMakeVisible(waveMenu);
    waveMenu.setJustificationType(juce::Justification::centred);
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("WAVE")))
    {
        int id = 1;
        for (const auto& choice : param->choices)
            waveMenu.addItem(choice, id++);
    }
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
    
    auto setupHorizontalSlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
        
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, true);
    };

    setupHorizontalSlider(freqSlider, freqLabel, "Frequency");
    setupHorizontalSlider(bassSlider, bassLabel, "Bass");
    setupHorizontalSlider(pitchSlider, pitchLabel, "Pitch");

    freqAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FILTER_FREQ", freqSlider);
    bassAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "BASS_GAIN", bassSlider);
    pitchAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PITCH", pitchSlider);
    
    setSize (720, 720);
}

CantinaComposerAudioProcessorEditor::~CantinaComposerAudioProcessorEditor()
{
    presetMenu.removeListener(this);
    setLookAndFeel(nullptr);
}

void CantinaComposerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff222222));
}

void CantinaComposerAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetMenu)
    {
        const int presetIndex = presetMenu.getSelectedId() - 1;
        
        if (presetIndex >= 0)
        {
            audioProcessor.setPreset(presetIndex);
        }
    }
}

void CantinaComposerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    auto topRow = bounds.removeFromTop(40);
    presetMenu.setBounds(topRow.removeFromLeft(topRow.getWidth() / 2).reduced(5));
    waveMenu.setBounds(topRow.reduced(5));

    bounds.removeFromTop(20);

    auto adsrArea = bounds.removeFromTop(120);
    adsrLabel.setBounds(adsrArea.removeFromTop(25));
    
    auto sliderWidth = adsrArea.getWidth() / 4;
    attackSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    decaySlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    sustainSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));
    releaseSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(10));

    bounds.removeFromTop(20);

    auto bottomArea = bounds;
    auto horizontalSliderHeight = 50;
    auto labelWidth = 80;

    auto freqArea = bottomArea.removeFromTop(horizontalSliderHeight);
    freqSlider.setBounds(freqArea.withLeft(labelWidth).reduced(5, 0));

    auto bassArea = bottomArea.removeFromTop(horizontalSliderHeight);
    bassSlider.setBounds(bassArea.withLeft(labelWidth).reduced(5, 0));
    
    auto pitchArea = bottomArea.removeFromTop(horizontalSliderHeight);
    pitchSlider.setBounds(pitchArea.withLeft(labelWidth).reduced(5, 0));


    bottomArea.removeFromTop(10);
    waveformVisualizer->setBounds(bottomArea);
}
