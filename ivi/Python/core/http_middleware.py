from starlette.middleware.base import BaseHTTPMiddleware
from starlette.requests import Request
from starlette.responses import Response

from drp.router import Router
from parser.parser import Parser
from tools.logging import logging


class AWGHttpMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        if str(request.url).replace(str(request.base_url), '').lower() != 'scpi' or request.method.lower() == 'options':
            response = await call_next(request)
            return response
        _request_data = await request.form()
        f = _request_data.get('file', None)
        scpi = _request_data.get('scpi', None)
        if f is not None:
            request_data = {'scpi': _request_data.get('scpi'), 'file': f}
        elif scpi is not None:
            request_data = {'scpi': _request_data.get('scpi')}
        else:
            _request_data = await request.json()
            request_data = _request_data
        if 'scpi' not in request_data:
            response = "SCPI command not found"
            return Response(response, status_code=400)
        parser = Parser()
        router = Router()

        try:
            scpi_request = parser.parse_request(request_data, request)
            # from parser.parser import SCPIRequest
            # if isinstance(scpi_request, SCPIRequest):
                # logging.info(f"SCPI: {scpi_request.full_scpi=} Param:{scpi_request.param=}")
            fn = router.get_executive_func(scpi_request)
            res = await fn(scpi_request)
            response = res.decode()
            status = 200
        except Exception as e:
            logging.error(e)
            response = (str(e))
            status = 400
        return Response(response, status_code=status)


