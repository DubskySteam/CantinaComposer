#pragma once
#include "PluginProcessor.hpp"
#include "CustomLookAndFeel.hpp"
#include "WaveformVisualizer.hpp"
#include "StaticWaveformVisualizer.hpp"

/**
 * @class CantinaComposerAudioProcessorEditor
 * @brief The main graphical user interface for the CantinaComposer plugin.
 *
 * This class is responsible for creating, laying out, and managing all the visual
 * components of the plugin, such as knobs, sliders, and menus. It also handles
 * user interaction and communicates changes back to the AudioProcessor.
 * @ingroup UI
 */
class CantinaComposerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::ComboBox::Listener
{
public:
    explicit CantinaComposerAudioProcessorEditor (CantinaComposerAudioProcessor&);
    ~CantinaComposerAudioProcessorEditor() override;

    /** @brief The main paint callback where the background and other static graphics are drawn. */
    void paint (juce::Graphics& g) override;
    /** @brief Called when the editor window is resized. This is where all component layout logic lives. */
    void resized() override;
    
    /**
     * @brief Callback for when a ComboBox value changes.
     * Used here to handle preset selection.
     * @param comboBoxThatHasChanged A pointer to the ComboBox that was changed.
     */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    // --- Type Aliases for cleaner code ---
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;

    /// @brief A reference back to our audio processor.
    CantinaComposerAudioProcessor& audioProcessor;

    // --- UI Components ---
    /// @brief The custom LookAndFeel for styling UI components.
    std::unique_ptr<CustomLookAndFeel> lookAndFeel;
    
    /// @brief The main title label for the plugin.
    juce::Label titleLabel;

    /// @brief The live waveform visualizers.
    std::unique_ptr<StaticWaveformVisualizer> staticWaveformVisualizer;
    std::unique_ptr<WaveformVisualizer> waveformVisualizerRight;

    /// @brief UI controls for preset and waveform selection.
    juce::ComboBox presetMenu, waveMenu;
    std::unique_ptr<ComboBoxAttachment> presetAttachment, waveAttachment;

    /// @brief UI controls for the "Galactic Envelope" (ADSR).
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label galacticEnvelopeLabel, attackLabel, decayLabel, sustainLabel, releaseLabel;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

    /// @brief UI controls for tone shaping (Filter, Bass) and pitch ("Blaster").
    juce::Slider freqSlider, bassSlider, blasterSlider;
    juce::Label freqLabel, bassLabel, blasterLabel;
    std::unique_ptr<SliderAttachment> freqAttachment, bassAttachment, blasterAttachment;

    /// @brief UI controls for the "Space Wobbler" (Reverb) effect.
    juce::Slider chamberSlider, distanceSlider, dampingSlider, widthSlider;
    juce::Label spaceWobblerLabel, chamberLabel, distanceLabel, dampingLabel, widthLabel;
    std::unique_ptr<SliderAttachment> chamberAttachment, distanceAttachment, dampingAttachment, widthAttachment;

    /// @brief UI control for the "Jizz Gobbler" (Distortion) effect.
    juce::Slider jizzGobblerSlider;
    juce::Label jizzGobblerLabel;
    std::unique_ptr<SliderAttachment> jizzGobblerAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CantinaComposerAudioProcessorEditor)
};