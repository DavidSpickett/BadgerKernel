set -e
for platform in "arm" "arm_linux" "thumb" "aarch64"; do
  for opt_level in "0" "3" "s"; do
    for ubsan in "ON" "OFF"; do
      for lto in "ON" "OFF"; do
        cmake . -DBUILD_PLATFORM=${platform} -DOPT_LEVEL=${opt_level} -DUBSAN=${ubsan} -DNATIVE=OFF -DLTO=${lto}
        make demos
        lit demos/
        make clean
      done
    done
  done
done
