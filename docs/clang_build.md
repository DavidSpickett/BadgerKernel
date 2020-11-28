# Building with Clang

## Install clang-11

On Linux you can install from https://apt.llvm.org/.

Your mileage may vary with anything older than 11.

## Patch CMake config

(this is for AArch64 but you can adapt it for other architectures)

```
diff --git a/CMakeLists.txt b/CMakeLists.txt
index b7ab371..20dc81d 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -25 +25 @@ elseif( BP_LOWER STREQUAL "aarch64" )
-  set( PLATFORM     "-mcpu=cortex-a57 -mgeneral-regs-only" )
+  set( PLATFORM     "-target aarch64-none-elf --sysroot=<...>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf/aarch64-none-elf -mcpu=cortex-a57 -mgeneral-regs-only" )
@@ -61,2 +61,2 @@ endif()
-set( CMAKE_C_COMPILER "${PREFIX}gcc" )
-set( CMAKE_CXX_COMPILER "${PREFIX}g++" )
+set( CMAKE_C_COMPILER "clang-11" )
+set( CMAKE_CXX_COMPILER "clang++-11" )
@@ -83 +83 @@ endif()
-set( CFLAGS "${CFLAGS} -ffreestanding -nostdlib -fno-common" )
+set( CFLAGS "${CFLAGS} -ffreestanding -nostdlib -fno-common -Wno-error=unused-command-line-argument" )
```

* Add `-target` option set to `aarch64-none-elf`. (the prefix of the gcc we would normally use)
* Add `--sysroot` pointing to the `aarch64-none-elf` folder **within** the normal gcc toolchain.
* Change C/C++ compiler name to clang-11.
* Downgrade `unused-command-line-argument` error to a warning.
  (because we use the C compiler to assemble and in this situation clang is more strict than gcc)

## Symlink Include Path

For reasons unknown Clang expects to find includes in `<sysroot>/usr/include`.

(see https://askubuntu.com/questions/947954/wrong-default-include-directories-for-clang-cross-compile/999925#999925)

```
<sysroot path>$ ln -s $(pwd)/include/ $(pwd)/usr/include
```

## Building

Configure and build as normal.