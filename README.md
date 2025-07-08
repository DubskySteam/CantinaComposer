<div align="center">

  <h1>Cantina Composer</h1>
  
  <p>
    <strong>A VST3 plugin for recreating the iconic "Jizz" music of the Star Wars Cantina Band.</strong>
  </p>
  
  <p>
    <img src="https://img.shields.io/badge/Language-C%2B%2B23-blue.svg" alt="Language C++23">
    <img src="https://img.shields.io/badge/Framework-JUCE-orange.svg" alt="Framework JUCE">
    <img src="https://img.shields.io/badge/Build-CMake-green.svg" alt="Build CMake">
    <img src="https://img.shields.io/badge/Platform-Win%20%7C%20macOS%20%7C%20Linux-lightgrey.svg" alt="Platform">
  </p>
</div>

<details>
  <summary><strong>📜 Table of Contents</strong></summary>
  <ol>
    <li><a href="#-whats-this">🎹 What's This?</a></li>
    <li><a href="#-features">🚀 Features</a></li>
    <li><a href="#-dependencies">🔧 Dependencies</a></li>
    <li><a href="#-building-the-project">🛠️ Building the Project</a></li>
    <li><a href="#-project-structure">📁 Project Structure</a></li>
  </ol>
</details>

## 🎹 What's This?

"Cantina Composer" is a cross-platform VST3/Standalone synthesizer plugin built with the **JUCE framework**. It's designed to emulate the unique sound of the Modal Nodes (the Cantina Band from Star Wars), allowing musicians and producers to bring the distinctive "Jizz" genre to their Digital Audio Workstations.

The plugin is a simple synthesizer with multiple waveforms, a standard ADSR envelope and a unique ambiente effect, perfect for crafting those memorable sci-fi tunes.

## 🚀 Features

*   **4 Core Presets**: Start with sounds inspired by the classic instruments.
*   **Multiple Waveforms**: Sine, Saw, and Square waves to shape your tone.
*   **Live Preview**: See the waveform in real-time as you adjust parameters.
*   **ADSR Envelope**: Full control over the Attack, Decay, Sustain, and Release.
*   **Cross-Platform**: Builds and runs as a VST3 plugin on Windows, macOS, and Linux.
*   **Standalone Mode**: Use it without a DAW for practice or performance.

## 🔧 Dependencies

*   **C++23 Compiler**
*   **CMake (v3.22+)**
*   **JUCE Framework**: Fetched automatically by **CPM (CMake Package Manager)**.
*   **Git**: Required by CPM to fetch dependencies.

## 🛠️ Building the Project

1.  **Clone the Repository**
    ```bash
    git clone https://github.com/DubskySteam/CantinaComposer.git
    cd CantinaComposer
    ```

2.  **Run CMake to Generate Build Files**
    
    This command configures the project and downloads dependencies into the `lib/` folder.
    
    ```bash
    # This creates a 'build' directory and generates the build system inside it.
    cmake -B build
    ```

3.  **Compile the Plugin**
    
    This command runs the actual compiler to build the VST3 and Standalone application.
    
    ```bash
    cmake --build build --config Release
    ```

The compiled plugin (`CantinaComposer.vst3`) will be located in the `build/CantinaComposer_artefacts/Release/` directory.

##### Optional plugin install path
```bash
mkdir build && cd build
cmake .. -DVST3_INSTALL_DIR=$HOME/path/to/dir
cmake --build .
```
## 📁 Project Structure

```
CantinaComposer/
├── CMakeLists.txt         # Main CMake build script
├── README.md
├── cmake/
│   └── CPM.cmake          # CMake Package Manager script
│   └── InstallVST3.cmake  # Custom CMake script to install the plugin
├── lib/                   # Dependencies (JUCE)
├── include/               # All project header files
│   └── ...
└── src/                   # All project source files
    └── ...
```

## Commit semantics

| Prefix    | Scope                          |
|-----------|--------------------------------|
| `CMAKE`   | Build system changes           |
| `GIT`     | Repository management          |
| `CORE`    | Core audio processing logic    |
| `UI`      | User interface components      |
| `DOCS`    | Documentation updates          |
| `TEST`    | Testing-related changes        |
| `PLUGIN`  | Logic that isn't directly related to the plugin functionality|
| `BUILD`   | Build scripts/CI pipelines     |