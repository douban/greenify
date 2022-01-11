import os
import platform
from distutils.core import setup, Extension

virtualenv_path = os.environ.get("VIRTUAL_ENV")

include_dirs = list(filter(None, [os.environ.get("INCLUDE_DIRS")]))
library_dirs = list(filter(None, [os.environ.get("LIBRARY_DIRS")]))

if virtualenv_path:
    include_dirs.append("%s/include" % virtualenv_path)
    library_dirs.append("%s/lib" % virtualenv_path)
    ld_lib_key = "DYLD_LIBRARY_PATH" if platform.platform() == "Darwin" else "LD_LIBRARY_PATH"
    os.environ[ld_lib_key] = "%s/lib" % virtualenv_path

mod_http_head = Extension(
    "mod_http_head",
    sources=["mod_http_head.c"],
    include_dirs=include_dirs,
    library_dirs=library_dirs,
)

setup(
    name="mod_http_head",
    version="1.0",
    description="A demo package http_head greenified",
    ext_modules=[mod_http_head],
)
