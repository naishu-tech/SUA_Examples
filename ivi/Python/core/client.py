import asyncio
from typing import Union


class Client:
    def __init__(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        self.reader = reader
        self.writer = writer

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        self.close()
        if exc_type is ConnectionResetError:
            return True

    def close(self):
        self.writer.close()

    async def read(self, recv_len):
        pass

    async def write(self, data):
        pass


class SocketClient(Client):
    def __init__(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        super().__init__(reader, writer)

    def close(self):
        super().close()

    async def read(self, recv_len):
        return await self.reader.read(recv_len)

    async def write(self, data):
        self.writer.write(data)
        await self.writer.drain()  # 刷写缓冲区

    async def receive_bytes(self) -> Union[bytes, None]:
        recv_data = bytes()
        while True:
            try:
                data = await self.read(1)
                if not data:
                    # 如果data为None
                    break
                recv_data += data
                if data == b"\n":
                    return recv_data
                if data == b'':
                    return None
            except Exception as e:
                break

    async def receive_bytes_data(self, recv_len) -> Union[bytes, None]:
        recv_data = bytes()
        while True:
            try:
                data = await self.read(recv_len)
                if not data:
                    # 如果data为None
                    break
                recv_data += data
                recv_len -= len(data)
                if recv_len <= 0:
                    return recv_data
            except Exception as e:
                break

    async def send_bytes(self, bytes_data):
        try:
            await self.write(bytes_data)
        except Exception as e:
            raise e


class StreamClient(SocketClient):
    """鸭子类型，具有WebSocket对象有的功能"""

    def __init__(self, client: Client):
        super().__init__(client.reader, client.writer)

    async def accept(self):
        pass
