#!/usr/bin/env bash
set -euo pipefail
set -x

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

for version in 22.04 24.04 26.04; do
    image="radiotrayng/circleci:ubuntu-${version}"
    docker build -t "$image" "ubuntu/${version}"
    docker push "$image"
done
