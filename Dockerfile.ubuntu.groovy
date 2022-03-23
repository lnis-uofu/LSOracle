FROM ubuntu:20.10 AS runtime
RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    apt-get install -y --no-install-recommends\
    libreadline8 \
    python3-numpy \
    python3-matplotlib && \
    apt-get clean

FROM runtime AS builder

RUN export DEBIAN_FRONTEND=noninteractive; \
    export DEBCONF_NONINTERACTIVE_SEEN=true; \
    echo 'tzdata tzdata/Areas select Etc' | debconf-set-selections; \
    echo 'tzdata tzdata/Zones/Etc select UTC' | debconf-set-selections; \
    apt-get update && \
    apt-get install -y tzdata \
        gcc-8 g++-8 \
        gcc-9 g++-9 \
        gcc-10 g++-10 \
        clang-8 \
        clang-9 \
        clang-10 \
        clang-11 \
        cmake \
        ccache \
        libboost-program-options-dev \
        libreadline-dev \
        libpython3-dev \
        pybind11-dev \
        lcov \
        gcovr \
        git \
        bison \
        make \
        libpython3.9-dev \
        python3.9-distutils \
        swig \
        flex && \
    apt-get clean
ENV CC=gcc-10 CXX=g++-10
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
WORKDIR /LSOracle
COPY --from=compiled /LSOracle /LSOracle
ENV PATH="/LSOracle/build/core:${PATH}"
CMD ["lsoracle"]
