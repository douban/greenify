greenify_
=========

greenify_ can make Python extension modules having network operations in C
code to be compatible with gevent_.

It includes two components:

* A dynamic link library named ``libgreenify``, exporting gevent_ friendly
  network functions like ``green_connect``, ``green_recv``, etc.  An extension
  module should use these functions instead of ``connect`` and ``recv``
  provided by libc;

* A Python extension module named ``greenify`` to provide a function to
  activate these green functions.


Install from source
-------------------

``libgreenify`` is installed using CMake_::

  cmake -G 'Unix Makefiles' .

``greenify`` module is installed using setuptools or pip::

  python setup.py install

or::

  pip install git+git://github.com/douban/greenify.git#egg=greenify


Getting started by greenify'ing MySQL-Python
--------------------------------------------

A common headache of gevent_ users is that the MySQL-Python_ module, which is
the de facto standard MySQL driver for Python, will block all running
greenlets when executing a query.  This is because its network operations
occur in libmysqlclient, a C library it depends on.

There are several solutions for this problem, with drawbacks for each:

* pymysql_ is a pure Python MySQL driver, so it can use gevent monkey patched
  ``socket`` module.  But its performance is not as good as MySQL-Python, due
  to the slowness of Python code.  And it is not 100% compatible with
  MySQL-Python, although it claims itself a drop-in replacement.

* ultramysql_ is a fast MySQL driver written in C/C++ and compatible with
  gevent.  But it is not compatible with MySQL-Python either, and has much
  smaller feature set.
  
* umysqldb_ is trying to provide `DB API 2.0`_ interface by wrapping
  ultramysql.  But due to the feature lackness of ultramysql, it does not
  implement many of the MySQL-Python specific interfaces.

The main trouble met by these attempts is compatibility with MySQL-Python, so,
why not re-use MySQL-Python's code and make it gevent compatible?  With
greenify, this is possible as long as we replace all the network functions in
libmysqlclient library to corresponding green ones.  Thanks for CMGS_, who
contributes a step-by-step guide to do so:

1. make virtualenv::

    mkvirtualenv test

2. install libgreenify::

    cmake -G 'Unix Makefiles' -D CMAKE_INSTALL_PREFIX=$VIRTUAL_ENV .

3. install greenify::

    export LIBGREENIFY_PREFIX=$VIRTUAL_ENV
    pip install git+git://github.com/douban/greenify.git#egg=greenify

4. modify mysql-connector-c_ manually, or get it from https://github.com/CMGS/mysql-connector-c

5. install mysql-connector-c_ like::

    cmake -G 'Unix Makefiles' -D GREENIFY_INCLUDE_DIR='$VIRTUAL_ENV_INCLUDE' -D GREENIFY_LIB_DIR='$VIRTUAL_ENV_LIB' -D WITH_GREENIFY=1 -D CMAKE_INSTALL_PREFIX='$VIRTUAL_ENV'

6. modify MySQL-Python_ manually, or get it from https://github.com/CMGS/MySQL-python. Let setup.py know where is mysql-connector-c_ which was modified. I'm not sure the new-version branch is working good or not.

7. in your app, active greenify_ before initiate environment::

    import greenify
    greenify.greenify()

8. enjoy the new world.


Thread Safety
-------------

Once activated, the green C functions will, on potentially blocking operation,
pass control to gevent's main event loop, which may switch to other ready
greenlet which is also in one of the green functions.  So, make sure your C
code can handle this kind of execution pause and resume.  A thread safe
program usually is ready for greenify, but remember that all the switches
happen in a single thread.


License
-------

greenify_ is written and maintained by `douban`_ and is licensed under New BSD license.


.. _gevent: http://www.gevent.org
.. _greenify: https://github.com/douban/greenify
.. _douban: http://www.douban.com
.. _mysql-connector-c: http://dev.mysql.com/downloads/connector/c/
.. _MySQL-Python: https://github.com/farcepest/MySQLdb1
.. _SQLALchemy: http://www.sqlalchemy.org/
.. _CMake: http://www.cmake.org/
.. _pymysql: http://www.pymysql.org/
.. _ultramysql: https://github.com/esnme/ultramysql
.. _umysqldb: https://github.com/hongqn/umysqldb
.. _DB API 2.0: http://legacy.python.org/dev/peps/pep-0249/
.. _CMGS: https://github.com/CMGS
