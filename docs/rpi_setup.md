# Set up Raspberry Pi 4 to run BK

To run BK on Raspberry Pi, we need several steps to set up your hardware.

## Pre-requisites

- Raspberry Pi 4
- TTL to USB cable
- micro SD card
- USB type A to type C cable (or power cable)

## Format SD card

To boot from an SD card, we need to install Raspberry Pi firmware.

The simplest way to do it is to use [Raspberry Pi Imager](https://www.raspberrypi.org/software/).

You can select `Raspberry Pi OS Lite (32 bit)` image and click write to start the installation.

After formatting the SD card, enter the boot section of the card, append the following lines to `config.txt`.

```
core_freq_min=500
enable_uart=1
```

## Connect to UART

Connect the TTL cable to the corresponding pins on Raspberry Pi:
- `GND` to `6` pin
- `RX` to `8` pin
- `TX` to `10` pin

You can check the pinout at [UART at Raspberry Pi GPIO Pinout](https://pinout.xyz/pinout/uart).

Then we can power up the Raspberry Pi and open the UART output.
If it boots successfully, we will see the output like the following:

```shell
<...>

[    3.176335] Segment Routing with IPv6
[    3.214711] systemd[1]: systemd 241 running in system mode. (+PAM +AUDIT +SELINUX +IMA +APPARMOR +SMACK +SYSVINIT +UTMP +LIBCRYPTSETUP +GCRYPT +GNUTLS +ACL +XZ +LZ4 +SECCOMP +BLKID +ELFUTILS +KMOD -IDN2 +IDN -PCRE2 default-hierarchy=hybrid)
[    3.237254] systemd[1]: Detected architecture arm64.
[    3.267411] systemd[1]: Set hostname to <raspberrypi>.
[    3.828087] random: systemd: uninitialized urandom read (16 bytes read)
[    3.846401] random: systemd: uninitialized urandom read (16 bytes read)
[    3.854305] systemd[1]: Listening on Journal Socket.
[    3.860399] random: systemd: uninitialized urandom read (16 bytes read)
[    3.872492] systemd[1]: Mounting Kernel Debug File System...
[    3.883067] systemd[1]: Starting Create list of required static device nodes for the current kernel...
[    3.894427] systemd[1]: Created slice User and Session Slice.
[    3.901770] systemd[1]: Listening on fsck to fsckd communication Socket.
[    3.909961] systemd[1]: Listening on Syslog Socket.
[    3.920163] systemd[1]: Starting Set the console keyboard layout...

Raspbian GNU/Linux 10 raspberrypi ttyS0

raspberrypi login:
```

We now know that the setting and the UART works. In the next step, we will replace Raspberry Pi OS with BK image.

## Boot BK

In the beginning, we need to compile an executable

```shell
$ cmake . -DBUILD_PLATFORM=raspi4
$ make yielding
```

Translate the ELF file into an image.

```shell
$ aarch64-none-elf-objcopy -O binary demos/yielding/yielding kernel8.img
```

Now, plug in the SD card to your computer, delete all `kernel*.img` in the boot section, copy `kernel8.img` we made before into that section.

Then plug the SD into the Raspberry Pi and power up.
If it works, you will see the output just like the one on QEMU.

```
Thread            0: working
Thread            0: yielding
Thread  <scheduler>: scheduling new thread
Thread  <scheduler>: next thread chosen
Thread            1: yielding
Thread  <scheduler>: scheduling new thread
Thread  <scheduler>: next thread chosen
Thread            0: resuming
Thread            0: working
Thread            0: yielding
Thread  <scheduler>: scheduling new thread
Thread  <scheduler>: next thread chosen
Thread            1: resuming
Thread            1: yielding
Thread  <scheduler>: scheduling new thread
Thread  <scheduler>: next thread chosen
Thread            0: resuming
Thread            0: working
Thread            0: yielding
Thread  <scheduler>: scheduling new thread
Thread  <scheduler>: next thread chosen
Thread            1: resuming
Thread            1: working
Thread            1: exiting
Exiting kernel
```
