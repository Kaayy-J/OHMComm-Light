/*
 * File:   AudioProcessorFactory.h
 * Author: daniel
 *
 * Created on June 27, 2015, 10:15 AM
 */

#ifndef AUDIOPROCESSORFACTORY_H
#define	AUDIOPROCESSORFACTORY_H

#include "AudioProcessor.h"
#include <vector>
#include <stdexcept>

/*!
 * Factory-class to provide audio-processor objects without needing to know the details of the implementation.
 */
class AudioProcessorFactory
{
public:
    static const std::string OPUS_CODEC;
    static const std::string WAV_WRITER;

    /*!
     * Returns the AudioProcessor for the given name
     *
     * \param name The name to look for
     *
     * \param createProfiler whether to create a profiler for the audio-processor
     */
    static AudioProcessor* getAudioProcessor(const std::string name, bool createProfiler);

    /*!
     * \return a list of the names of all available audio-processors
     */
    static const std::vector<std::string> getAudioProcessorNames();
private:

};

#endif	/* AUDIOPROCESSORFACTORY_H */

