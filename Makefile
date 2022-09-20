CXX=g++
LD=g++

CFLAGS=-pedantic -Wall --std=c++17

OBJS=src/main.o \
    src/CGIScript.o \
    src/ClientConnection.o \
    src/ServerConnection.o \
    src/HttpRequest.o \
    src/HttpResponse.o

all: cfws

%.o: %.cpp
	$(CXX) $< -o $@ -c $(CFLAGS)

cfws: $(OBJS)
	$(LD) $^ -o $@ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) cfws
