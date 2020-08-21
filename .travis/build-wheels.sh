#!/bin/bash
set -e -x

PACKAGE=$1
PYENV=$2


# Install a system package required by our library
# Compile wheels
for PYBIN in /opt/python/${PYENV}*/bin; do
    ${PYBIN}/pip install -r /io/req.txt
    ${PYBIN}/pip wheel /io/ -w wheelhouse/
done

# Bundle external shared libraries into the wheels
for whl in wheelhouse/${PACKAGE}*.whl; do
    auditwheel repair $whl --plat $PLAT -w /io/dist/
done
