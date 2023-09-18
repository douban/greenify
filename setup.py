from glob import glob
from setuptools import setup, Extension

version = "0.4.1"


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
    ext_modules=[
        Extension(
            "greenify",
            sources,
            include_dirs=include_dirs,
            libraries=libraries,
        )
    ],
)
