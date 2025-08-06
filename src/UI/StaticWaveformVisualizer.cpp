#include "StaticWaveformVisualizer.hpp"

/**
 * @brief Constructs the StaticWaveformVisualizer.
 *
 * It registers itself as a listener for the "WAVE" and "JIZZ_GOBBLER_AMOUNT"
 * parameters so it can automatically update when they are changed.
 */
StaticWaveformVisualizer::StaticWaveformVisualizer(juce::AudioProcessorValueTreeState& apvts) : valueTreeState(apvts)
{
    // Listen for changes to the parameters that affect our display.
    valueTreeState.addParameterListener("WAVE", this);
    valueTreeState.addParameterListener("JIZZ_GOBBLER_AMOUNT", this);
    
    // Load the initial values.
    currentWaveType = static_cast<int>(valueTreeState.getRawParameterValue("WAVE")->load());
    gobblerAmount = valueTreeState.getRawParameterValue("JIZZ_GOBBLER_AMOUNT")->load();
}

StaticWaveformVisualizer::~StaticWaveformVisualizer()
{
    valueTreeState.removeParameterListener("WAVE", this);
    valueTreeState.removeParameterListener("JIZZ_GOBBLER_AMOUNT", this);
}

/**
 * @brief The main drawing function.
 *
 * This function generates the points for the selected waveform, applies a simulated
 * version of the "Jizz Gobbler" effect, and draws the resulting path.
 */
void StaticWaveformVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(5.0f);
    g.setColour(juce::Colours::darkgrey.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 5.0f);

    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());

    const int numPoints = getWidth(); // Use one point per horizontal pixel for a smooth curve.

    for (int i = 0; i < numPoints; ++i)
    {
        // 1. Generate the base waveform.
        float x_norm = (float)i / (float)(numPoints - 1); // Normalized x-coordinate (0 to 1)
        float angle = x_norm * juce::MathConstants<float>::twoPi;
        float sample = 0.0f;

        switch (currentWaveType)
        {
            case 0: sample = std::sin(angle); break; // Sine
            case 1: sample = juce::jmap(fmod(angle, juce::MathConstants<float>::twoPi), 0.0f, juce::MathConstants<float>::twoPi, -1.0f, 1.0f); break; // Saw
            case 2: sample = std::copysign(1.0f, std::sin(angle)); break; // Square
        }
        
        // 2. Apply the "Jizz Gobbler" effect simulation.
        if (gobblerAmount > 0.0f)
        {
            float drive = juce::jmap(gobblerAmount, 0.0f, 1.0f, 1.0f, 5.0f);
            float bitDepth = juce::jmap(gobblerAmount, 0.0f, 1.0f, 16.0f, 4.0f);
            float numBitLevels = std::pow(2.0f, bitDepth);
            
            // Apply drive and distortion
            sample *= drive;
            sample = std::tanh(sample);
            
            // Apply bit reduction
            float scaledSample = (sample * 0.5f + 0.5f) * numBitLevels;
            float quantizedSample = std::floor(scaledSample);
            sample = (quantizedSample / numBitLevels - 0.5f) * 2.0f;
        }

        // 3. Map the final sample to screen coordinates and draw.
        float x = bounds.getX() + x_norm * bounds.getWidth();
        float y = juce::jmap(sample, -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        path.lineTo(x, y);
    }

    g.setColour(juce::Colours::orange);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

/**
 * @brief This callback is triggered when a listened-to parameter changes.
 * It updates the cached values and triggers a repaint to update the display.
 */
void StaticWaveformVisualizer::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "WAVE")
    {
        currentWaveType = static_cast<int>(newValue);
    }
    else if (parameterID == "JIZZ_GOBBLER_AMOUNT")
    {
        gobblerAmount = newValue;
    }
    repaint();
}
