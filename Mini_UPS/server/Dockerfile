FROM ubuntu:20.04
RUN mkdir /code
ADD . /code/
WORKDIR /code

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
RUN ls 
RUN apt-get update && \
    apt-get install -y make gcc && \
    apt-get install -y python3
RUN apt-get -y install build-essential libpqxx-dev postgresql 
RUN apt-get install -y autoconf automake libtool curl make g++ unzip wget python3


RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.20.0/protobuf-cpp-3.20.0.zip && \
    unzip protobuf-cpp-3.20.0.zip && \
    cd protobuf-3.20.0 && \
    ./configure && \
    make && \
    make install && \
    ldconfig

RUN chmod u+r+x run.sh