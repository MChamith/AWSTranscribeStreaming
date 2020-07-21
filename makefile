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
	g++ -std=c++11 -g -Wall -Wextra -Werror  -I.  -o main main.cpp  -L$/usr/local/lib -L$/home/chamith/aws_deps/lib   -I/usr/local/include  -laws-cpp-sdk-transcribestreaming -laws-cpp-sdk-core -laws-c-event-stream -laws-c-common -laws-checksums    -lpistache -ljsoncpp -lportaudio -pthread -lcurl -lcrypto