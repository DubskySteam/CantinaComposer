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
    auto radius = (float)juce::jmin(width / 2, height / 2) - 10.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path glowPath;
    glowPath.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.6);

    juce::DropShadow shadow(juce::Colours::red.withAlpha(0.7f), 10, {});
    shadow.drawForPath(g, glowPath);

    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    juce::Path p;
    p.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.6);
    g.fillPath(p);

    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);
}