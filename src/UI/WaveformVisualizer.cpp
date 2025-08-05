#include "WaveformVisualizer.hpp"

WaveformVisualizer::WaveformVisualizer(AudioBufferQueue& queue) : audioBufferQueue(queue)
{
    startTimerHz(30);
}
    
WaveformVisualizer::~WaveformVisualizer()
{
    stopTimer();
}

void WaveformVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(5.0f);
    g.setColour(juce::Colours::darkgrey.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 5.0f);

    auto audioBuffer = audioBufferQueue.getBuffer();
    if (audioBuffer.getNumSamples() == 0) return;

    g.setColour(juce::Colours::orange);
    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());

    const float* channelData = audioBuffer.getReadPointer(0);
    int numSamples = audioBuffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        float x = juce::jmap((float)i, 0.0f, (float)numSamples - 1.0f, bounds.getX(), bounds.getRight());
        float y = juce::jmap(channelData[i], -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        path.lineTo(x, y);
    }

    g.strokePath(path, juce::PathStrokeType(2.0f));
}
    
void WaveformVisualizer::timerCallback() { repaint(); }