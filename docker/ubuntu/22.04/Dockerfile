FROM ubuntu:22.04

RUN set -ex; \
    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        ca-certificates gcc g++ make cmake curl wget git openssh-client pkg-config dpkg-dev \
        lsb-release \
        libcurl4-openssl-dev \
        libjsoncpp-dev \
        libxdg-basedir-dev \
        libnotify-dev \
        libboost-filesystem-dev \
        libboost-log-dev \
        libboost-program-options-dev \
        libboost-thread-dev \
        libgstreamer1.0-dev \
        libayatana-appindicator3-dev \
        libgtk-3-dev \
        libbsd-dev \
        libglibmm-2.4-dev \
        libwxgtk3.0-gtk3-dev \
    && rm -rf /var/lib/apt/lists/*
