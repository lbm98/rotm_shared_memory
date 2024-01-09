#!/bin/bash
set -e

if [ "$EUID" -ne 0 ]; then
  echo "Please run this script using sudo"
  exit 1
fi

./network/srsRAN_Project/build/apps/gnb/gnb -c ./configs/gnb_zmq.yaml