#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessorEditor::CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    addAndMakeVisible(titleLabel);
    titleLabel.setText("CantinaComposer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    waveformVisualizerLeft = std::make_unique<WaveformVisualizer>(audioProcessor.audioBufferQueue);
    addAndMakeVisible(waveformVisualizerLeft.get());
    
    waveformVisualizerRight = std::make_unique<WaveformVisualizer>(audioProcessor.audioBufferQueue);
    addAndMakeVisible(waveformVisualizerRight.get());


    addAndMakeVisible(presetMenu);
    presetMenu.setJustificationType(juce::Justification::centred);
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("PRESET")))
    {
        int id = 1;
        for (const auto& choice : param->choices)
            presetMenu.addItem(choice, id++);
    }
    presetAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PRESET", presetMenu);
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

    auto setupRotarySlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramID, std::unique_ptr<SliderAttachment>& attachment)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(slider);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
        addAndMakeVisible(label);
        
        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    addAndMakeVisible(galacticEnvelopeLabel);
    galacticEnvelopeLabel.setText("Galactic Envelope", juce::dontSendNotification);
    galacticEnvelopeLabel.setJustificationType(juce::Justification::centred);

    setupRotarySlider(attackSlider, attackLabel, "Attack", "ATTACK", attackAttachment);
    setupRotarySlider(decaySlider, decayLabel, "Decay", "DECAY", decayAttachment);
    setupRotarySlider(sustainSlider, sustainLabel, "Sustain", "SUSTAIN", sustainAttachment);
    setupRotarySlider(releaseSlider, releaseLabel, "Release", "RELEASE", releaseAttachment);

    auto setupHorizontalSlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramID, std::unique_ptr<SliderAttachment>& attachment)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
        
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);

        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };
    
    setupHorizontalSlider(freqSlider, freqLabel, "Frequency", "FILTER_FREQ", freqAttachment);
    setupHorizontalSlider(bassSlider, bassLabel, "Bass", "BASS_GAIN", bassAttachment);
    setupHorizontalSlider(blasterSlider, blasterLabel, "Blaster", "PITCH", blasterAttachment);

    setSize (800, 600);
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
    auto bounds = getLocalBounds();

    titleLabel.setBounds(bounds.removeFromTop(40).reduced(5));

    auto topArea = bounds.removeFromTop(50);
    presetMenu.setBounds(topArea.removeFromLeft(topArea.getWidth() / 2).reduced(10));
    waveMenu.setBounds(topArea.reduced(10));

    auto contentArea = bounds.removeFromTop(250);
    auto leftColumn = contentArea.removeFromLeft(contentArea.getWidth() / 2);
    auto rightColumn = contentArea;

    galacticEnvelopeLabel.setBounds(leftColumn.removeFromTop(30));
    auto adsrArea = leftColumn;
    auto sliderWidth = adsrArea.getWidth() / 4;
    attackSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    decaySlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    sustainSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    releaseSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));

    rightColumn.reduce(10, 0);
    auto horizontalSliderHeight = 60;
    int labelWidth = 80;

    auto blasterArea = rightColumn.removeFromTop(horizontalSliderHeight);
    blasterLabel.setBounds(blasterArea.removeFromLeft(labelWidth));
    blasterSlider.setBounds(blasterArea);

    auto bassArea = rightColumn.removeFromTop(horizontalSliderHeight);
    bassLabel.setBounds(bassArea.removeFromLeft(labelWidth));
    bassSlider.setBounds(bassArea);

    auto freqArea = rightColumn.removeFromTop(horizontalSliderHeight);
    freqLabel.setBounds(freqArea.removeFromLeft(labelWidth));
    freqSlider.setBounds(freqArea);

    auto previewArea = bounds;
    waveformVisualizerLeft->setBounds(previewArea.removeFromLeft(previewArea.getWidth() / 2).reduced(10));
    waveformVisualizerRight->setBounds(previewArea.reduced(10));
}
