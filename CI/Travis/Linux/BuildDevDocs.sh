#!/bin/sh -ex

# Builds developer documentation with Doxygen. This script is meant to be ran from an Ubuntu 18.04
# Docker container, started from "StartDockerDevDocBuild.sh"

# Enter the documentation directory.
cd ${PROJECT_ROOT_MOUNT_POINT}/Docs

# Update the system.
apt-get update
# Install documentation build dependencies. This is not done in "InstallDeps.sh" because they would
# be lost after the "install" TravisCI step. The dependencies are:
# - doxygen   The main Doxygen executable.
# - graphviz  The dot tool used for generating diagrams.
apt-get install -y doxygen graphviz

# Build the docmentation, and quit if any errors are encountered.
doxygen
