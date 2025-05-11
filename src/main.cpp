#include <JuceHeader.h>

// Define M_PI if not defined (Windows compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Configuration structure for piano parameters
struct PianoConfig {
  double attackTime = 0.005;    // Attack time in seconds
  double releaseTime = 0.8;     // Release time in seconds
  double pan = 0.0;            // Stereo pan (-1.0 to 1.0)
  double hollowness = 0.3;     // High-pass filter control
  double hammerStrength = 0.3;  // Hammer excitation strength
  double harmonicMix = 0.5;    // Harmonic content mix
};

// Configuration structure for note parameters
struct NoteConfig {
  double frequency;  // Frequency in Hz
  float velocity;
  double duration;  // Duration in seconds
};

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
  SimplePianoVoice() : t(0) {
    config = PianoConfig();  // Default config
  }

  void setConfig(const PianoConfig& newConfig) {
    config = newConfig;
  }

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
      // Compute envelope using configurable parameters
      double env = amp * std::exp(-t / (config.releaseTime * sampleRate));
      if (t < config.attackTime * sampleRate) {
        env = amp * (t / (config.attackTime * sampleRate));
      }

      // Compute hammer excitation with configurable strength
      double hammerFreq = freq * 2.0;
      double hammer = config.hammerStrength * std::sin(2 * M_PI * hammerFreq * t / sampleRate) * std::exp(-t / (0.02 * sampleRate));

      // Main tone generation with harmonics
      double snd = std::sin(2 * M_PI * freq * t / sampleRate);
      snd += config.harmonicMix * std::sin(4 * M_PI * freq * t / sampleRate);
      snd += (config.harmonicMix * 0.5) * std::sin(6 * M_PI * freq * t / sampleRate);

      // Mix hammer and tone
      snd = snd * 0.7 + hammer * 0.3;

      // Apply high-pass filter with configurable hollowness
      double hpfFreq = config.hollowness * 2000 + 100;
      snd = snd * (1 - std::exp(-2 * M_PI * hpfFreq / sampleRate));

      // Apply envelope
      snd *= env;

      // Apply configurable panning
      double left = snd * (1 - config.pan);
      double right = snd * (1 + config.pan);

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
  PianoConfig config;
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
  MainComponent(const std::vector<NoteConfig>& notes, const PianoConfig& pianoConfig)
    : noteSequence(notes), hasStartedPlaying(false) {
    // Configure synth
    for (int i = 0; i < synth.getNumVoices(); ++i) {
      if (auto* voice = dynamic_cast<SimplePianoVoice*>(synth.getVoice(i))) {
        voice->setConfig(pianoConfig);
      }
    }

    // Initialize audio with 2 output channels
    setAudioChannels(0, 2);
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

  void releaseResources() override {
    // Clean up any resources if needed
  }

  void playNextNote() {
    if (currentNote >= noteSequence.size()) {
      std::cout << "Finished playing all notes. Waiting for final release..." << std::endl;
      // Wait for the last note to finish releasing before quitting
      juce::Timer::callAfterDelay(static_cast<int>((noteSequence.back().duration + 1.0) * 1000), []() {
        std::cout << "Final release complete, quitting..." << std::endl;
        juce::JUCEApplication::quit();
        });
      return;
    }

    const auto& note = noteSequence[currentNote];
    size_t noteIndex = currentNote;  // Store current index before incrementing

    // Convert frequency to MIDI note number
    int midiNote = static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0)));

    std::cout << "Starting note " << noteIndex << " (MIDI: " << midiNote
      << ", Freq: " << note.frequency << "Hz, Duration: " << note.duration << "s)" << std::endl;

    // Play the note
    synth.noteOn(1, midiNote, note.velocity);

    // Schedule note off after the specified duration
    juce::Timer::callAfterDelay(static_cast<int>(note.duration * 1000), [this, midiNote, noteIndex]() {
      std::cout << "Stopping note " << noteIndex << " (MIDI: " << midiNote << ")" << std::endl;
      synth.noteOff(1, midiNote, 0.0f, true);

      // Schedule next note to start after a small gap (50ms) to ensure clean note transitions
      juce::Timer::callAfterDelay(50, [this]() {
        std::cout << "Scheduling next note..." << std::endl;
        ++currentNote;  // Move to next note after scheduling
        playNextNote();
        });
      });
  }

  SimplePianoSynth synth;
  std::vector<NoteConfig> noteSequence;
  size_t currentNote = 0;

private:
  bool hasStartedPlaying;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

int main() {
  juce::ScopedJuceInitialiser_GUI juceInit;

  // Configure piano parameters
  PianoConfig pianoConfig;
  pianoConfig.attackTime = 0.01;    // Slightly longer attack for clarity
  pianoConfig.releaseTime = 0.5;    // Shorter release to prevent overlapping
  pianoConfig.pan = 0.0;
  pianoConfig.hollowness = 0.2;     // Reduced hollowness for clearer tone
  pianoConfig.hammerStrength = 0.2;  // Reduced hammer strength
  pianoConfig.harmonicMix = 0.4;    // Slightly reduced harmonic content

  // Define the note sequence with frequencies in Hz
  std::vector<NoteConfig> notes = {
    {261.63, 0.8f, 0.4},  // C4
    {293.66, 0.8f, 0.4},  // D4
    {329.63, 0.8f, 0.4},  // E4
    {349.23, 0.8f, 0.4},  // F4
    {392.00, 0.8f, 0.4},  // G4
    {440.00, 0.8f, 0.4},  // A4
    {493.88, 0.8f, 0.4},  // B4
    {523.25, 0.8f, 0.4}   // C5
  };

  // Happy Birthday melody
  std::vector<NoteConfig> notes_happy_birthday = {
    // Happy Birthday to you
    {261.63, 0.8f, 0.9},  // C4
    {261.63, 0.8f, 0.9},  // C4
    {293.66, 0.8f, 0.9},  // D4
    {261.63, 0.8f, 0.9},  // C4
    {349.23, 0.8f, 0.9},  // F4
    {329.63, 0.8f, 1.8},  // E4 (longer note)

    // Happy Birthday to you
    {261.63, 0.8f, 0.9},  // C4
    {261.63, 0.8f, 0.9},  // C4
    {293.66, 0.8f, 0.9},  // D4
    {261.63, 0.8f, 0.9},  // C4
    {392.00, 0.8f, 0.9},  // G4
    {349.23, 0.8f, 1.8},  // F4 (longer note)

    // Happy Birthday dear [name]
    {261.63, 0.8f, 0.9},  // C4
    {261.63, 0.8f, 0.9},  // C4
    {523.25, 0.8f, 0.9},  // C5
    {440.00, 0.8f, 0.9},  // A4
    {349.23, 0.8f, 0.9},  // F4
    {329.63, 0.8f, 0.9},  // E4
    {293.66, 0.8f, 1.8},  // D4 (longer note)

    // Happy Birthday to you
    {466.16, 0.8f, 0.9},  // A#4
    {466.16, 0.8f, 0.9},  // A#4
    {440.00, 0.8f, 0.9},  // A4
    {349.23, 0.8f, 0.9},  // F4
    {392.00, 0.8f, 0.9},  // G4
    {349.23, 0.8f, 1.8}   // F4 (longer note)
  };

  auto* mainComponent = new MainComponent(notes_happy_birthday, pianoConfig);
  mainComponent->prepareToPlay(512, 44100.0);

  // Add a small delay to ensure audio system is fully initialized before starting playback
  juce::Timer::callAfterDelay(100, [mainComponent]() {
    mainComponent->playNextNote();
    });

  juce::MessageManager::getInstance()->runDispatchLoop();

  return 0;
}