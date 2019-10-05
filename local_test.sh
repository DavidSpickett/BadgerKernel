set -e
for platform in "arm" "thumb" "aarch64" "linux"; do
  for opt_level in "0" "3" "s"; do
    for sanitizers in "ON" "OFF"; do
      for lto in "ON" "OFF"; do
        cmake . -DBUILD_PLATFORM=${platform} -DOPT_LEVEL=${opt_level} -DSANITIZERS=${sanitizers} -DLTO=${lto}
        make demos
        lit demos/
        make clean
      done
    done
  done
done
