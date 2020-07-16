// audio-capture.cpp
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/core/utils/threading/Semaphore.h>
#include <aws/transcribestreaming/model/AudioStream.h>
#include <csignal>
#include <cstdio>
#include <portaudio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "feat/wave-reader.h"
#include "matrix/kaldi-matrix.h"

using SampleType = int16_t;
extern int SampleRate;
int Finished = paContinue;
Aws::Utils::Threading::Semaphore pasignal(0 /*initialCount*/, 1 /*maxCount*/);
std::vector<float> wave_piece_buffer;


// static int AudioReadCallback(const void* inputBuffer, void* outputBuffer,  void* userData)
// {
//     auto stream = static_cast<Aws::TranscribeStreamingService::Model::AudioStream *>(userData);
//     // const auto beg = static_cast<const unsigned char*>(inputBuffer);
//     // const auto end = beg + framesPerBuffer * sizeof(SampleType);

//     (void)outputBuffer; // Prevent unused variable warnings
//     // (void)timeInfo;
//     // (void)statusFlags;
 
//     Aws::Vector<unsigned char> bits { beg, end };
//     Aws::TranscribeStreamingService::Model::AudioEvent event(std::move(bits));
//     stream->WriteAudioEvent(event);

//     if (Finished == paComplete) {
//         pasignal.Release(); // signal the main thread to close the stream and exit
//     }

//     return Finished;
// }
int ReadAudio(Aws::TranscribeStreamingService::Model::AudioStream& targetStream)

{

    std::string speech = "voice.wav";
    std::ifstream is(speech, std::ios_base::binary);
    Aws::TranscribeStreamingService::Model::AudioStream* stream = &targetStream;
    WaveData wave;
    wave.Read(is);
    is.close();
    SubVector<BaseFloat> waveform(wave.Data(), 0);
    wave_piece_buffer = waveform.Range(0, waveform.Dim());
    // Aws::Vector<unsigned char> bits { beg, end };
    Aws::TranscribeStreamingService::Model::AudioEvent event(std::move(wave_piece_buffer));
    stream->WriteAudioEvent(event);
}
