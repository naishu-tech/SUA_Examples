import uvicorn
from uvicorn.config import LOGGING_CONFIG
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from core.http_middleware import AWGHttpMiddleware
from drp.router import Router
from tools.logging import logging

app = FastAPI()

LOGGING_CONFIG["formatters"]["default"]["fmt"] = logging.base_formatter_str  # 更改uvicorn日志格式
app.add_middleware(AWGHttpMiddleware)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


def register_websocket_router():
    router = Router()
    for row in router.websocket_routers:
        path, fn = row
        app.add_websocket_route(path, fn)


def start_http_server(ip: str, port: int):
    uvicorn.run(app, host=ip, port=port)


if __name__ == '__main__':
    start_http_server('0.0.0.0', 8000)
