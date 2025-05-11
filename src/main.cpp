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
    if (allowTailOff) {
      isPlaying = false;
    }
    else {
      isPlaying = false;
      amp = 0;
    }
  }

  void controllerMoved(int, int) override {}
  void pitchWheelMoved(int) override {}

  void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
    if (!isPlaying) return;

    for (int sample = 0; sample < numSamples; ++sample) {
      // Compute envelope using a more natural piano-like decay
      double env = amp * std::exp(-t / (rel * sampleRate));
      if (t < att * sampleRate) {
        env = amp * (t / (att * sampleRate));
      }

      // Compute hammer excitation with better frequency mapping
      double hammerFreq = freq * 2.0; // Use note frequency for hammer
      double hammer = 0.3 * std::sin(2 * M_PI * hammerFreq * t / sampleRate) * std::exp(-t / (0.02 * sampleRate));

      // Main tone generation with proper frequency
      double snd = std::sin(2 * M_PI * freq * t / sampleRate);

      // Add some harmonics for richness
      snd += 0.5 * std::sin(4 * M_PI * freq * t / sampleRate);
      snd += 0.25 * std::sin(6 * M_PI * freq * t / sampleRate);

      // Mix hammer and tone
      snd = snd * 0.7 + hammer * 0.3;

      // Apply high-pass filter for brightness control
      double hpfFreq = hollowness * 2000 + 100;
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

  // Synth parameters - adjusted for better piano sound
  const double att = 0.005;  // Faster attack
  const double rel = 0.8;    // Shorter release
  const double pan = 0.0;
  const double hollowness = 0.3;
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

class MainComponent : public juce::AudioAppComponent, public juce::Timer {
public:
  MainComponent() {
    setAudioChannels(0, 2); // 0 input channels, 2 output channels
    synth.addVoice(new SimplePianoVoice());
    synth.addSound(new SimplePianoSound());

    // Start the audio thread
    startTimer(500); // 50ms timer for note timing
  }

  ~MainComponent() override {
    stopTimer();
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

  void timerCallback() override {
    if (currentNote < 8) {
      // Note on
      synth.noteOn(1, 60 + currentNote, 0.8f);

      // Schedule note off
      juce::Timer::callAfterDelay(500, [this]() {
        synth.noteOff(1, 60 + currentNote, 0.8f, true);

        // If this was the last note, schedule application close
        if (currentNote == 7) {
          juce::Timer::callAfterDelay(1000, []() {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            });
        }
        });

      currentNote++;
    }
  }

  SimplePianoSynth synth;
  int currentNote = 0;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

int main() {
  juce::ScopedJuceInitialiser_GUI juceInit;
  auto* mainComponent = new MainComponent();

  // Start the audio processing and run the message loop
  mainComponent->prepareToPlay(512, 44100.0);
  juce::MessageManager::getInstance()->runDispatchLoop();

  return 0;
}