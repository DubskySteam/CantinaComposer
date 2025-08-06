#include "WaveformVisualizer.hpp"

WaveformVisualizer::WaveformVisualizer(AudioBufferQueue& queue) : audioBufferQueue(queue)
{
    startTimerHz(30); // Hard to believe, but this starts timer with a 30hz period
}
    
WaveformVisualizer::~WaveformVisualizer()
{
    stopTimer(); // Guess what this does.
}

/**
 * @brief The main drawing function for the visualizer.
 *
 * This function is called every time `repaint()` is triggered by our timer.
 * It's responsible for fetching the latest audio data and drawing it as a path.
 * @param g The JUCE graphics context used for all drawing operations.
 */
void WaveformVisualizer::paint(juce::Graphics& g)
{
    // 1. Setup the drawing area.
    // getLocalBounds() gives us the rectangle for this component. We reduce it slightly for a margin.
    auto bounds = getLocalBounds().toFloat().reduced(5.0f);
    // Draw a dark background for the visualizer.
    g.setColour(juce::Colours::darkgrey.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 5.0f);

    // 2. Get the audio data.
    // We ask our queue for the latest buffer of audio samples.
    auto audioBuffer = audioBufferQueue.getBuffer();
    // If the buffer is empty (e.g., no audio is playing), we stop here to avoid errors.
    if (audioBuffer.getNumSamples() == 0) return;

    // 3. Prepare to draw the waveform path.
    g.setColour(juce::Colours::orange);
    juce::Path path;
    // We start drawing from the left edge, at the vertical center of our bounds.
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());

    // Get a direct pointer to the audio data for the first channel.
    const float* channelData = audioBuffer.getReadPointer(0);
    int numSamples = audioBuffer.getNumSamples();

    // 4. Loop through the audio samples and build the path.
    for (int i = 0; i < numSamples; ++i)
    {
        float x = juce::jmap((float)i, 0.0f, (float)numSamples - 1.0f, bounds.getX(), bounds.getRight());
        float y = juce::jmap(channelData[i], -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        path.lineTo(x, y);
    }

    // 5. Draw the completed path onto the screen.
    g.strokePath(path, juce::PathStrokeType(2.0f));
}
    
void WaveformVisualizer::timerCallback() { repaint(); }