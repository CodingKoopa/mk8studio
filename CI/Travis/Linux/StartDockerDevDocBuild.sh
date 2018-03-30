#!/bin/sh -ex

# Starts a Docker container and runs the developer documentation build script.

# The Docker container needs to be able to access the project root, to run the build script.
export PROJECT_ROOT_MOUNT_POINT="/mk8studio"
# In the Ubuntu 18.04 image installed by InstallDeps.sh, run the build script "Build.sh".
docker run -e PROJECT_ROOT_MOUNT_POINT -v $(pwd):${PROJECT_ROOT_MOUNT_POINT} ubuntu:18.04 \
  /bin/bash -ex "${PROJECT_ROOT_MOUNT_POINT}/CI/Travis/Linux/BuildDevDocs.sh"
