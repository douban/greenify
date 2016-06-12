# coding: utf-8

# greenify
import greenify
greenify.greenify()

# python patch
import gevent
import gevent.monkey
gevent.monkey.patch_all()

import sys
import time
import mod_http_head
assert greenify.patch_lib(mod_http_head.__file__.encode('ascii'))
import fake_slow_http_server

stack = []


def c_http_head_check(addr):
    stack.append(('begin', addr, 'c'))
    print('%.5f head %s begin' % (time.time(), addr), file=sys.stderr)
    ret = mod_http_head.http_head(*addr)
    print('%.5f head %s end' % (time.time(), addr), file=sys.stderr)

    stack.append(('end', addr, 'c'))
    assert ret == 1


def python_http_head_check(addr):
    import http.client as httplib
    stack.append(('begin', addr, 'python'))
    print('%.5f head %s begin' % (time.time(), addr), file=sys.stderr)

    conn = httplib.HTTPConnection(*addr)
    conn.request("HEAD", "/")
    resp = conn.getresponse()
    status_code = resp.status
    print('%.5f head %s end' % (time.time(), addr), file=sys.stderr)

    stack.append(('end', addr, 'python'))
    assert 200 <= status_code < 400


def sleeper():
    stack.append(('begin', 'sleeper'))
    print('%.5f sleeper begin' % time.time(), file=sys.stderr)

    time.sleep(fake_slow_http_server.BLOCKING_SECONDS/2)
    print('%.5f sleeper end' % time.time(), file=sys.stderr)

    stack.append(('end', 'sleeper'))


def main():
    global stack
    local_addr = ("localhost", str(fake_slow_http_server.PORT))
    test_sites = (local_addr, ("msn.com", "80"), ("douban.com", "80"),
                  ("douban.fm", "80"), ("qq.com", "80"))
    for fn in [python_http_head_check, c_http_head_check]:
        stack = []
        print('test %s' % fn.__name__)
        t0 = time.time()
        workers = [
            gevent.spawn(fn, addr)
            for addr in test_sites
        ]
        workers.append(gevent.spawn(sleeper))

        gevent.joinall(workers)
        time_elapsed = time.time() - t0
        print('done in %.5fs' % (time_elapsed))
        print()
        # HEAD to fake_slow_http_server is expected to
        # be the slowest worker
        assert stack[-1][:2] == ('end', local_addr), \
            "unexpected: %r" % (stack[-1][:2], )
        assert time_elapsed < fake_slow_http_server.BLOCKING_SECONDS * 1.5


if __name__ == '__main__':
    main()
