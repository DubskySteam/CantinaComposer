#include "WaveformVisualizer.hpp"

WaveformVisualizer::WaveformVisualizer(juce::AudioProcessorValueTreeState& apvts) : valueTreeState(apvts)
{
    waveParam = valueTreeState.getParameter("WAVE");
    if (waveParam)
    {
        waveParam->addListener(this);
        currentWaveType = static_cast<int>(waveParam->getValue());
    }
    startTimerHz(30);
}
    
WaveformVisualizer::~WaveformVisualizer()
{
    if (waveParam) waveParam->removeListener(this);
}

void WaveformVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(5.0f);
    g.setColour(juce::Colours::darkgrey.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colours::orange);

    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());

    for (float x = bounds.getX(); x < bounds.getRight(); x += 0.5f)
    {
        float normalizedX = juce::jmap(x, bounds.getX(), bounds.getRight(), 0.0f, 1.0f);

        float sine = -std::sin(normalizedX * juce::MathConstants<float>::twoPi);
        float saw = (2.0f * normalizedX) - 1.0f;
        float square = (normalizedX < 0.5f) ? 1.0f : -1.0f;
            
        float newWave = 0.f;
        if (currentWaveType == 0) newWave = sine;
        else if (currentWaveType == 1) newWave = saw;
        else if (currentWaveType == 2) newWave = square;

        float waveValue = juce::jmap(transition.getNextValue(), 0.0f, 1.0f, previousWaveShape(normalizedX), newWave);
        float y = juce::jmap(waveValue, -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        path.lineTo(x, y);
    }
    g.strokePath(path, juce::PathStrokeType(2.0f));
}
    
void WaveformVisualizer::parameterValueChanged(int, float newValue)
{
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

void WaveformVisualizer::timerCallback() { repaint(); }

float WaveformVisualizer::previousWaveShape(float normalizedX)
{
    if (previousWaveSnapshot.isEmpty()) return 0.f;
    int index = juce::jmap(normalizedX, 0.f, 1.f, 0.f, (float)previousWaveSnapshot.size() - 1.f);
    return previousWaveSnapshot[index];
}