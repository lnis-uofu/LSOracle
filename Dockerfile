# This image gathers contains all the dependencies needed to build and
# run tests on LSOracle. The primary purpose is to provide a container to run
# Github actions in.

FROM ubuntu:latest

RUN export DEBIAN_FRONTEND=noninteractive; \
    export DEBCONF_NONINTERACTIVE_SEEN=true; \
    echo 'tzdata tzdata/Areas select Etc' | debconf-set-selections; \
    echo 'tzdata tzdata/Zones/Etc select UTC' | debconf-set-selections; \
    apt-get update && \
    apt-get install -y tzdata \
        gcc-8 \
        g++-8 \
        cmake \
        libboost-all-dev \
        libreadline-dev \
        lcov \
        gcovr \
        git \
        python3-pip && \
    apt-get clean

RUN pip3 install numpy && pip3 install matplotlib

RUN git clone https://github.com/berkeley-abc/abc.git &&
    cd abc && make && \
    cp abc/abc /usr/local/bin/abc
