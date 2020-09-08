#!/bin/bash
set -e

for platform in "arm" "thumb" "aarch64"; do
  for opt_level in "0" "3" "s"; do
    for sanitizers in "ON" "OFF"; do
      for lto in "ON" "OFF"; do
        rm -f CMakeCache.txt
        cmake . -DBUILD_PLATFORM=${platform} -DOPT_LEVEL=${opt_level} -DSANITIZERS=${sanitizers} -DLTO=${lto}
        make make_demos
        lit demos/ -a
        make clean
      done
    done
  done
done
