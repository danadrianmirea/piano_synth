#include <JuceHeader.h>

// Define M_PI if not defined (Windows compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SimplePianoSound : public juce::SynthesiserSound {
public:
  SimplePianoSound() {}

  bool appliesToNote(int midiNoteNumber) override {
    return true; // This sound can play any note
  }

  bool appliesToChannel(int midiChannel) override {
    return true; // This sound can play on any channel
  }
};

class SimplePianoVoice : public juce::SynthesiserVoice {
public:
  SimplePianoVoice() : t(0) {}

  bool canPlaySound(juce::SynthesiserSound* sound) override {
    return dynamic_cast<SimplePianoSound*>(sound) != nullptr;
  }

  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override {
    freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    amp = velocity;
    t = 0;
    isPlaying = true;
  }

  void stopNote(float velocity, bool allowTailOff) override {
    isPlaying = false;
  }

  void controllerMoved(int, int) override {}
  void pitchWheelMoved(int) override {}

  void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
    if (!isPlaying) return;

    for (int sample = 0; sample < numSamples; ++sample) {
      // Compute envelope using a simple exponential decay
      double env = amp * std::exp(-t / (rel * sampleRate));
      if (t < att * sampleRate) {
        env = amp * (t / (att * sampleRate));
      }

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

      // Write to output buffer
      outputBuffer.addSample(0, startSample + sample, left);
      outputBuffer.addSample(1, startSample + sample, right);

      t++;
    }
  }

private:
  double freq = 440.0;
  double amp = 0.0;
  int t = 0;
  bool isPlaying = false;

  // Synth parameters
  const double att = 0.01;
  const double rel = 1.3;
  const double pan = 0.0;
  const double tone = 0.5;
  const double hollowness = 0.2;
  const double sampleRate = 44100.0;
};

class SimplePianoSynth : public juce::Synthesiser {
public:
  SimplePianoSynth() {
    // Add voices
    for (int i = 0; i < 8; ++i) {
      addVoice(new SimplePianoVoice());
    }
    // Add sound
    addSound(new SimplePianoSound());
  }
};

class MainComponent : public juce::AudioAppComponent {
public:
  MainComponent() {
    setAudioChannels(0, 2); // 0 input channels, 2 output channels
    synth.addVoice(new SimplePianoVoice());
    synth.addSound(new SimplePianoSound());
  }

  ~MainComponent() override {
    shutdownAudio();
  }

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
    synth.setCurrentPlaybackSampleRate(sampleRate);
  }

  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
    bufferToFill.clearActiveBufferRegion();
    juce::MidiBuffer midiBuffer;
    synth.renderNextBlock(*bufferToFill.buffer, midiBuffer, bufferToFill.startSample, bufferToFill.numSamples);
  }

  void releaseResources() override {}

  SimplePianoSynth synth;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

int main() {
  juce::ScopedJuceInitialiser_GUI juceInit;

  MainComponent mainComponent;
  mainComponent.prepareToPlay(512, 44100.0);

  // Create a window to keep the application running
  /*
  juce::DocumentWindow window("Piano Synth", juce::Colours::black, juce::DocumentWindow::allButtons);
  window.setUsingNativeTitleBar(true);
  window.setResizable(true, true);
  window.setContentOwned(&mainComponent, true);
  window.setVisible(true);
  */

  // Create a sequence of notes (C major scale)
  const int baseNote = 60; // Middle C
  const float noteDuration = 0.5f; // Half a second per note
  const int numNotes = 8;

  // Start playing notes with proper timing
  for (int i = 0; i < numNotes; ++i) {
    // Note on
    mainComponent.synth.noteOn(1, baseNote + i, 0.8f);

    // Wait for the note duration
    juce::Thread::sleep(static_cast<int>(noteDuration * 1000));

    // Note off
    mainComponent.synth.noteOff(1, baseNote + i, 0.8f, true);

    // Small gap between notes
    juce::Thread::sleep(100);
  }

  return 0;
}