# Ev3-robot-client over WiFi

After cloning, don't forget to initialize the Asio submodule.

## Creating Docker image for Ev3
- Install Docker `sudo apt install docker`.
- Download the Ev3Dev base image `docker pull
  ev3dev/ev3dev-jessie-ev3-base`.
- Tag image `docker tag ev3dev/ev3dev-jessie-ev3-base ev3-base`.
- Start docker `docker run --name="ev3-robot-client" -it -v ~/:/src
  ev3-base`.
- Login as robot `login robot`. Password is `maker`.
- Update package archive `sudo apt-get update`.
- Upgrade packages `sudo apt-get upgrade`.
- Need to install g++ 5 or higher.
  - Add `deb http://ftp.us.debian.org/debian testing main contrib
    non-free` to `/etc/apt/sources.list`.
  - Add the following lines to `/etc/apt/preferences.d/gplusplus`
    - `Package: g++`
    - `Pin: release a=testing`
    - `Pin-Priority: 100`
  - Update `sudo apt-get update`.
  - Upgrade existing packages `sudo apt-get upgrade`.
  - Install `sudo apt-get install -t testing g++`
- Install libraries `sudo apt-get install build-essential cmake
  libprotobuf-dev protobuf-compiler libgflags-dev libgoogle-glog-dev`.
- Copy controller code `cp -r /src/ev3dev/ev3-robot-client ~/`.
- Move into build directory `cd ~/ev3-robot-client/build`.
- Remove desktop build code `rm -r *`.
- Set up to build for Ev3 `cmake -DCMAKE_BUILD_TYPE=Release
  -DBUILD_FOR_EV3=ON .. && make`.
- Go to home directory `cd ~/`.
- Add shell script and specify ip of host server.  Put the following lines in `ev3Client.sh`
  - `#!/bin/bash`
  - `~/ev3-robot-client/build/src/main/ev3Client --hostname="HOST.SERVER.IP.ADDRESS"`
- Make executable `chmod +x ev3Client.sh`.
- Log out of user robot `exit`.
- Exit docker `exit`.
- Commit Docker container `docker commit ev3-robot-client
  ev3-robot-client`
- Navigate to top of working directory `cd ~/ev3dev`.
- Clone Brickstrap `git clone
  https://github.com/ev3dev/brickstrap`.
- Install packages `sudo apt-get install docker-engine
  libguestfs-tools qemu-user-static`.
- If you have never used `libguestfs` before, set it up.
  - `sudo update-guestfs-appliance`
  - `sudo usermod -a -G kvm $USER`
  - `newgrp kvm`
  - `sudo chmod +r /boot/vmlinuz*`
- Create tar from Docker image `./brickstrap/src/brickstrap.sh create-tar
  ev3-robot-client ev3-robot-client.tar`.
- Create image from tar `./brickstrap/src/brickstrap.sh create-image
  ev3-robot-client.tar ev3-robot-client.img`.
- Burn the image to an SD-card using the instructions on
  the [ev3-dev website](http://www.ev3dev.org/docs/getting-started/).
