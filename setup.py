from glob import glob
from setuptools import setup, Extension

version = "0.5.0"


def readme():
    with open("README.rst") as f:
        return f.read()


def make_limited_api_macro(version):
    s = 0
    step = 8
    pos = step * 3
    for i in version.split("."):
        s += int(i) << pos
        pos -= step
    return s


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
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: Python :: 3.14",
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
    setup_requires=["Cython"],
    install_requires=["gevent"],
    ext_modules=[
        Extension(
            "greenify",
            sources,
            include_dirs=include_dirs,
            libraries=libraries,
            define_macros=[
                ("Py_LIMITED_API", make_limited_api_macro("3.9")),
            ],
            py_limited_api=True,
        )
    ],
)
