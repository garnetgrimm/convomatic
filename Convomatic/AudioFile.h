//=======================================================================
/** @file AudioFile.h
 *  @author Adam Stark
 *  @copyright Copyright (C) 2017  Adam Stark
 *
 * This file is part of the 'AudioFile' library
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//=======================================================================

//https://github.com/adamstark/AudioFile

#ifndef _AS_AudioFile_h
#define _AS_AudioFile_h

#include <iostream>
#include <vector>
#include <assert.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <iterator>
#include <algorithm>

//=============================================================
/** The different types of audio file, plus some other types to
 * indicate a failure to load a file, or that one hasn't been
 * loaded yet
 */
enum class AudioFileFormat
{
    Error,
    NotLoaded,
    Wave,
    Aiff
};

//=============================================================
template <class T>
class AudioFile
{
public:

    //=============================================================
    typedef std::vector<std::vector<T> > AudioBuffer;

    //=============================================================
    /** Constructor */
    AudioFile();

    //=============================================================
    /** Loads an audio file from a given file path.
     * @Returns true if the file was successfully loaded
     */
    bool load (std::string filePath);

    /** Saves an audio file to a given file path.
     * @Returns true if the file was successfully saved
     */
    bool save (std::string filePath, AudioFileFormat format = AudioFileFormat::Wave);

    //=============================================================
    /** @Returns the sample rate */
    uint32_t getSampleRate() const;

    /** @Returns the number of audio channels in the buffer */
    int getNumChannels() const;

    /** @Returns true if the audio file is mono */
    bool isMono() const;

    /** @Returns true if the audio file is stereo */
    bool isStereo() const;

    /** @Returns the bit depth of each sample */
    int getBitDepth() const;

    /** @Returns the number of samples per channel */
    int getNumSamplesPerChannel() const;

    /** @Returns the length in seconds of the audio file based on the number of samples and sample rate */
    double getLengthInSeconds() const;

    /** Prints a summary of the audio file to the console */
    void printSummary() const;

    //=============================================================

    /** Set the audio buffer for this AudioFile by copying samples from another buffer.
     * @Returns true if the buffer was copied successfully.
     */
    bool setAudioBuffer (AudioBuffer& newBuffer);

    /** Sets the audio buffer to a given number of channels and number of samples per channel. This will try to preserve
     * the existing audio, adding zeros to any new channels or new samples in a given channel.
     */
    void setAudioBufferSize (int numChannels, int numSamples);

    /** Sets the number of samples per channel in the audio buffer. This will try to preserve
     * the existing audio, adding zeros to new samples in a given channel if the number of samples is increased.
     */
    void setNumSamplesPerChannel (int numSamples);

    /** Sets the number of channels. New channels will have the correct number of samples and be initialised to zero */
    void setNumChannels (int numChannels);

    /** Sets the bit depth for the audio file. If you use the save() function, this bit depth rate will be used */
    void setBitDepth (int numBitsPerSample);

    /** Sets the sample rate for the audio file. If you use the save() function, this sample rate will be used */
    void setSampleRate (uint32_t newSampleRate);

    //=============================================================
    /** Sets whether the library should log error messages to the console. By default this is true */
    void shouldLogErrorsToConsole (bool logErrors);

    //=============================================================
    /** A vector of vectors holding the audio samples for the AudioFile. You can
     * access the samples by channel and then by sample index, i.e:
     *
     *      samples[channel][sampleIndex]
     */
    AudioBuffer samples;

private:

    //=============================================================
    enum class Endianness
    {
        LittleEndian,
        BigEndian
    };

    //=============================================================
    AudioFileFormat determineAudioFileFormat (std::vector<uint8_t>& fileData);
    bool decodeWaveFile (std::vector<uint8_t>& fileData);
    bool decodeAiffFile (std::vector<uint8_t>& fileData);

    //=============================================================
    bool saveToWaveFile (std::string filePath);
    bool saveToAiffFile (std::string filePath);

    //=============================================================
    void clearAudioBuffer();

    //=============================================================
    int32_t fourBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int16_t twoBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int getIndexOfString (std::vector<uint8_t>& source, std::string s);

    //=============================================================
    T sixteenBitIntToSample (int16_t sample);
    int16_t sampleToSixteenBitInt (T sample);

    //=============================================================
    uint8_t sampleToSingleByte (T sample);
    T singleByteToSample (uint8_t sample);

    uint32_t getAiffSampleRate (std::vector<uint8_t>& fileData, int sampleRateStartIndex);
    bool tenByteMatch (std::vector<uint8_t>& v1, int startIndex1, std::vector<uint8_t>& v2, int startIndex2);
    void addSampleRateToAiffData (std::vector<uint8_t>& fileData, uint32_t sampleRate);
    T clamp (T v1, T minValue, T maxValue);

    //=============================================================
    void addStringToFileData (std::vector<uint8_t>& fileData, std::string s);
    void addInt32ToFileData (std::vector<uint8_t>& fileData, int32_t i, Endianness endianness = Endianness::LittleEndian);
    void addInt16ToFileData (std::vector<uint8_t>& fileData, int16_t i, Endianness endianness = Endianness::LittleEndian);

    //=============================================================
    bool writeDataToFile (std::vector<uint8_t>& fileData, std::string filePath);

    //=============================================================
    void reportError (std::string errorMessage);

    //=============================================================
    AudioFileFormat audioFileFormat;
    uint32_t sampleRate;
    int bitDepth;
    bool logErrorsToConsole {true};
};


//=============================================================
// Pre-defined 10-byte representations of common sample rates
static std::unordered_map <uint32_t, std::vector<uint8_t>> aiffSampleRateTable = {
    {8000, {64, 11, 250, 0, 0, 0, 0, 0, 0, 0}},
    {11025, {64, 12, 172, 68, 0, 0, 0, 0, 0, 0}},
    {16000, {64, 12, 250, 0, 0, 0, 0, 0, 0, 0}},
    {22050, {64, 13, 172, 68, 0, 0, 0, 0, 0, 0}},
    {32000, {64, 13, 250, 0, 0, 0, 0, 0, 0, 0}},
    {37800, {64, 14, 147, 168, 0, 0, 0, 0, 0, 0}},
    {44056, {64, 14, 172, 24, 0, 0, 0, 0, 0, 0}},
    {44100, {64, 14, 172, 68, 0, 0, 0, 0, 0, 0}},
    {47250, {64, 14, 184, 146, 0, 0, 0, 0, 0, 0}},
    {48000, {64, 14, 187, 128, 0, 0, 0, 0, 0, 0}},
    {50000, {64, 14, 195, 80, 0, 0, 0, 0, 0, 0}},
    {50400, {64, 14, 196, 224, 0, 0, 0, 0, 0, 0}},
    {88200, {64, 15, 172, 68, 0, 0, 0, 0, 0, 0}},
    {96000, {64, 15, 187, 128, 0, 0, 0, 0, 0, 0}},
    {176400, {64, 16, 172, 68, 0, 0, 0, 0, 0, 0}},
    {192000, {64, 16, 187, 128, 0, 0, 0, 0, 0, 0}},
    {352800, {64, 17, 172, 68, 0, 0, 0, 0, 0, 0}},
    {2822400, {64, 20, 172, 68, 0, 0, 0, 0, 0, 0}},
    {5644800, {64, 21, 172, 68, 0, 0, 0, 0, 0, 0}}
};

#endif /* AudioFile_h */
