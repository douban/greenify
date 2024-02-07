from glob import glob
from setuptools import setup, Extension
import functools
import inspect
from setuptools.command.build_ext import build_ext
from Cython.Build import cythonize

version = "0.4.1"

COMPILER_PREARGS = [
    "-DDYNAMIC_ANNOTATIONS_ENABLED=1",
    "-g3",
    "-Og",
    "-Wall",
    "-march=x86-64",
    "-mtune=generic",
    "-pipe",
    "-Wp,-D_FORTIFY_SOURCE=2",
    "-Werror=format-security",
    "-fPIC",
]
COMPILER_FLAGS = [
    "-fno-strict-aliasing",
    "-fno-exceptions",
    "-fno-rtti",
    "-Wall",
    "-DMC_USE_SMALL_VECTOR",
    "-DDEBUG",
]
LINKER_FLAGS = [
    "-shared",
    "-g3",
    "-Wl,-O0,--sort-common,--as-needed,-z,relro,-z,now"
]

# https://shwina.github.io/custom-compiler-linker-extensions/
class BlankBuildExt(build_ext):
    def build_extensions(self):
        orig = self.compiler.compile

        @functools.wraps(orig)
        def prearg_compile(*args, **kwargs):
            bound = inspect.Signature.from_callable(orig).bind(*args, **kwargs)
            bound.apply_defaults()
            bound.arguments["extra_preargs"] = COMPILER_PREARGS
            return orig(*bound.args, **bound.kwargs)

        self.compiler.compile = prearg_compile

        self.compiler.set_executable("compiler_so", "g++")
        #self.compiler.set_executable("compiler_cxx", "gcc")
        self.compiler.set_executable("linker_so", "g++")
        build_ext.build_extensions(self)


def readme():
    with open("README.rst") as f:
        return f.read()


include_dirs = ["include"]
sources = glob("*.pyx") + glob("src/*.c")
libraries = ["dl"]

setup(
    name="greenify",
    version=version,
    description="Make C module compatible with gevent at runtime.",
    long_description=readme(),
    platforms=["Linux"],
    classifiers=[
        "Intended Audience :: Developers",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C",
        "Programming Language :: Cython",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: Implementation :: CPython",
        "Development Status :: 5 - Production/Stable",
        "Operating System :: POSIX :: Linux",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: BSD License",
        "Topic :: Software Development :: Libraries",
    ],
    author="Zhongbo Tian",
    author_email="tianzhongbo@douban.com",
    url="https://github.com/douban/greenify",
    download_url="https://github.com/douban/greenify/archive/%s.tar.gz" % version,
    setup_requires=["Cython >= 0.18, < 3"],
    install_requires=["gevent"],
    cmdclass={"build_ext": BlankBuildExt},
    zip_safe=False,
    ext_modules=cythonize([
        Extension(
            "greenify",
            sources,
            include_dirs=include_dirs,
            libraries=libraries,
            extra_compile_args=COMPILER_FLAGS,
            extra_link_args=LINKER_FLAGS,
        )
    ], gdb_debug=True),
)
