CFLAGS=-pedantic -Wall --std=c++11

SRC=src/main.cpp \
    src/ClientConnection.cpp \
    src/ServerConnection.cpp \
    src/HttpRequest.cpp \
    src/HttpResponse.cpp

OUT=cfws

$(OUT): $(SRC)
	$(CXX) $(CFLAGS) -o $(OUT) $(SRC)
