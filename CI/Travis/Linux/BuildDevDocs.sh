#!/bin/sh -ex

# Builds developer documentation with Doxygen.

# Enter the documentation directory.
cd Docs
# Build the docmentation, and quit if any errors are encountered.
doxygen
