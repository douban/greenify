greenify_
=========

greenify_ is a library, which can make networking library compatible with gevent_.

Features include:

* Ability to use 3rd party c modules written for standard blocking sockets.

greenify_ is written and maintained by `douban`_ and is licensed under MIT license.

installing from github
----------------------

To install the latest development version:

  pip install git+git://github.com/douban/greenify.git#egg=greenify

getting started
---------------

make MySQL-Python compatible with gevent_.

1. mkvirtualenv test

2. pip install git+git://github.com/douban/greenify.git#egg=greenify

3. modify mysql-connector-c_ manually, or get it from https://github.com/CMGS/mysql-connector-c

4. install mysql_connector-c_ like:
  cmake -G 'Unix Makefiles' -D GREENIFY_INCLUDE_DIR='$VIRTUAL_ENV_INCLUDE' -D GREENIFY_LIB_DIR='$VIRTUAL_ENV_LIB' -D WITH_GREENIFY=1 -D CMAKE_INSTALL_PREFIX='$VIRTUAL_ENV'

5. modify MySQL-Python_ manually, or get it from https://github.com/CMGS/MySQL-python. Let setup.py know where is mysql-connector-c_ which was modified. I'm not sure the new-version branch is working good or not.

6. in your app, active greenify_ before initiate environment:
   import greenify
   greenify.greenify()

7. enjoy the new world.

be careful
----------

because gevent_ will make your app run like **multi-thread**, so you must care about the thread safe. I suggest using pooling for manage connections. In our program we use pool_ which is 'split from SQLALchemy_'.

.. _gevent: http://www.gevent.org
.. _greenify: https://github.com/douban/greenify
.. _douban: http://www.douban.com
.. _mysql_connector-c: http://dev.mysql.com/downloads/connector/c/
.. _MySQL-Python: https://github.com/farcepest/MySQLdb1
.. _pool: https://github.com/CMGS/pool
.. _SQLALchemy: http://www.sqlalchemy.org/
