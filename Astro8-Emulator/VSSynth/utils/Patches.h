#pragma once

#include "Envelope.h"
#include "Waveforms.h"

namespace VSSynth
{
    /**
     * @brief A collection of different synth patches and their envelopes
     */
    namespace Patches
    {
        /**
         * @brief A Patch takes a frequency and time. It outputs a signed value between -1.0 and 1.0
         */
        typedef std::function<double(double, double)> Patch;

        const Patch BASS = [](double freq, double time) {
            return 0.5 * (Waveforms::pulse(freq / 2, time, 10) +
                          0.5 * (Waveforms::square(freq, time)) + 0.0001 * Waveforms::noise());
        };

        const Patch BRASS = [](double freq, double time) {
            return 0.5 * (Waveforms::pulse(freq, time, 25) + 0.25 * Waveforms::sawtooth(freq, time) + 0.0001 * Waveforms::noise());
        };

        const Patch CYMBAL = [](double freq, double time) {
            return 0.25 * Waveforms::noise();
        };

        const Patch GLOCKENSPIEL = [](double freq, double time) {
            return 0.5 * (Waveforms::sine(freq * 2, time) + 0.0001 * Waveforms::noise());
        };

        const Patch GUITAR = [](double freq, double time) {
            return 0.5 * (Waveforms::pulse(freq, time, 10) +
                          0.5 * (Waveforms::square(freq * 2, time)) + 0.0001 * Waveforms::noise());
        };

        const Patch ORGAN = [](double freq, double time) {
            return 0.5 * (Waveforms::sine(freq, time) +
                          0.5 * Waveforms::sine(freq * 2, time) +
                          0.25 * Waveforms::sine(freq * 4, time) +
                          0.125 * Waveforms::sine(freq * 8, time));
        };

        const Patch PIANO = [](double freq, double time) {
            return 0.5 * (Waveforms::sine(freq, time) +
                          0.5 * Waveforms::sine(freq * 2, time) +
                          0.25 * Waveforms::sine(freq * 3, time));
        };

        const Patch REED = [](double freq, double time) {
            return 0.5 * (Waveforms::triangle(freq * 2, time) + 0.5 * Waveforms::sine(freq, time) + 0.25 * Waveforms::sawtooth(freq, time) + 0.04 * Waveforms::noise());
        };

        const ADSREnvelope BASS_ENVELOPE(1.00f, 0.15f, 0.001f, 0.1f, 0.80f, false);
        const ADSREnvelope BRASS_ENVELOPE(0.90f, 0.30f, 0.1f, 0.1f, 0.05f);
        const ADSREnvelope CYMBAL_ENVELOPE(0.90f, 0.30f, 0.001f, 0.5f, 1.00f);
        const ADSREnvelope GLOCKENSPIEL_ENVELOPE(0.90f, 0.30f, 0.1f, 0.1f, 1.00f, true);
        const ADSREnvelope GUITAR_ENVELOPE(1.00f, 0.15f, 0.001f, 0.1f, 0.80f, false);
        const ADSREnvelope ORGAN_ENVELOPE(0.90f, 0.30f, 0.1f, 0.1f, 0.40f);
        const ADSREnvelope PIANO_ENVELOPE(0.90f, 0.30f, 0.1f, 0.1f, 0.30f);
        const ADSREnvelope REED_ENVELOPE(0.90f, 0.30f, 0.1f, 0.1f, 0.05f);

    }; // namespace Patches
};     // namespace VSSynth