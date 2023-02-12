#pragma once

#include "Instrument.h"

#include <vector>

namespace VSSynth
{
    namespace Generators
    {

        /**
         * @brief Note Sequencer for Instruments
         * 
         * A sequencer will queue up notes to be played for the set instrument.
         * 
         */
        class Sequencer : public SoundGenerator
        {
        public:
            Sequencer(Instrument *instrument);
            virtual ~Sequencer();

            double sample(double time);

            /**
             * @brief Queue a note to be played
             * 
             * @param note The note to play (Frequency in Hertz)
             * @param startTime Time to start the note (Seconds)
             * @param duration How long to hold the note (Seconds)
             */
            void queueNote(double note, double startTime, double duration);

            /**
             * @brief Sort all notes by their queued times
             *
             * You should call this function after you have finished
             * calling queueNote(). You only need to call it once, after all
             * other calls to queueNote().
             * 
             * This is important if you have queued notes out of order.
             * This includes notes that have a duration that lasts after the
             * next queued note begins.
             */
            void sortNotes();

            /**
             * @brief Set the sequencer to loop at end of note sequence
             * @param loop 
             */
            void setLooping(bool loop);

        private:
            struct NoteEvent
            {
                double note;
                bool hold;
            };

            Instrument *mInstrument;

            double mStartTime;
            double mCurTime;

            bool mLoop;

            std::vector<std::pair<NoteEvent, double>> mEvents;
            std::vector<std::pair<NoteEvent, double>>::iterator mEventIt;
        };

    } // namespace Generators
} // namespace VSSynth