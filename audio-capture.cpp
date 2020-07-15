// audio-capture.cpp
// #include <aws/core/utils/memory/stl/AWSVector.h>
// #include <aws/core/utils/threading/Semaphore.h>
// #include <aws/transcribestreaming/model/AudioStream.h>
// #include <csignal>
// #include <cstdio>
// #include <portaudio.h>
// #include <iostream>
// #include <string>
// #include <fstream>
// #include "feat/wave-reader.h"
// #include "matrix/kaldi-matrix.h"
// #include <vector>
// #include "base/kaldi-common.h"
// #include "util/common-utils.h"
// #include "feat/feature-mfcc.h"
// #include "feat/feature-functions.h"

// using SampleType = int16_t;
// extern int SampleRate;
// int Finished = paContinue;
// Aws::Utils::Threading::Semaphore pasignal(0 /*initialCount*/, 1 /*maxCount*/);


// using namespace kaldi;
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
// int CaptureAudio(Aws::TranscribeStreamingService::Model::AudioStream& targetStream)

// {
//     Vector<BaseFloat> wave_piece_buffer;
//     std::string speech = "voice.wav";
//     std::ifstream is(speech, std::ios_base::binary);
//     Aws::TranscribeStreamingService::Model::AudioStream* stream = &targetStream;
//     WaveData wave;
//     wave.Read(is);
//     is.close();
//     SubVector<BaseFloat> waveform(wave.Data(), 0);
//     int32 wave_length = waveform.Dim(),			   // audio length
// 		audio_segment = 0.01 * wave.SampFreq(),	// # of samples for 10ms
// 		num_segment = wave_length / audio_segment, // # of 10ms segments in total
// 		offset_start = 0;
    
//     wave_piece_buffer = waveform.Range(0, wave_length);
//     unsigned char *buf = NULL;
//     for (int i = 0; i < wave_length; i++)
// 	{
// 		buf[i] = wave_piece_buffer(i);
// 	}
//     Aws::Vector<unsigned char> bits{buf[0], buf[wave_length]};
//     // Aws::Vector<unsigned char> bits { beg, end };
//     Aws::TranscribeStreamingService::Model::AudioEvent event(std::move(bits));
//     stream->WriteAudioEvent(event);
// }



// audio-capture.cpp
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/core/utils/threading/Semaphore.h>
#include <aws/transcribestreaming/model/AudioStream.h>
#include <csignal>
#include <cstdio>
#include <portaudio.h>

using SampleType = int16_t;
extern int SampleRate;
int Finished = paContinue;
Aws::Utils::Threading::Semaphore pasignal(0 /*initialCount*/, 1 /*maxCount*/);

static int AudioCaptureCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    auto stream = static_cast<Aws::TranscribeStreamingService::Model::AudioStream *>(userData);
    const auto beg = static_cast<const unsigned char*>(inputBuffer);
    const auto end = beg + framesPerBuffer * sizeof(SampleType);

    (void)outputBuffer; // Prevent unused variable warnings
    (void)timeInfo;
    (void)statusFlags;
 
    Aws::Vector<unsigned char> bits { beg, end };
    Aws::TranscribeStreamingService::Model::AudioEvent event(std::move(bits));
    stream->WriteAudioEvent(event);

    if (Finished == paComplete) {
        pasignal.Release(); // signal the main thread to close the stream and exit
    }

    return Finished;
}

void interruptHandler(int)
{
    Finished = paComplete;
}
 
int CaptureAudio(Aws::TranscribeStreamingService::Model::AudioStream& targetStream)
{
 
    signal(SIGINT, interruptHandler); // handle ctrl-c
    PaStreamParameters inputParameters;
    PaStream* stream;
    PaError err = paNoError;
 
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Error: Failed to initialize PortAudio.\n");
        return -1;
    }

    inputParameters.device = Pa_GetDefaultInputDevice(); // default input device
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        Pa_Terminate();
        return -1;
    }

    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    // start the audio capture
    
    err = Pa_OpenStream(&stream, &inputParameters, nullptr, /* &outputParameters, */
        SampleRate, paFramesPerBufferUnspecified,
        paClipOff, // you don't output out-of-range samples so don't bother clipping them.
        AudioCaptureCallback, &targetStream);

    // static int AudioCaptureCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    // const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    if (err != paNoError) {
        fprintf(stderr, "Failed to open stream.\n");        
        goto done;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "Failed to start stream.\n");
        goto done;
    }
    printf("=== Now recording!! Speak into the microphone. ===\n");
    fflush(stdout);

    if ((err = Pa_IsStreamActive(stream)) == 1) {
        pasignal.WaitOne();
    }
    if (err < 0) {
        goto done;
    }

    Pa_CloseStream(stream);

done:
    Pa_Terminate();
    return 0;
}