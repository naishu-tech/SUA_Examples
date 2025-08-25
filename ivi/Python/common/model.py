"""
模型类。组织数据结构
"""
import queue
import threading
from typing import Dict, Any, Union
from dataclasses import dataclass, field
from enum import Enum
import requests
from pydantic import BaseModel, computed_field, Field, ConfigDict

from common.utils import GlobalData
from core.client import StreamClient
from ivi import VI
from Tools.logging import logging


@dataclass
class NoticePusher:
    index: int = 0
    lock: threading.Lock = threading.Lock()
    message_que: queue.Queue = queue.Queue(maxsize=100)
    client: dict[StreamClient] = field(default_factory=dict)

    def push_message(self, message: str) -> None:
        self.message_que.put(message)

    def add_client(self, client: StreamClient) -> int:
        with self.lock:
            index = self.index
            self.index += 1
            self.client.update({index: client})
        return index

    def del_client(self, index: int) -> None:
        with self.lock:
            self.client.pop(index)

    async def send_message(self) -> None:
        with self.lock:
            try:
                message = self.message_que.get(timeout=1)
                for i in self.client:
                    try:
                        await self.client[i].send_bytes(message.encode('utf-8'))
                    except Exception as e:
                        logging.error(e)
                        continue
            except queue.Empty:
                pass


class RequestBase:
    client: Any


class StreamRequest(RequestBase):
    def __init__(self, path, client):
        self.path = path
        self.path_params = {}
        self.client = StreamClient(client)  # 如果是流请求，则升级为StreamClient，使其具有WebSocket一样的方法

    def receive_bytes(self):
        pass

    def accept(self):
        pass

    def send_bytes(self, param):
        pass


class SCPIRequest(RequestBase):
    device_num: str
    device_session: int

    def __init__(self, scpi, full_scpi, param, client):
        self.scpi = scpi
        self.full_scpi = full_scpi
        self.param = param
        self.client = client


class ArgType(Enum):
    """SCPI命令中关键字的参数的参数类型"""
    # 整型、实数、离散型、布尔型、ASCLL字符串、IEEE488.2 block、字符串
    INT = 0  # 整形
    BOOLEAN = 1  # 布尔型
    STRING = 2  # 字符串
    ARRAY = 3  # 数组
    FLOAT = 4  # 小数


@dataclass
class Arg:
    """SCPI命令中的关键字"""
    key_word_name: str
    key_word_type: ArgType
    key_word_default: Any

    def __init__(self, name, *args):
        self.key_word_name = name
        try:
            self.key_word_type = args[0]
        except IndexError:
            raise RuntimeError('Keyword parameters must provide parameter types')

        assert type(
            self.key_word_type) is ArgType, f"The keyword parameter type cannot be recognized: {self.key_word_type}"

        try:
            self.key_word_default = args[1]  # 默认值
        except IndexError:
            self.key_word_default = None


def extract_parameter(scpi_request: SCPIRequest, *args):
    param = scpi_request.param
    result = []
    for arg in args:
        result.append(param.get(arg, None))
    return tuple(result)


class Board:

    # def __init__(self, name: str = '', board_idx: int = 1, hwid: int = 1, in_channels: dict = None, out_channels=None, vi=None, ds_target: str = '', cr_target: str = '', cs_target: str = '', mode_functions: list = [], mode_config: dict = None):
    def __init__(self):
        self._name = ''  # 板卡别名
        self._board_idx = 1
        self._hwid = 1  # 板卡槽位号
        self._in_channels = {}  # {'通道虚拟编号': 通道真实编号}
        self._out_channels = {}  # {'通道虚拟编号': 通道真实编号}
        self._vi = None  # 程序与板卡的会话指针
        self._cs_target = ''
        self._cr_target = ''
        self._ds_target = ''
        self._mode_functions = []
        self._serial_number = ''
        self._firmware_version = ''
        self._module_model = ''
        self._mode_config = {}

    def test(self):
        print(self.__dict__)

    @property
    def name(self):
        return self._name

    @property
    def board_idx(self):
        return self._board_idx

    @property
    def hwid(self):
        return f'S{self._hwid}'

    @property
    def in_channels(self):
        return self._in_channels

    @property
    def out_channels(self):
        return self._out_channels

    @property
    def vi(self):
        return self._vi

    @property
    def cs_target(self):
        return self._cs_target

    @property
    def cr_target(self):
        return self._cr_target

    @property
    def ds_target(self):
        return self._ds_target

    @property
    def mode_functions(self):
        return self._mode_functions

    @property
    def mode_config(self):
        return self._mode_config

    @property
    def serial_number(self):
        return self._serial_number

    @property
    def firmware_version(self):
        return self._firmware_version

    @property
    def module_model(self):
        return self._module_model


@dataclass
class HttpAPIExpress:
    """
    @param api_type: str 'get', 'post', 'put', 'delete'....
    @param api: str
    @param params: dict json
    """
    server_ip: str = field(init=False)
    server_port: str = field(init=False)
    timeout: int = field(init=False)
    api_type: str
    api_path: str
    params: dict
    files: dict = None

    def __post_init__(self):
        self.server_ip = GlobalData().get_data('config').get('api_express').get('server_ip')
        self.server_port = GlobalData().get_data('config').get('api_express').get('server_port')
        self.timeout = GlobalData().get_data('config').get('api_express').get('timeout')

    def build_request(self) -> str:
        api_head = "http://" + self.server_ip + ':' + self.server_port
        http_headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) '
                          'Chrome/74.0.3729.131 Safari/537.36',
            'Content-Type': 'application/json',
            'origin': f'{self.server_ip}'
        }
        request_str = f"requests.{self.api_type}('{api_head}{self.api_path}', json={self.params}, timeout={self.timeout}, headers={http_headers})"
        if self.files:
            http_headers.pop('Content-Type')
            request_str = f"requests.{self.api_type}('{api_head}{self.api_path}', json={self.params}, timeout={self.timeout}, files={self.files}, headers={http_headers})"
        return request_str

    def execute_request(self, api_request: Union[str, None] = None) -> requests.Response:
        if api_request is None:
            api_request = self.build_request()
        resp = eval(api_request)
        resp.close()
        return resp


class RCDAPIExpress:
    @staticmethod
    def execute_operation(operation_id: int):
        """
        执行服务
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/operation/execute/{operation_id}/',
                             params={}).execute_request()
        return res

    @staticmethod
    def get_operation_info():
        """
        获取服务id
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/operation/?mode=9',
                             params={}).execute_request()
        return res

    @staticmethod
    def get_device_info():
        """
        执行服务
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/device/device/',
                             params={}).execute_request()
        return res

    @staticmethod
    def set_params(params: dict):
        """
        配置 配置参数
        {device_id: [{'id': parm_id, 'value': parm_value}]}"
        """
        res = HttpAPIExpress(api_type='post',
                             api_path='/command/parm/parmconf/',
                             params=params).execute_request()
        return res

    @staticmethod
    def get_param_status():
        """
        获取状态参数
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/parm/getparm/?ptype=状态参数',
                             params={}).execute_request()
        return res

    @staticmethod
    def get_param_config():
        """
        获取配置参数
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/parm/getparm/?ptype=配置参数',
                             params={}).execute_request()
        return res

    @staticmethod
    def get_param_all():
        """
        获取所有参数
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/parm/getparm/',
                             params={}).execute_request()
        return res

    @staticmethod
    def service_config(command: int):
        """
        开关任务
        """
        res = HttpAPIExpress(api_type='post',
                             api_path=f'/command/service/',
                             params={'command': command, 'comment': ''}).execute_request()
        return res

    @staticmethod
    def update_task_comment(task_id: int, comment: str):
        """
        更改任务信息
        """
        res = HttpAPIExpress(api_type='patch',
                             api_path=f'/device/task/{task_id}/',
                             params={'comment': comment}).execute_request()
        return res

    @staticmethod
    def switch_work_mode(word_mode_id: int):
        """
        切换工作模式
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/workmode/switch/?id={word_mode_id}',
                             params={}).execute_request()
        return res

    @staticmethod
    def get_work_mode_info():
        """
        获取工作模式信息
        """
        res = HttpAPIExpress(api_type='get',
                             api_path=f'/command/workmode/',
                             params={}).execute_request()
        return res

    @staticmethod
    def upload_sequence(device_ip: str, command_id: int, file_obj):
        """
        上传指令串文件
        """

        res = HttpAPIExpress(api_type='post',
                             api_path=f'/command/command/confcmdlist/{command_id}/{device_ip}/',
                             params={})
        res.files = {'file': (file_obj.filename, f"{file_obj.file}, 'rb')")}
        return res.execute_request()

    @staticmethod
    def delete_sequence(device_ip: str, command_id: int):
        """
        删除指令串
        """
        res = HttpAPIExpress(api_type='post',
                             api_path=f'/command/command/seq_delete/{command_id}/{device_ip}/',
                             params={}).execute_request()
        return res


class BVCModel(BaseModel):
    class Meta:
        ...

    @classmethod
    def model_meta(cls):
        return cls.Meta


class BvcPXIParameter(BVCModel):
    name: str
    id: str
    value: Any
    unit: str


class BvcPXIDeviceConfig(BVCModel):
    ...


class BvcPXIFuncConfig(BVCModel):
    class Meta:
        registered_function_confclass = {}

    trigger_source: str = ''
    trigger_cycle: float = 0
    trigger_width: float = 0
    trigger_count: int = 0

    @classmethod
    def get_func_config_class(cls, func):
        conf_cls = cls.Meta.registered_function_confclass.get(func, cls)
        return conf_cls


    @classmethod
    def register_func_config(cls, _type: "BvcPXIFunction.FuncType"):
        def der(sub_class: "Type[BvcPXIFuncConfig]"):
            cls.Meta.registered_function_confclass[_type] = sub_class
            return sub_class
        return der


class BvcPXIBoardConfig(BVCModel):
    ...


class BvcPXIChannelConfig(BVCModel):
    ...


class BvcPXIFunction(BVCModel):
    class FuncType(str, Enum):
        FGEN = 'fgen'
        DIGITIZER = 'digitizer'
        SAT = 'sat'

    type: FuncType
    config: BvcPXIFuncConfig

    @classmethod
    def with_func(cls, func: FuncType):
        conf_cls = BvcPXIFuncConfig.get_func_config_class(func)
        return cls(type=func, config=conf_cls())


class BvcPXIChannel(BVCModel):
    class ChannelType(str, Enum):
        AD = 'ad'
        DA = 'da'

    chnl_id: int
    dev_chnl: str
    io_flag: ChannelType = 'da'
    parameters: BvcPXIChannelConfig = BvcPXIChannelConfig()


class BvcPXIBoard(BVCModel):
    name: str = ''
    idx: int = 0
    slot: int = 0
    serial_number: int = 0
    firmware_ident: int = 0
    function_ident: int = 0
    capability_ident: int = 0
    vendor_id: int = Field(0x22a2)
    device_id: int = Field(0x4902)
    cs_target: str = 'xdma://0'
    cr_target: str = 'xdma://0'
    ds_target: str = 'xdma://0'
    vi: VI = Field(default_factory=VI)
    functions: dict[str, BvcPXIFunction] = Field(default_factory=dict)
    config: BvcPXIBoardConfig = BvcPXIBoardConfig()
    in_channels: dict[str, BvcPXIChannel] = Field(default_factory=dict)
    out_channels: dict[str, BvcPXIChannel] = Field(default_factory=dict)

    model_config = ConfigDict(
        arbitrary_types_allowed=True,
        # json_encoders={VI: lambda v: repr(v)},  # 自定义序列化
        extra='allow',
        use_enum_values=True,
        populate_by_name=True
    )

    @computed_field
    @property
    def id(self) -> str:
        return f'S{str(self.slot)}_{self.fpga_id}'

    @computed_field
    @property
    def fpga_id(self) -> str:
        return f'M{hex(self.device_id)[2:].zfill(4)}'

    @computed_field
    @property
    def fpga_firmware(self) -> str:
        return f'{hex(self.firmware_ident)[2:].zfill(8)}'

    @computed_field
    @property
    def fpga_function(self) -> str:
        return hex(self.function_ident)[2:].zfill(8)

    @computed_field
    @property
    def fpga_capability(self) -> str:
        capa_str = hex(self.capability_ident)[2:]
        capa_str = capa_str.zfill(8)
        return f'F{capa_str[:2]}C{capa_str[2:4]}A{capa_str[4]}D{capa_str[5]}I{capa_str[6]}V{capa_str[7]}'

    @computed_field
    @property
    def full_version(self) -> str:
        return f'Ver-{self.fpga_capability}-{self.fpga_firmware}-{self.fpga_function}'


class BvcPXIDevice(BVCModel):
    boards: Dict[str, BvcPXIBoard] = Field(default_factory=dict)
    chassis: str = ''
    config: BvcPXIDeviceConfig = BvcPXIDeviceConfig()



if __name__ == '__main__':

    # 示例数据
    board_data = BvcPXIBoard(
        slot=2,
        device_id=0x4901,
        firmware_ident = 0x00000001,
        function_ident = 0x00000001,
        capability_ident = 0x00000001,
        channels={
            'S2-O1': BvcPXIChannel(chnl_id = 1),
            'S2-O2': BvcPXIChannel(chnl_id = 2),
        }
    )
    device_data = BvcPXIDevice(
        boards={
            board_data.id: board_data
        }
    )

