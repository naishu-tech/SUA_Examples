import asyncio
from common.model import StreamRequest
from tools.logging import logging
from core.client import SocketClient
from drp.router import Router
from parser.parser import Parser


class Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port

        self.router = Router()
        self.parser = Parser()

        asyncio.run(self.run_server())

    async def run_server(self):
        server = await asyncio.start_server(self.client_handler, self.host, self.port)
        async with server:
            await server.serve_forever()

    async def client_handler(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        async with SocketClient(reader, writer) as client:
            while True:
                try:
                    received_bytes = await client.receive_bytes()
                    if received_bytes is None:
                        break
                    request_obj = self.parser.parse_request(received_bytes, client)
                    client_info = writer.get_extra_info('peername')
                    if not isinstance(request_obj, StreamRequest):
                        logging.info(f"Request from: {client_info}, content is: {request_obj.scpi}")
                    else:
                        logging.info(f"Request from: {client_info}")
                    fn = self.router.get_executive_func(request_obj)
                    res = await fn(request_obj)
                    # 这里可以对响应包装
                    response = res
                    if not response:
                        continue
                    await client.send_bytes(response)
                except Exception as e:
                    logging.error(e)


def start_tcp_server(host: str, port: int):
    Server(host, port)


if __name__ == '__main__':
    start_tcp_server('127.0.0.1', 8001)
