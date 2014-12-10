import sys
from glob import glob
from setuptools import setup, Extension
# setuptools DWIM monkey-patch madness: http://dou.bz/37m3XL

if 'setuptools.extension' in sys.modules:
    m = sys.modules['setuptools.extension']
    m.Extension.__dict__ = m._Extension.__dict__

include_dirs = ["include"]
sources = glob("*.pyx") + glob("src/*.c")
libraries = ["dl"]

setup(
    name="greenify",
    version='0.2',
    setup_requires=['setuptools_cython', 'Cython >= 0.18'],
    ext_modules=[
        Extension('greenify', sources, include_dirs=include_dirs,
                  libraries=libraries)
    ]
)
