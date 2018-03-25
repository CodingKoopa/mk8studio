#!/bin/sh -ex

# Builds the project. This script is meant to be ran from an Ubuntu 18.04 Docker container, started
# from "StartDockerBuild.sh"

# Enter the project root directory.
cd ${PROJECT_ROOT_MOUNT_POINT}

# Update the system.
apt-get update
# Install build dependencies. This is not done in "InstallDeps.sh" because they would be lost after
# the "install" TravisCI step. The dependencies are:
# - build-essential   GCC and other compiling utilities.
# - qtbase5-dev       Base Qt libraries.
# - qttools5-dev      Qt utilities such as uic for compiling UI files.
# - qt5-default       Sets Qt5 as the default Qt version. Needed for qmake to run.
apt-get install -y build-essential qtbase5-dev qttools5-dev qt5-default

# Make and enter a build directory.
mkdir Build && cd Build
# Generate a Makefile from the Qt PRO project file.
qmake ../Source/
# BUILD IT
make -j4
