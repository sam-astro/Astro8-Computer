#pragma once

namespace VSSynth
{
    /**
     * @brief Collection of middleware for use with the synthesizer
     */
    namespace Middleware
    {

    }

    /**
     * @brief Read and modify samples prior to sending them to the speaker 
     * 
     * Middleware has the ability to modify any samples prior to sending it to the speaker.
     * This can allow for the creation of filters, sample analyzers or audio file writers.
     * 
     * @see Synthesizer
     */
    class SynthMiddleware
    {
    public:
        /**
         * @brief  Process the current sample and output the modified sample
         * @param currentSample 
         * @param time 
         * @return short 
         */
        virtual short processSample(short currentSample, double time) = 0;
    };
} // namespace VSSynth