#!/bin/bash

set -xe

docker build -t radiotrayng/circleci:ubuntu-22.04 - <ubuntu/22.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-22.04

docker build -t radiotrayng/circleci:ubuntu-24.04 - <ubuntu/24.04/Dockerfile
docker push radiotrayng/circleci:ubuntu-24.04

docker build -t radiotrayng/circleci:ubuntu-25.10 - <ubuntu/25.10/Dockerfile
docker push radiotrayng/circleci:ubuntu-25.10
