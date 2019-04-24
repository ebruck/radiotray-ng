#!/bin/bash
set -xe

docker build -t radiotrayng/circleci:ubuntu-18.04 - <ubuntu/18.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-18.04

docker build -t radiotrayng/circleci:ubuntu-18.10  - <ubuntu/18.10/Dockerfile
docker push radiotrayng/circleci:ubuntu-18.10

docker build -t radiotrayng/circleci:ubuntu-19.04 - <ubuntu/19.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-19.04
