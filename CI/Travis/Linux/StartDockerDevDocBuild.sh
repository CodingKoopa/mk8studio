#!/bin/sh -ex

# Starts a Docker container and runs the developer documentation build script.

cd Docs

# The Docker container needs to be able to access the project root, to run the build script.
export DOCUMENTATION_MOUNT_POINT="/mk8studio-docs"
# In the Ubuntu 18.04 image installed by InstallDeps.sh, run the build script "Build.sh".
docker run -e DOCUMENTATION_MOUNT_POINT -v $(pwd):${DOCUMENTATION_MOUNT_POINT} ubuntu:18.04 \
  /bin/bash -ex "${PROJECT_ROOT_MOUNT_POINT}/CI/Travis/Linux/BuildDevDocs.sh"
