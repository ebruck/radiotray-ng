#!/bin/bash

set -xe

docker build -t radiotrayng/circleci:ubuntu-16.04 - <ubuntu/16.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-16.04

docker build -t radiotrayng/circleci:ubuntu-18.04 - <ubuntu/18.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-18.04

docker build -t radiotrayng/circleci:ubuntu-19.04 - <ubuntu/19.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-19.04

docker build -t radiotrayng/circleci:fedora-30 - <fedora/30/Dockerfile
docker push radiotrayng/circleci:fedora-30
