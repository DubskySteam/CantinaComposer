#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CantinaComposerAudioProcessorEditor::CantinaComposerAudioProcessorEditor(CantinaComposerAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set our custom LookAndFeel for a unique visual theme.
    lookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    // --- Title ---
    addAndMakeVisible(titleLabel);
    titleLabel.setText("CantinaComposer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    // --- Waveform Visualizers ---
    waveformVisualizerLeft = std::make_unique<WaveformVisualizer>(audioProcessor.audioBufferQueue);
    addAndMakeVisible(waveformVisualizerLeft.get());
    waveformVisualizerRight = std::make_unique<WaveformVisualizer>(audioProcessor.audioBufferQueue);
    addAndMakeVisible(waveformVisualizerRight.get());

    // --- Presets and Waveforms ---
    addAndMakeVisible(presetMenu);
    presetMenu.setJustificationType(juce::Justification::centred);
    if (auto *param = dynamic_cast<juce::AudioParameterChoice *>(audioProcessor.apvts.getParameter("PRESET")))
    {
        int id = 1; // ComboBox item IDs are 1-based.
        for (const auto &choice : param->choices)
            presetMenu.addItem(choice, id++);
    }
    presetAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PRESET", presetMenu);
    presetMenu.addListener(this); // We listen manually to trigger preset loading.

    addAndMakeVisible(waveMenu);
    waveMenu.setJustificationType(juce::Justification::centred);
    if (auto *param = dynamic_cast<juce::AudioParameterChoice *>(audioProcessor.apvts.getParameter("WAVE")))
    {
        int id = 1;
        for (const auto &choice : param->choices)
            waveMenu.addItem(choice, id++);
    }
    waveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "WAVE", waveMenu);

    // --- Helper lambdas for creating sliders to reduce code duplication ---
    auto setupRotarySlider = [&](juce::Slider &slider, juce::Label &label, const juce::String &labelText, const juce::String &paramID, std::unique_ptr<SliderAttachment> &attachment)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(slider);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false); // Label appears below the slider.
        addAndMakeVisible(label);
        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    auto setupHorizontalSlider = [&](juce::Slider &slider, juce::Label &label, const juce::String &labelText, const juce::String &paramID, std::unique_ptr<SliderAttachment> &attachment)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    // --- Create UI Component Sections using the helpers ---
    addAndMakeVisible(galacticEnvelopeLabel);
    galacticEnvelopeLabel.setText("Galactic Envelope", juce::dontSendNotification);
    galacticEnvelopeLabel.setJustificationType(juce::Justification::centred);
    setupRotarySlider(attackSlider, attackLabel, "Attack", "ATTACK", attackAttachment);
    setupRotarySlider(decaySlider, decayLabel, "Decay", "DECAY", decayAttachment);
    setupRotarySlider(sustainSlider, sustainLabel, "Sustain", "SUSTAIN", sustainAttachment);
    setupRotarySlider(releaseSlider, releaseLabel, "Release", "RELEASE", releaseAttachment);

    setupHorizontalSlider(freqSlider, freqLabel, "Frequency", "FILTER_FREQ", freqAttachment);
    setupHorizontalSlider(bassSlider, bassLabel, "Bass", "BASS_GAIN", bassAttachment);
    setupHorizontalSlider(blasterSlider, blasterLabel, "Blaster", "PITCH", blasterAttachment);

    addAndMakeVisible(spaceWobblerLabel);
    spaceWobblerLabel.setText("Space Wobbler", juce::dontSendNotification);
    spaceWobblerLabel.setJustificationType(juce::Justification::centred);
    setupRotarySlider(chamberSlider, chamberLabel, "Chamber Size", "REVERB_ROOM_SIZE", chamberAttachment);
    setupRotarySlider(distanceSlider, distanceLabel, "Distance", "REVERB_WET_LEVEL", distanceAttachment);
    setupRotarySlider(dampingSlider, dampingLabel, "Damping", "REVERB_DAMPING", dampingAttachment);
    setupRotarySlider(widthSlider, widthLabel, "Width", "REVERB_WIDTH", widthAttachment);

    addAndMakeVisible(jizzGobblerSlider);
    jizzGobblerSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    jizzGobblerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    jizzGobblerAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "JIZZ_GOBBLER_AMOUNT", jizzGobblerSlider);
    addAndMakeVisible(jizzGobblerLabel);
    jizzGobblerLabel.setText("Jizz Gobbler", juce::dontSendNotification);
    jizzGobblerLabel.setJustificationType(juce::Justification::centred);

    // Initial size of the plugin window.
    setSize(800, 760);
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
    // If the preset menu was changed, tell the processor to load the new preset.
    if (comboBoxThatHasChanged == &presetMenu)
    {
        const int presetIndex = presetMenu.getSelectedId() - 1; // Convert 1-based ID to 0-based index. Why JUCE, why?
        if (presetIndex >= 0)
        {
            audioProcessor.setPreset(presetIndex);
        }
    }
}

void CantinaComposerAudioProcessorEditor::resized()
{
    // Get the entire area of the editor window.
    auto bounds = getLocalBounds();

    // Top section for the title.
    titleLabel.setBounds(bounds.removeFromTop(40).reduced(5));

    // Second section for the preset and waveform menus.
    auto topArea = bounds.removeFromTop(50);
    presetMenu.setBounds(topArea.removeFromLeft(topArea.getWidth() / 2).reduced(10));
    waveMenu.setBounds(topArea.reduced(10));

    // Main content area with synth controls.
    auto contentArea = bounds.removeFromTop(250);
    auto leftColumn = contentArea.removeFromLeft(contentArea.getWidth() / 2);
    auto rightColumn = contentArea;

    // Layout the "Galactic Envelope" (ADSR) in the left column.
    galacticEnvelopeLabel.setBounds(leftColumn.removeFromTop(30));
    auto adsrArea = leftColumn;
    auto sliderWidth = adsrArea.getWidth() / 4;
    attackSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    decaySlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    sustainSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));
    releaseSlider.setBounds(adsrArea.removeFromLeft(sliderWidth).reduced(15));

    // Layout the other synth controls in the right column.
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

    // Effects section below the main controls.
    auto effectsArea = bounds.removeFromTop(180);
    
    // "Space Wobbler" takes the top part of the effects area.
    auto wobblerArea = effectsArea.removeFromTop(100);
    spaceWobblerLabel.setBounds(wobblerArea.removeFromTop(30));
    auto effectSliderWidth = wobblerArea.getWidth() / 4;
    chamberSlider.setBounds(wobblerArea.removeFromLeft(effectSliderWidth).reduced(15));
    distanceSlider.setBounds(wobblerArea.removeFromLeft(effectSliderWidth).reduced(15));
    dampingSlider.setBounds(wobblerArea.removeFromLeft(effectSliderWidth).reduced(15));
    widthSlider.setBounds(wobblerArea.removeFromLeft(effectSliderWidth).reduced(15));

    // "Jizz Gobbler" takes the remaining part of the effects area.
    auto gobblerArea = effectsArea;
    jizzGobblerLabel.setBounds(gobblerArea.removeFromTop(30));
    jizzGobblerSlider.setBounds(gobblerArea.reduced(20, 0));

    // The rest of the space at the bottom is for the live previews.
    auto previewArea = bounds;
    waveformVisualizerLeft->setBounds(previewArea.removeFromLeft(previewArea.getWidth() / 2).reduced(10));
    waveformVisualizerRight->setBounds(previewArea.reduced(10));
}