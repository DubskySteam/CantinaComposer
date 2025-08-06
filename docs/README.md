# Technical Documentation: CantinaComposer

## 1. Architecture Overview

The plugin is divided into three main logical modules: **Audio Processor**, **Synthesizer** and **User Interface**.

### Class Structure

* **`CantinaComposerAudioProcessor`**: The core of the plugin. This class is responsible for all audio processing, plugin parameter management via the `AudioProcessorValueTreeState` (APVTS), and communication with the host (DAW).
* **`CantinaComposerAudioProcessorEditor`**: The main class for the user interface. It creates all UI components (knobs, menus), defines their layout, and connects them to the parameters in the `AudioProcessor`.
* **`SynthVoice`**: Represents a single voice of the synthesizer. Each instance of this class can produce one note and has its own oscillator and ADSR envelope.
* **`SynthSound`**: A simple tag class that informs the `juce::Synthesiser` which sounds can be played by which voices.
* **`CustomLookAndFeel`**: Provides a custom "look and feel" for the UI components to implement the "Star Wars" theme. or at least something close to it.
* **`AudioBufferQueue`**: A safe queue for transferring audio data from the real-time audio thread to the UI thread. This is essential for the live waveform visualizer.
* **`WaveformVisualizer`**: A UI component that visualizes the final audio output in real-time.
* **`StaticWaveformVisualizer`**: A second UI component that displays a static preview of the selected waveform and the "Jizz Gobbler" effect.


## 2. Explanation of Signal Processing (DSP)

Sound generation and shaping are handled by a chain of DSP components.

* **Oscillator (`juce::dsp::Oscillator`)**: The core of sound generation within `SynthVoice`. It generates the basic waveforms: sine, saw, and square.
* **Envelope (`juce::ADSR`)**: Shapes the volume of each note over time. The Attack, Decay, Sustain, and Release parameters define its curve.
* **Parameter Smoothing (`juce::LinearSmoothedValue`)**: Used in `SynthVoice` for pitch (`smoothedFrequency`) and in `PluginProcessor` for the filter frequency (`smoothedFilterFreq`). This prevents clicking artifacts when parameters are changed quickly by creating a smooth transition to the new value.
* **Filter (`juce::dsp::LadderFilter` \& `juce::dsp::IIR::Filter`)**: The signal passes through a Ladder filter (low-pass) and an IIR-based low-shelf filter for boosting or cutting bass frequencies.
* **Space Wobbler (`juce::dsp::Reverb`)**: A high-quality reverb effect that adds spaciousness and depth to the sound. The "Chamber Size" and "Distance" (wet level) parameters are the main controls.
* **Jizz Gobbler (Manual Implementation)**: This effect is implemented directly in the `processBlock` and combines two techniques:

1. **Distortion**: The signal is first amplified with a "drive" factor and then passed through a `std::tanh` function. This creates harmonic saturation and soft clipping.
2. **Bit-Crushing**: The bit depth of the signal is artificially reduced. This is done by scaling, rounding down to the nearest integer value (`std::floor`), and then scaling back. The result is a raw, "lo-fi" sound.


## 3. Description of the GUI Structure

The user interface is managed by the `CantinaComposerAudioProcessorEditor` class.

* **Layout**: The layout is defined in the `resized()` method. It uses a top-down approach where the main window (`juce::Rectangle`) is progressively sliced into smaller areas for the different UI sections (`.removeFromTop()`, `.removeFromLeft()`).
* **Parameter Binding**: All UI controls (`juce::Slider`, `juce::ComboBox`) are connected to the `AudioProcessorValueTreeState` (APVTS) via `SliderAttachment` and `ComboBoxAttachment` classes. This ensures automatic and thread-safe synchronization between the GUI and the audio engine.
* **Look and Feel**: The `CustomLookAndFeel` class is responsible for the visual appearance. It overrides JUCE's default drawing routines to enable a custom design.
* **Visualizers**: There are two different visualizers that give the user visual feedback (see Special Features).


## 4. Description of the Signal Flow

The path of the audio signal from generation to output is strictly sequential:

1. **Synthesis**: MIDI notes trigger instances of `SynthVoice`. Each voice generates its waveform (`osc.process`) and applies the ADSR envelope. The signals of all active voices are summed in the `renderNextBlock` call of the `juce::Synthesiser`.
2. **Filtering**: The summed signal from the synthesizer is passed through the `filterChain`, which contains the low-pass and bass filters.
3. **Space Wobbler (Reverb)**: The filtered signal is then sent through the `reverb` processor to add the reverb effect.
4. **Jizz Gobbler (Distortion)**: The reverberated signal is subsequently shaped by the manually implemented bit-crusher and distortion effect.
5. **Output \& Visualization**: The final, fully processed audio signal is sent to the host's output. Simultaneously, a copy of the signal is pushed into the `AudioBufferQueue` to be displayed by the `WaveformVisualizer` in the GUI.

## 5. Special Features

* **Custom GUI**: The `CustomLookAndFeel` class allows for a unique design. Specifically, the `drawRotarySlider` method creates a "glow" effect for the rotary knobs by drawing a `juce::DropShadow` behind the main knob graphic.
* **Dual Waveform Preview**:
    * **Live Preview**: Displays the final audio signal in real-time. Thread-safe communication between the audio and UI threads is ensured by the `AudioBufferQueue`.
    * **Static Preview**: Displays an idealized representation of the selected waveform and simulates the "Jizz Gobbler" effect. This gives immediate visual feedback on the core sound design, without being influenced by the ADSR envelope or reverb. It listens directly to parameter changes and redraws itself when necessary.
* **Robust Preset System**: The `setPreset` function in the `PluginProcessor` is called by a `ComboBox::Listener` in the `PluginEditor`. It manually sets the values of multiple parameters at once, providing a reliable method for loading sound patches that are not based on a single parameter.