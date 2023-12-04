#pragma once

#include "../SynthMiddleware.h"

#include <fstream>
#include <string>

namespace VSSynth
{

    namespace Middleware
    {

        /**
         * @brief Outputs Audio Samples to a wave file
         * 
         * WAVWriter will take audio samples and write them into a WAV file.
         * In order to accomplish this with the best performance, it will
         * delay writing to a file until a large buffer has been filled.
         * Also a separate thread will be created to write to the file.
         */
        class WAVWriter : public SynthMiddleware
        {
        public:
            WAVWriter(unsigned long int samplingRate, unsigned int channels = 2);
            ~WAVWriter();

            /**
             * @brief Opens a file for writing
             * @param filePath 
             */
            void open(std::string filePath);

            /**
             * @brief Closes a file for writing
             */
            void close();

            /**
             * @brief Writes the given sample to file
             * @param sample
             */
            short processSample(short sample, double time);

        private:
            void writeRIFFHeader();
            void writeFormatSubChunk();
            void writeDataSubChunkHeader();
            void writeChunkSizes();

            std::ofstream mWAVFile;

            unsigned long int mNumWritten;
            unsigned long int mSamplingRate;
            unsigned int mNumChannels;
        };

    }; // namespace Middleware

} // namespace VSSynth