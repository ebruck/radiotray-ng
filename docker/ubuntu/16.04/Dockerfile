FROM ubuntu:16.04

RUN set -ex; \
    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends ca-certificates gcc g++ curl wget git openssh-client \
                   lsb-release \
                   libcurl3-openssl-dev \
                   libjsoncpp-dev \
                   libxdg-basedir-dev \
                   libnotify-dev \
                   libboost-filesystem-dev \
                   libgstreamer1.0-dev \
                   libappindicator3-dev \
                   libboost-log-dev \
                   libboost-program-options-dev \
                   libgtk-3-dev \
                   libnotify-dev \
                   lsb-release \
                   libbsd-dev \
                   libncurses5-dev \
                   libglibmm-2.4-dev \
                   libwxgtk3.0-dev \
                   libwxgtk3.0-0v5 \
        && wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.14.3/cmake-3.14.3-Linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local \
        && rm -rf /var/lib/apt/lists/*
