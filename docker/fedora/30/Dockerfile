FROM fedora:30

RUN set -ex; \
    dnf -y update \
    && dnf -y install git openssh-clients rpm-build redhat-lsb cmake libcurl-devel boost-devel wxGTK3-devel jsoncpp-devel gstreamer1-devel libxdg-basedir-devel libbsd-devel libappindicator-gtk3-devel libnotify-devel glibmm24-devel \
    && dnf clean all
