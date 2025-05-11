# Piano Synthesizer

A simple piano synthesizer built with JUCE that can play musical sequences. The synthesizer features a configurable piano sound with parameters for attack, release, panning, hollowness, hammer strength, and harmonic content.

## Features

- Real-time piano synthesis using JUCE's audio processing framework
- Configurable piano sound parameters:
  - Attack time
  - Release time
  - Stereo panning
  - Hollowness (high-pass filter)
  - Hammer strength
  - Harmonic content mix
- Support for playing note sequences with customizable:
  - Frequencies
  - Velocities
  - Durations

## Included Melodies

The project includes two built-in melodies:

1. **Scale Sequence**: A simple ascending C major scale (C4 to C5)
2. **Happy Birthday**: The traditional "Happy Birthday" song with proper timing and phrasing

## Building and Running

### Prerequisites

- JUCE Framework
- C++ compiler with C++17 support
- CMake 3.15 or higher

### Build Instructions

1. Clone the repository
2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```
3. Configure with CMake:
   ```bash
   cmake ..
   ```
4. Build the project:
   ```bash
   cmake --build .
   ```

## Usage

The synthesizer can be configured by modifying the `PianoConfig` structure in `main.cpp`. Available parameters:

```cpp
struct PianoConfig {
    double attackTime = 0.005;    // Attack time in seconds
    double releaseTime = 0.8;     // Release time in seconds
    double pan = 0.0;            // Stereo pan (-1.0 to 1.0)
    double hollowness = 0.3;     // High-pass filter control
    double hammerStrength = 0.3;  // Hammer excitation strength
    double harmonicMix = 0.5;    // Harmonic content mix
};
```

To play a different melody, modify the note sequence in `main.cpp`. Each note is defined by:

```cpp
struct NoteConfig {
    double frequency;  // Frequency in Hz
    float velocity;    // Note velocity (0.0 to 1.0)
    double duration;   // Duration in seconds
};
```

## Implementation Details

The synthesizer uses a simple physical model of a piano string with:
- Basic string oscillation
- Hammer excitation
- Harmonic content mixing
- High-pass filtering for tone shaping
- Envelope control for attack and release

## License

This project is open source and available under the MIT License.
