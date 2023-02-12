#pragma once

namespace VSSynth
{
    /**
     * @brief Collection of sound generators provided with VSSynth
     */
    namespace Generators
    {
    }

    /**
     * @brief Sound generating device
     *
     * A sound generator is responsible for creating the sound output which will be eventually sent to the speaker.
     * Sound generators can be simple waveforms, a modulated waveform from an ADSR envelope, or it can playback PCM audio from a file.
     */
    class SoundGenerator
    {
    public:
        SoundGenerator();
        virtual ~SoundGenerator();

        /**
         * @brief Sample the SoundGenerator at the given time
         * 
         * This is the main interaction between the Synthesizer and the SoundGenerator. This method will be called at the rate of the sampling rate specified by the Synthesizer's constructor.
         * 
         * @warning Since this function is called so frequently you should make computations as fast and efficient as possible.
         * Needless to say, do not block the running thread when implementing this function.
         * 
         * @param time - in seconds
         * @return double - sound sample in range [-1.0,1.0]
         *
         * @see Synthesizer
         */
        virtual double sample(double time) = 0;

        /**
         * @brief Sets the volume at a certain percent
         * 
         * @param percent - volume as a percent in range [0,100]
         */
        void setVolume(double percent);

        /**
         * @brief Gets the current volume as a percent
         * 
         * @return double - volume as a percent [0, 100]
         */
        double getVolume();

        /**
         * @brief Amplitude for synthesizer
         * 
         * Sound generators, when sampled, return a value in [-1.0, 1.0].
         * This value will be multiplied by the amplitude returned by this
         * function. The amplitude will be a value such that it can fit
         * in a 16-bit signed integer.
         * 
         * This function is mainly for the Synthesizer. You shouldn't
         * really have a reason to call this function.
         * 
         * @see Synthesizer
         * @return double 
         */
        double getAmplitude();

    protected:
        double mAmplitude;
    };
} // namespace VSSynth