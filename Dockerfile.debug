FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
  && apt-get install -yq --no-install-recommends \
    g++ \
    gcc \
    gdb \
    gdbserver \
    libcurl4-openssl-dev \
    libjansson-dev \
    libssl-dev \
    libsqlite3-dev \
    make \
    sendemail \
  && rm -rf /var/lib/apt/lists/*