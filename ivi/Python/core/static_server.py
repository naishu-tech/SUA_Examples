from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler


class StaticServer(ThreadingHTTPServer):
    ...


def start_static_server(ip: str, port: int, directory: str):
    class StaticHandler(SimpleHTTPRequestHandler):
        def __init__(self, *args, **kwargs):
            super(StaticHandler, self).__init__(*args, directory=directory, **kwargs)

    server = StaticServer((ip, port), StaticHandler)
    server.serve_forever()
