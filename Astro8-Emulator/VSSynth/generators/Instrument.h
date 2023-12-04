#pragma once

#include <functional>

#include "../utils/Envelope.h"
#include "../SoundGenerator.h"

namespace VSSynth
{
    namespace Generators
    {
        /**
         * @brief Device capable of playing multiple notes with an ADSR envelope 
         */
        class Instrument : public SoundGenerator
        {
        public:
            Instrument(std::function<double(double, double)> wave);

            virtual ~Instrument();

            /**
             * @brief Samples the instrument at the given time
             * 
             * @param time
             * @return double 
             */
            virtual double sample(double time) = 0;

            /**
             * @brief Holds the given note
             * 
             * @param frequency - the note
             */
            virtual void holdNote(double frequency) = 0;
            /**
             * @brief Releases the given note
             * 
             * @param frequency - the note
             */
            virtual void releaseNote(double frequency) = 0;

        protected:
            std::function<double(double, double)> mWave;
        };
    } // namespace Generators
} // namespace VSSynth