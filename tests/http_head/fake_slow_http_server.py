# coding: utf-8

import time

try:
    from SimpleHTTPServer import SimpleHTTPRequestHandler
    from SocketServer import TCPServer
except ImportError:
    from http.server import SimpleHTTPRequestHandler
    from socketserver import TCPServer


PORT = 0x2304
BLOCKING_SECONDS = 10  # seconds


class Server(TCPServer):
    allow_reuse_address = True


class Handler(SimpleHTTPRequestHandler):
    def do_HEAD(self):
        time.sleep(BLOCKING_SECONDS)
        return SimpleHTTPRequestHandler.do_HEAD(self)


if __name__ == "__main__":
    httpd = Server(("", PORT), Handler)
    try:
        httpd.serve_forever()
    except Exception:
        httpd.server_close()
