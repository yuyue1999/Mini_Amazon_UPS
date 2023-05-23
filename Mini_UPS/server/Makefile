CC=g++
CFLAGS=-g
PROTO_CC = ./protobuf/world_ups.pb.cc 
PROTO_H = ./protobuf/world_ups.pb.h 
GPB = `pkg-config --cflags --libs protobuf`
VERSION = -std=c++11
THREAD = -pthread
PQXX = -lpqxx -lpq
CPPFILES = main.cpp
HEADERS = helper.hpp socket.hpp amazonreq.hpp worldreq.hpp threadpool.hpp

all: main

main:$(HEADERS) $(PROTO_H) $(CPPFILES)
	$(CC) $(CFLAGS)  -o main $(CPPFILES) $(PROTO_CC) $(GPB) $(PQXX) $(THREAD) $(VERSION)


.PHONY:
	clean

clean:
	rm -rf *.o main