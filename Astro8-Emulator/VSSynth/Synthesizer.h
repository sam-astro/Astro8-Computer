#pragma once

#include <SDL.h>

#include <functional>
#include <vector>

#include "SoundGenerator.h"
#include "SynthMiddleware.h"

namespace VSSynth
{
    struct SynthData
    {
        double *time;
        double sampleDeltaTime;
        std::vector<SoundGenerator *> soundGenerators;
        std::vector<SynthMiddleware *> middleware;
    };

    /**
     * @brief Entrypoint and interface for audio synthesis
     * 
     * The synthesizer is the main building block of the VSSynth audio synthesizer.
     * This synthesizer uses additive synthesis to create sound.
     * 
     * SoundGenerators are responsible for creating the sounds and must be added to this synth.
     * 
     * Middleware can be added to read and/or modify the sound output prior to reaching the speaker.
     * 
     * @see SoundGenerator
     * @see Middleware
     */
    class Synthesizer
    {
    public:
        /**
         * @brief Create a new Synthesizer
         * 
         * @param samplingRates - Sampling rate (# of samples per second).
         * Higher values are computationally intensive while lower values have a limit scope of sound frequencies.
         * @param numFrames - Number of sampling frames. A sampling frame is an instance where sound generators are sampled. 
         */
        Synthesizer(unsigned int samplingRates = 48000, unsigned int numFrames = 20);
        virtual ~Synthesizer();

        /**
         * @brief Creates the internal audio device (SDL should be initialized prior to calling this)
         * 
         * @warning SDL AUDIO must be initialized prior to calling open()
         */
        void open();

        /**
         * @brief Destroys the internal audio device
         * 
         */
        void close();

        /**
         * @brief Pauses the synthesizer
         * 
         * Sound will stop if the synthesizer is currently running
         */
        void pause();

        /**
         * @brief Unpauses the synthesizer
         * 
         * Sound will resume if the synthesizer has been paused
         */
        void unpause();

        /**
         * @brief Adds a sound generator to the synthesizer
         * 
         * @param soundGenerator 
         */
        void addSoundGenerator(SoundGenerator *soundGenerator);

        /**
         * @brief Adds a middleware to the synthesizer
         * 
         * @param middleware 
         */
        void addMiddleware(SynthMiddleware *middleware);

    private:
        SDL_AudioDeviceID mDeviceID;

        double mTime;

        unsigned int mSamplingRate;
        unsigned int mNumFrames;

        SynthData mSynthData;
    };

}; // namespace VSSynth