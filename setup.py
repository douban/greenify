import os
from setuptools import setup, Extension
from Cython.Distutils import build_ext

include_dirs = ['.']
library_dirs = ['.']

if 'LIBGREENIFY_PREFIX' in os.environ:
    include_dirs.append(os.path.join(os.environ['LIBGREENIFY_PREFIX'],
                                     'include'))
    library_dirs.append(os.path.join(os.environ['LIBGREENIFY_PREFIX'], 'lib'))


setup(name='greenify',
      version='0.1',
      ext_modules=[Extension('greenify', ['greenify.pyx'],
                             include_dirs=include_dirs,
                             library_dirs=library_dirs,
                             libraries=['greenify'])],
      cmdclass = {'build_ext': build_ext},
     )
