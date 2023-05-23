CC=g++

EXTRAFLAGS=-lpqxx -lpq -lpthread
GOOGLE_BUFFER= `pkg-config --cflags --libs protobuf`
PROTO= protobuf/amazon_ups.pb.cc protobuf/world_amazon.pb.cc 
<<<<<<< HEAD
CXXFLAGS = -std=c++17 -O2 -g -mcmodel=large
SERVER= common.cpp server.cpp
=======
CXXFLAGS = -std=c++11 -O2 -g -mcmodel=large
SERVER= common.cpp server.cpp frontend_handle.hpp
>>>>>>> 7700c18a4b07750c159e258fbecc3311f3dedfe4
AUX = tinyxml2.cpp
TASKFLOW = -I.

<<<<<<< HEAD
server: server.cpp common.cpp
	$(CC) $(CXXFLAGS) $(TASKFLOW) -o $@ $(SERVER) $(PROTO) $(AUX) $(GOOGLE_BUFFER) $(EXTRAFLAGS)
=======
server: server.cpp common.cpp frontend_handle.hpp
	$(CC) $(CXXFLAGS) -o $@ $(SERVER) $(PROTO) $(AUX) $(GOOGLE_BUFFER) $(EXTRAFLAGS)
>>>>>>> 7700c18a4b07750c159e258fbecc3311f3dedfe4


.PHONY:
	clean
clean:
	rm -rf *.o *.cpp~ *.hpp~ *# main
