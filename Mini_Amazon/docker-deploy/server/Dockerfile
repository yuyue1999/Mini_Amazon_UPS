FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get install -y netcat postgresql postgresql-contrib libpqxx-dev && \
    apt-get install -y autoconf automake libtool curl make g++ unzip wget
    # apt-get install -y protobuf-compiler libprotobuf-dev libprotoc-dev

RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.20.0/protobuf-cpp-3.20.0.zip && \
    unzip protobuf-cpp-3.20.0.zip && \
    cd protobuf-3.20.0 && \
    ./configure && \
    make && \
    make install && \
    ldconfig

ENV PKG_CONFIG_PATH /usr/local/lib/pkgconfig
ENV LD_LIBRARY_PATH /usr/local/lib

WORKDIR /app
COPY . .

EXPOSE 5688
EXPOSE 8873

RUN chmod u+r+x run.sh
