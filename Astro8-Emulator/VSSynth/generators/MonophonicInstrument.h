#pragma once

#include "Instrument.h"

namespace VSSynth
{
    namespace Generators
    {
        /**
         * @brief Device capable of playing a single note at a time.
         * A monophonic instrument is capable of playing
         * only a single note at a time. Real life examples
         * include: woodwind, brass and many of the 
         * percussive instruments.
         */
        class MonophonicInstrument : public Instrument
        {
        public:
            /**
             * @brief Creates a Monophonic instrument with the given waveform and adsr curve
             * 
             * @param wave - the waveform to output
             * @param adsr - the ADSR curve
             */
            MonophonicInstrument(
                std::function<double(double, double)> wave,
                const ADSREnvelope &adsr);

            virtual ~MonophonicInstrument();

            double sample(double time);

            void holdNote(double frequency);
            void releaseNote(double frequency);

        private:
            Envelope mEnvelope;

            double mCurrentNote;
            double mPrevSample;
        };

    } // namespace Generators
} // namespace VSSynth
