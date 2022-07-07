CC=clang++
CFLAGS=-pedantic -Wall --std=c++11

SRC=src/main.cpp \
    src/ClientConnection.cpp \
    src/ServerConnection.cpp \
    src/HttpRequest.cpp \
    src/HttpResponse.cpp

OUT=cfws

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)
