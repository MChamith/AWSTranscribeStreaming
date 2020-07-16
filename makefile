PROJECT_LOCATION = .
# path to kaldi
KALDI_LOCATION = /opt/kaldi
OPENFST_LOCATION = $(KALDI_LOCATION)/tools/openfst
ATLAS_HEADERLOCATION = $(KALDI_LOCATION)/tools/ATLAS
ATLAS_LIBLOCATION = /usr/lib64
# path to libfvad
WEBRTC_LIBLOCATION = /usr/local/lib


all : main

main : main.cpp 
	g++ -std=c++11 -g -Wall -Wextra -Werror -I$(KALDI_LOCATION)/src -I$(OPENFST_LOCATION)/include -DHAVE_ATLAS -I$(ATLAS_HEADERLOCATION)/include -I$(PROJECT_LOCATION)/src -I$(PROJECT_LOCATION)/src/extension -I$(WEBRTC_LIBLOCATION)/include -I.  -o main main.cpp audio-capture.cpp -L$/usr/local/lib -L$/home/chamith/aws_deps/lib -I/usr/lib/x86_64-linux-gnu/  -I/usr/local/include -L$(KALDI_LOCATION)/src/lib -L$(OPENFST_LOCATION)/lib -L$(ATLAS_LIBLOCATION)/atlas -lkaldi-base -lkaldi-feat -lkaldi-util -lkaldi-matrix -lkaldi-hmm -lkaldi-gmm -lkaldi-ivector -lkaldi-transform -lkaldi-tree -lfst -lfvad -laws-cpp-sdk-transcribestreaming -laws-cpp-sdk-core -laws-c-event-stream -laws-c-common -laws-checksums    -lpistache -ljsoncpp -lportaudio -pthread -lcurl -lcrypto