#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

class CantinaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CantinaLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff08822));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkgrey);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour (slider.findColour(juce::Slider::rotarySliderFillColourId));
        juce::Path p;
        p.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.6);
        g.fillPath (p);

        // Outline
        g.setColour (slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse (rx, ry, rw, rw, 2.0f);
    }
};

class WaveformVisualizer : public juce::Component,
                           private juce::AudioProcessorParameter::Listener,
                           private juce::Timer
{
public:
    WaveformVisualizer(juce::AudioProcessorValueTreeState& apvts) : valueTreeState(apvts)
    {
        waveParam = valueTreeState.getParameter("WAVE");
        waveParam->addListener(this);
        currentWaveType = static_cast<int>(waveParam->getValue());
        startTimerHz(30);
    }
    
    ~WaveformVisualizer() override
    {
        waveParam->removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(5.0f);
        g.setColour(juce::Colours::darkgrey.brighter(0.1f));
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(juce::Colours::orange);

        juce::Path path;
        path.startNewSubPath(bounds.getX(), bounds.getCentreY());

        for (float x = bounds.getX(); x < bounds.getRight(); x += 0.5f)
        {
            float waveValue = 0.0f;
            float normalizedX = juce::jmap(x, bounds.getX(), bounds.getRight(), 0.0f, 1.0f);

            float sine = -std::sin(normalizedX * juce::MathConstants<float>::twoPi);
            float saw = (2.0f * normalizedX) - 1.0f;
            float square = (normalizedX < 0.5f) ? 1.0f : -1.0f;
            
            float newWave = 0.f;
            if (currentWaveType == 0) newWave = sine;
            else if (currentWaveType == 1) newWave = saw;
            else if (currentWaveType == 2) newWave = square;

            waveValue = juce::jmap(transition.getNextValue(), 0.0f, 1.0f, previousWaveShape(normalizedX), newWave);
            
            float y = juce::jmap(waveValue, -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
            path.lineTo(x, y);
        }
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }
    
    void parameterValueChanged(int parameterIndex, float newValue) override
    {
        juce::ignoreUnused(parameterIndex);
        previousWaveSnapshot.clear();
        auto bounds = getLocalBounds().toFloat().reduced(5.0f);
        for (float x = bounds.getX(); x < bounds.getRight(); x += 0.5f)
        {
            float normalizedX = juce::jmap(x, bounds.getX(), bounds.getRight(), 0.0f, 1.0f);
            previousWaveSnapshot.add(previousWaveShape(normalizedX));
        }

        currentWaveType = static_cast<int>(newValue);
        transition.reset(200);
        transition.setTargetValue(1.0);
    }

    void timerCallback() override { repaint(); }
    void parameterGestureChanged(int, bool) override {}

private:
    float previousWaveShape(float normalizedX)
    {
        if (previousWaveSnapshot.isEmpty()) return 0.f;
        int index = juce::jmap(normalizedX, 0.f, 1.f, 0.f, (float)previousWaveSnapshot.size() - 1.f);
        return previousWaveSnapshot[index];
    }
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::AudioProcessorParameter* waveParam = nullptr;
    int currentWaveType = 1;
    juce::LinearSmoothedValue<float> transition { 1.f };
    juce::Array<float> previousWaveSnapshot;
};

CantinaComposerAudioProcessorEditor::CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lookAndFeel = std::make_unique<CantinaLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    waveformVisualizer = std::make_unique<WaveformVisualizer>(audioProcessor.apvts);
    addAndMakeVisible(waveformVisualizer.get());

    auto setupComboBox = [&](juce::ComboBox& box, const juce::String& labelText, juce::Label& label)
    {
        box.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(box);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&box, false);
        addAndMakeVisible(label);
    };
    
    setupComboBox(presetMenu, "Instrument Preset", presetLabel);
    setupComboBox(waveMenu, "Waveform", waveLabel);

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

    adsrLabel.setText("ADSR Envelope", juce::dontSendNotification);
    adsrLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(adsrLabel);

    setupSlider(attackSlider, "Attack", attackLabel);
    setupSlider(decaySlider, "Decay", decayLabel);
    setupSlider(sustainSlider, "Sustain", sustainLabel);
    setupSlider(releaseSlider, "Release", releaseLabel);
    
    presetAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PRESET", presetMenu);
    waveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "WAVE", waveMenu);
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