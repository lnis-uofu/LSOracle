# This image gathers contains all the dependencies needed to build and
# run tests on LSOracle. The primary purpose is to provide a container to run
# Github actions in.

FROM debian:testing AS runtime
RUN apt-get update && \
    apt-get install -y --no-install-recommends\
        libboost-program-options1.74.0 \
        libreadline8 \
        python3-pip && \
    apt-get clean

RUN pip3 install numpy && pip3 install matplotlib

FROM runtime AS builder

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
	bison \
        flex && \
    apt-get clean
ENV CC=gcc-8
ENV CXX=g++-8

# RUN git clone https://github.com/berkeley-abc/abc.git &&
#     cd abc && make && \
#     cp abc/abc /usr/local/bin/abc

FROM builder AS compiled
RUN mkdir -p /LSOracle/build
COPY . /LSOracle/
WORKDIR /LSOracle
RUN cmake -B build -D CMAKE_BUILD_TYPE=RELEASE . && cmake --build build -j$(nproc)

FROM runtime AS dist
RUN mkdir -p /LSOracle
COPY --from=compiled /LSOracle /LSOracle
COPY --from=compiled /LSOracle/build/lib/kahypar/lib/libkahypar.so /usr/lib/x86_64-linux-gnu/
ENV PATH="/LSOracle/build/core:${PATH}"
CMD ["lsoracle"]
