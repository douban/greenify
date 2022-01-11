#!/bin/bash

set -e
cd tests/http_head
python fake_slow_http_server.py&
pid=$!
pip install .
python test.py
kill $pid || true
