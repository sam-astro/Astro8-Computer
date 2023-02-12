#pragma once

namespace VSSynth
{

    /**
    * @brief ADSR envelope data
    * 
    * Represent the ADSR curve with five parameters:
    * 
    * Attack Amplitude
    * Sustain Amplitude
    * Attack Time
    * Decay Time
    * Release Time
    * 
    * An optional sixth paramater, sustainable, can also be taken.
    * Sustainable means that the envelope can enter the sustain section of the curve.
    * If sustainable is false, the curve will go directly from decay to release.
    *
    */
    struct ADSREnvelope
    {
        ADSREnvelope(double attack, double sustain, double attackTime, double decayTime, double releaseTime, bool sustainable = true)
            : attack(attack),
              sustain(sustain),
              attackTime(attackTime),
              decayTime(decayTime),
              releaseTime(releaseTime),
              sustainable(sustainable)
        {
        }

        double attack, sustain;                    // Amplitudes
        double attackTime, decayTime, releaseTime; // Time-lengths
        bool sustainable;
    };

    /**
     * @brief Waveform modulator
     * 
     * Modulates audio waveforms in accordance with the ADSR envelope.
     * Envelopes can be manipulated with the hold() and release() functions.
     * 
     * @see hold()
     * @see release()
     */
    class Envelope
    {
    public:
        /**
         * @brief Construct a new Envelope object
         * 
         * @param adsr - ADSR data
         */
        Envelope(const ADSREnvelope adsr);

        /**
         * @brief Construct a new Envelope object
         */
        Envelope();

        virtual ~Envelope();

        /**
         * @brief Get the current amplitude modifier
         * @return the current amplitude modifier
         */
        double getAmplitude() const;

        /**
         * @brief Update the timestamp for the envelope
         * 
         * While the envelope is within the Attack -> Release state,
         * the amplitudes returned by the envelope will modulate each
         * time this function is called. When the envelope is outside
         * those ranges of states, this function does nothing.
         * 
         * @param deltaTime - time in milliseconds
         */
        void update(double deltaTime);

        /**
         * @brief Sets a new ADSR for this envelope
         * @param adsr
         */
        void setADSR(const ADSREnvelope adsr);

        /**
         * @brief Starts the envelopes modulation
         * 
         * Starts the enevlope, or in other terms "presses the key".
         * Calling this function will restart the envelope to the attack state.
         */
        void hold();

        /**
         * @brief releases
         * 
         * Starts the ending for the envelope, or in other terms "releases the key".
         * Calling this function will transition the envelope to the release state.
         */
        void release();

    private:
        // Different curves of the ADSR + an invalid section to represent no sound
        enum Curves
        {
            ATTACK,
            DECAY,
            SUSTAIN,
            RELEASE,
            INACTIVE
        } mCurrentCurve;

        ADSREnvelope mADSR;

        bool mHold;
        double mAmplitude;
    };
}; // namespace VSSynth