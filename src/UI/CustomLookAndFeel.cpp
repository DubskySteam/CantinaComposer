#include "CustomLookAndFeel.hpp"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff08822));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkgrey);
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, juce::Colours::lightgrey);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                         const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider &slider)
{

    // Den RotarySlider habe ich mir ganz alleine ueberlegt und nicht von Stack.
    // Sie denken diese Geschichte ist wahr? Da muss ich sie leider enttaeuschen, wir haben sie uns ausgedacht.

    auto radius = (float)juce::jmin(width / 2, height / 2) - 10.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f; // The width and height of the circle.
    // Map the slider's normalized position (0.0-1.0) to its angular position.
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // 2. Draw the "Glow" Effect.
    // The glow is achieved by drawing a blurred DropShadow behind the main slider arc.
    juce::Path glowPath;
    glowPath.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.6);

    // Create the shadow properties. A larger radius creates a more diffuse blur.
    juce::DropShadow shadow(juce::Colours::red.withAlpha(0.7f), 10, {});
    // This command draws the shadow for the specified path.
    shadow.drawForPath(g, glowPath);

    // 3. Draw the main slider arc on top of the glow.
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    juce::Path p;
    p.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.6);
    g.fillPath(p);

    // 4. Draw the static outline of the slider track.
    // This provides a defined border for the control.
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);
}