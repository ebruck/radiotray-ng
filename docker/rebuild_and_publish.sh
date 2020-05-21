#!/bin/bash

set -xe

docker build -t radiotrayng/circleci:ubuntu-16.04 - <ubuntu/16.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-16.04

docker build -t radiotrayng/circleci:ubuntu-18.04 - <ubuntu/18.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-18.04

docker build -t radiotrayng/circleci:ubuntu-20.04 - <ubuntu/20.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-20.04

docker build -t radiotrayng/circleci:fedora-31 - <fedora/31/Dockerfile
docker push radiotrayng/circleci:fedora-31
