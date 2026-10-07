#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -eq 0 ]]; then
  echo "usage: $0 package [package ...]" >&2
  exit 2
fi

apt_options=(
  -o Acquire::Retries=2
  -o Acquire::http::Timeout=90
  -o Acquire::https::Timeout=90
)

for attempt in 1 2 3; do
  echo "Installing apt packages (attempt ${attempt}/3): $*"
  if timeout --kill-after=10s 4m sudo apt-get "${apt_options[@]}" update &&
     timeout --kill-after=10s 4m sudo apt-get "${apt_options[@]}" install -y "$@"; then
    exit 0
  fi
  if [[ "$attempt" -lt 3 ]]; then
    sleep "$((attempt * 10))"
  fi
done

echo "::error::apt package installation failed after three bounded attempts"
exit 1
