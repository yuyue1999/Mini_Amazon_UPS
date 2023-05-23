How to use protobuf

1. Download source code from github using this link: protobuf-cpp-3.20.0.zip. I use version 3.20.0(not the latest one.)

2. Run unzip protobuf-cpp-3.20.0.zip to unzip the download file in your VM

3. cd protobuf-cpp-3.20.0

4. Install pre-requisite packages using  sudo apt-get install -y autoconf automake libtool curl make g++

5. Run the following commands ./configure, make, make check(this is an optional one) , sudo make install , sudo ldconfig .

6. compiling: protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/addressbook.proto

7.g++ -o writeExample writeExample.cpp addressbook.pb.cc `pkg-config --cflags --libs protobuf` 
