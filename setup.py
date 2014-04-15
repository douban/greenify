from setuptools import setup
from Cython.Build import cythonize

setup(
    name = "greenify",
    version='0.1',
    ext_modules = cythonize('*.pyx'),
)
