#!/bin/bash

BLOCKS_PER_GROUP=""
NUM_GROUPS=""
NUM_INODES=""

usage() {
  echo "Usage: $0 [-g blocks-per-group] [-G number-of-groups] [-N number-of-inodes]"
  echo "Example: $0 -g 32768 -N 100"
  exit 1
}

while [[ $# -gt 0 ]]; do
  case $1 in
    -g)
      if [[ -z "$2" || "$2" == -* ]]; then
        echo "Error: Option $1 requires an argument." >&2
        usage
      fi
      BLOCKS_PER_GROUP="$1 $2"
      shift
      shift
      ;;
    -G)
      if [[ -z "$2" || "$2" == -* ]]; then
        echo "Error: Option $1 requires an argument." >&2
        usage
      fi
      NUM_GROUPS="$1 $2"
      shift
      shift
      ;;
    -N)
      if [[ -z "$2" || "$2" == -* ]]; then
        echo "Error: Option $1 requires an argument." >&2
        usage
      fi
      NUM_INODES="$1 $2"
      shift
      shift
      ;;
    -h|--help)
      usage
      ;;
    -*|--*)
      echo "Unknown option: $1" >&2
      usage
      ;;
    *)
      echo "Unexpected argument: $1" >&2
      usage
      ;;
  esac
done

bash run-tests.sh $BLOCKS_PER_GROUP $NUM_GROUPS $NUM_INODES

exit 0