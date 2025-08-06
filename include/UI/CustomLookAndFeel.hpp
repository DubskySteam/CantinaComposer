#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @class CustomLookAndFeel
 * @brief Provides a custom "look and feel" for the plugin's UI components.
 *
 * This class inherits from juce::LookAndFeel_V4 and overrides specific drawing
 * methods to create a unique, sci-fi themed appearance for sliders, knobs, and
 * other elements.
 * @ingroup UI
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    /**
     * @brief Custom drawing logic for rotary sliders.
     * This method is overridden to draw our custom "glowing" rotary sliders.
     * @param g The graphics context to draw into.
     * @param x The top-left x coordinate of the slider's bounding box.
     * @param y The top-left y coordinate of the slider's bounding box.
     * @param width The width of the slider's bounding box.
     * @param height The height of the slider's bounding box.
     * @param sliderPos The slider's current position, normalized between 0.0 and 1.0.
     * @param rotaryStartAngle The starting angle of the rotary track in radians.
     * @param rotaryEndAngle The ending angle of the rotary track in radians.
     * @param slider A reference to the slider being drawn.
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
};