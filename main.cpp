// main.cpp
#include <aws/core/Aws.h>
#include <aws/core/utils/threading/Semaphore.h>
#include <aws/transcribestreaming/TranscribeStreamingServiceClient.h>
#include <aws/transcribestreaming/model/StartStreamTranscriptionHandler.h>
#include <aws/transcribestreaming/model/StartStreamTranscriptionRequest.h>
#include <cstdio>
#include <aws/core/platform/FileSystem.h>
#include <fstream>

using namespace Aws;
using namespace Aws::TranscribeStreamingService;
using namespace Aws::TranscribeStreamingService::Model;


int SampleRate = 16000; // 16 Khz
// int CaptureAudio(AudioStream& targetStream);
static const char TEST_FILE_NAME[] = "voice.wav";

int main()
{
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);


    {
        Aws::Client::ClientConfiguration config;
        config.connectTimeoutMs = 30000;
config.requestTimeoutMs = 600000;
#ifdef _WIN32
        config.httpLibOverride = Aws::Http::TransferLibType::WIN_INET_CLIENT;
#endif
        TranscribeStreamingServiceClient client(config);
        StartStreamTranscriptionHandler handler;
        Aws::String transcribedResult;
        
        handler.SetTranscriptEventCallback([](const TranscriptEvent& ev) {
            
            for (auto&& r : ev.GetTranscript().GetResults()) {
                if (r.GetIsPartial()) {
                    printf("[partial] ");
                } else {
                    printf("[Final] ");
                }
                for (auto&& alt : r.GetAlternatives()) {
                    printf("%s\n", alt.GetTranscript().c_str());
                    
                }
            }
            
        });

        StartStreamTranscriptionRequest request;
        request.SetMediaSampleRateHertz(SampleRate);
        request.SetLanguageCode(LanguageCode::en_US);
        request.SetMediaEncoding(MediaEncoding::pcm);
        request.SetEventStreamHandler(handler);

        auto OnStreamReady = [](AudioStream& stream) {
            Aws::FStream file(TEST_FILE_NAME, std::ios_base::in | std::ios_base::binary);
       
        char buf[1024];
        while(file)
        {
            file.read(buf, sizeof(buf));
            Aws::Vector<unsigned char> bits{buf, buf + file.gcount()};
            AudioEvent event(std::move(bits));
            if (!stream)
            {
                printf("stream not");
                break;
            }
            if (!stream.WriteAudioEvent(event))
            {
                printf("stream not writing");
                break;
            }
        }
        stream.WriteAudioEvent({}); // per the spec, we have to send an empty event (i.e. without a payload) at the end.
        stream.flush();
        stream.Close();
        printf("stream closed");

        };

        Aws::Utils::Threading::Semaphore signaling(0 /*initialCount*/, 1 /*maxCount*/);
        auto OnResponseCallback = [&signaling](const TranscribeStreamingServiceClient*,
                  const Model::StartStreamTranscriptionRequest&,
                  const Model::StartStreamTranscriptionOutcome&,
                  const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) { signaling.Release(); };

        client.StartStreamTranscriptionAsync(request, OnStreamReady, OnResponseCallback, nullptr /*context*/);
        signaling.WaitOne(); // prevent the application from exiting until we're done
    }

    Aws::ShutdownAPI(options);

    return 0;
}