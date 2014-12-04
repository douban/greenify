.PHONY: build

all: test

build:
	python setup.py build_ext --inplace

test: build
	python test.py
