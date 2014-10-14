from setuptools import setup
from Cython.Build import cythonize

setup(
    name = "greenify",
    version='0.2',
    ext_modules = cythonize('*.pyx'),
)
