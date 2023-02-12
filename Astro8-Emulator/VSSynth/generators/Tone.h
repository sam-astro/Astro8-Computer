#pragma once

#include "../SoundGenerator.h"

#include <functional>
#include <mutex>
#include <vector>

namespace VSSynth
{
    namespace Generators
    {

        /**
         * @brief Generates a given tone at multiple frequencies.
         */
        class Tone : public SoundGenerator
        {
        public:
            /**
             * @brief Construct a new Tone object
             * 
             * @param wave : soundwave to output
             */
            Tone(const std::function<double(double, double)> wave);
            virtual ~Tone();

            double sample(double time);

            /**
             * @brief Plays the waveform at the given frequency
             * 
             * @param frequency 
             */
            void playNote(double frequency);

            /**
             * @brief Stops playing the waveform at the given frequency
             * 
             * @param frequency 
             */
            void stopNote(double frequency);

        private:
            std::mutex mNoteLock;
            std::vector<double> mNotes;

            std::function<double(double, double)> mWave;
        };

    } // namespace Generators
};    // namespace VSSynth