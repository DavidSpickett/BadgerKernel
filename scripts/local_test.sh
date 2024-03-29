#!/bin/bash
set -e

for platform in "arm" "thumb" "aarch64" "raspi4"; do
  for opt_level in "0" "3" "s"; do
    for sanitizers in "ON" "OFF"; do
      for lto in "ON" "OFF"; do
        # LTO isn't enabled at O0
        if [[ "${opt_level}_${lto}" != "0_ON" ]]; then
          rm -f CMakeCache.txt
          # Since test.sh is a config generated file not a build generated file, clean will not remove it.
          # shellcheck disable=SC2046
          rm -f $(find demos/ -name test.sh)
          cmake . -G Ninja -DBUILD_PLATFORM=${platform} -DOPT_LEVEL=${opt_level} -DSANITIZERS=${sanitizers} -DLTO=${lto}
          ninja demos
          lit demos/ -a
          ninja clean
        fi
      done
    done
  done
done
