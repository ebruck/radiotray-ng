#!/bin/bash

set -xe

docker build -t radiotrayng/circleci:ubuntu-18.04 - <ubuntu/18.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-18.04

docker build -t radiotrayng/circleci:ubuntu-20.04 - <ubuntu/20.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-20.04

docker build -t radiotrayng/circleci:ubuntu-22.04 - <ubuntu/22.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-22.04

docker build -t radiotrayng/circleci:fedora-34 - <fedora/34/Dockerfile
docker push radiotrayng/circleci:fedora-34
