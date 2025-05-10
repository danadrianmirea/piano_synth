SynthDef(\cheapPiano1, {
    arg
        out = 0, freq = 440, amp = 0.1, att = 0.001, rel = 1, pan = 0,
        tone = 0.25, hollowness = 0;
    var hammer, snd, delay, tonefreq, env;
    env = Env.perc(att, rel, amp * 4, -1).ar(doneAction: 2);
    // Delay line times: freq, freq - 4 cents, freq + 3 cents
    delay = 1 / (freq * [2.pow(-0.04/12), 1, 2.pow(0.03/12)]);
    tonefreq = tone.linlin(0, 1, 1000, 5000);
    hammer = Decay2.ar(
        Impulse.ar(0.001),
        0.008,
        0.04,
        LFNoise2.ar(freq: amp.linlin(0, 1, tonefreq, 2 * tonefreq), mul: 0.25)
    );
    snd = CombL.ar(hammer, delay, delay, 50 * amp);
    snd = HPF.ar(snd, hollowness.linlin(0, 1, 50, 1000));
    snd = snd * env;
    snd = Limiter.ar(snd);
    Out.ar(out, Pan2.ar(snd, pan));
},
metadata: (
    credit: "based on something posted 2008-06-17 by jeff, based on an old example by james mcc",
    category: \keyboards,
    tags: [\casio, \piano, \pitched]
)).add;



// Play the major octave (C4 to C5) on the piano synth
(
Pbind(
    \instrument, \cheapPiano1,   // Use the cheapPiano1 synth
    \freq, Pseq([261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25], inf), // Notes of the major scale (C4 to C5)
    \amp, 0.3,                   // Amplitude
    \att, 0.01,                  // Attack time
    \rel, 1.3,                   // Release time
    \pan, 0,                     // Pan (center)
    \tone, 0.5,                  // Tone factor
    \hollowness, 0.2             // Hollowness factor
).play;
)
