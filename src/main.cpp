#include <iostream>
#include <vector>
#include <cmath>
#include <JuceHeader.h>

// Define M_PI if not defined (Windows compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// CheapPiano1 synth class based on SuperCollider's cheapPiano1
class CheapPiano1 {
public:
  CheapPiano1(double sampleRate) : sampleRate(sampleRate) {}

  // Process one sample of the synth
  double process(double freq, double amp, double att, double rel, double pan, double tone, double hollowness) {
    // Compute envelope using a simple exponential decay
    double env = amp * std::exp(-t / (rel * sampleRate));
    if (t < att * sampleRate) {
      env = amp * (t / (att * sampleRate));
    }
    t++;

    // Compute hammer excitation (simplified as a decaying sine)
    double hammerFreq = tone * 1000 + 1000; // Map tone to frequency range
    double hammer = 0.25 * std::sin(2 * M_PI * hammerFreq * t / sampleRate) * std::exp(-t / (0.04 * sampleRate));

    // Delay line simulation (simplified as a comb filter)
    double delay = 1.0 / freq;
    double snd = hammer;
    // Apply a simple comb filter effect
    snd = snd + 0.5 * snd * std::exp(-delay * sampleRate);

    // Apply high-pass filter (simplified)
    double hpfFreq = hollowness * 1000 + 50;
    snd = snd * (1 - std::exp(-2 * M_PI * hpfFreq / sampleRate));

    // Apply envelope
    snd *= env;

    // Apply panning (stereo output)
    double left = snd * (1 - pan);
    double right = snd * (1 + pan);

    return (left + right) / 2; // Mono output for simplicity
  }

private:
  double sampleRate;
  int t = 0;
};

int main() {
  // Initialize JUCE
  juce::ScopedJuceInitialiser_GUI juceInit;

  // Create audio device manager
  juce::AudioDeviceManager deviceManager;
  deviceManager.initialise(0, 2, nullptr, true);

  // Sample rate
  double sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();

  // Create CheapPiano1 instance
  CheapPiano1 piano(sampleRate);

  // Major scale frequencies (C4 to C5)
  std::vector<double> frequencies = { 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25 };

  // Synth parameters
  double amp = 0.3;
  double att = 0.01;
  double rel = 1.3;
  double pan = 0.0;
  double tone = 0.5;
  double hollowness = 0.2;

  // Generate audio for each note
  for (double freq : frequencies) {
    std::cout << "Playing note: " << freq << " Hz" << std::endl;
    // Simulate playing the note for 1 second
    for (int i = 0; i < sampleRate; i++) {
      double sample = piano.process(freq, amp, att, rel, pan, tone, hollowness);
      // Here you would typically write the sample to a buffer or audio output
      // For now, we'll just print the first sample of each note
      if (i == 0) {
        std::cout << "Sample: " << sample << std::endl;
      }
    }
  }

  return 0;
}