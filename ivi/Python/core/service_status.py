import asyncio
import sys
import threading
import yaml
import platform
import os
from common.model import NoticePusher
from common.utils import GlobalData, SingletonMeta
from core.http_server import start_http_server, register_websocket_router
from core.tcp_server import start_tcp_server
from core.static_server import start_static_server
from device.device_manager import BVCManager, DeviceManager, NetworkManager
from drp.router import Router, import_views
from license.license_initializer import LicenseInitializer
from parser.parser import Parser
from tools.logging import logging
from tools.utils import load_device_info, dump_device_info, load_software_version
from views.DAQ.utils import UploadControl
from views.SYS.utils import SysConfigControlor


class ServiceStatus(metaclass=SingletonMeta):
    def __init__(self, pid):
        self.pid = pid
        GlobalData().set_data("system_os", platform.system().lower())

        GlobalData().set_data("conf_path", "./etc/bvc/conf/")
        GlobalData().set_data("license_path", "./etc/bvc/license/")
        GlobalData().set_data("bvc_path", "./usr/share/bvc/")

        if getattr(sys, 'frozen', False):
            if GlobalData().get_data("system_os") != 'windows':
                GlobalData().set_data("conf_path", "/etc/bvc/conf/")
                GlobalData().set_data("license_path", "/etc/bvc/license/")
                GlobalData().set_data("bvc_path", "/usr/share/bvc/")
            load_software_version(is_debug=False)
        else:
            load_software_version(is_debug=True)
        
        logging.info(msg=f"GlobalData: {GlobalData()._get_all()}")

        try:
            config_path = GlobalData().get_data("conf_path") + "config.yaml"
            with open(config_path, "r", encoding="UTF-8") as f:
                self.config = yaml.load(f.read(), Loader=yaml.FullLoader)
        except Exception as e:
            logging.error(msg=f"Load config field: {e}")
            self.config = {'http': {'ip': '0.0.0.0', 'port': 8000}, 'socket': {'tcp': {'ip': '0.0.0.0', 'port': 5555}}}
        GlobalData().set_data("config", self.config)
        load_device_info()
        dump_device_info()

        initializer = LicenseInitializer()
        license_initialized = initializer.initialize()
        if not license_initialized:
            logging.warning("Failed to initialize license system!")

        self.router = Router()
        import_views(self.config.get('views_dir', ['views']))
        self.parser = Parser()
        self.parser.set_scpi_fmts(self.router.scpi_fmts)
        # 生成
        self.bvcmanager = BVCManager()
        self.network_manager = NetworkManager()
        if GlobalData().get_data("system_os") == 'linux':
            network_devices_map = self.network_manager.network_devices_scan(mac_header=self.config.get("ivi", None).get("mac_header", None))
            GlobalData().set_data("network_devices_map", network_devices_map)
            for _, network_ip in network_devices_map.items():
                if self.config.get("ivi", None).get("mac", None).get(os.uname().machine, None) in network_ip['mac']:
                    GlobalData().set_data("network_IP", network_ip['addr'])

        self.device_manager = DeviceManager()
        self.device_manager.load_config()
        SysConfigControlor().Rfconfig()
        UploadControl()

        register_websocket_router()

        self.http_server_thread = threading.Thread(target=start_http_server, args=(
            self.config.get('http').get('ip'), self.config.get('http').get('port')), daemon=True)
        self.tcp_server_thread = threading.Thread(
            target=start_tcp_server,
            args=(self.config.get('socket').get('tcp').get('ip'), self.config.get('socket').get('tcp').get('port')),
            daemon=True)
        self.static_server_thread = threading.Thread(target=start_static_server, args=(
            self.config.get('static').get('ip'), self.config.get('static').get('port'),
            GlobalData().get_data("bvc_path") + self.config.get('static').get('directory')), daemon=True)

        logging.info(f"Found {len(self.router.url_path)} scpi instructions:")
        # todo: scpi 导出功能
        # for module in self.router.module_url_map:
        #     logging.info(f'{module}({len(self.router.module_url_map[module])}): {self.router.module_url_map[module]}')
        GlobalData().set_data("NoticePusher", NoticePusher())
        GlobalData().set_data("LookData_extractMult", 1)
        GlobalData().set_data('NSQC_Config', {})
        GlobalData().set_data('function_config', self.config.get("function_config"))
        # logging.info(msg=f"GlobalData: {GlobalData()._get_all()}")

    def start_server(self):
        self.http_server_thread.start()
        self.tcp_server_thread.start()
        self.static_server_thread.start()
        asyncio.run(system_error_push())


async def system_error_push():
    while True:
        await GlobalData().get_data('NoticePusher').send_message()
        await asyncio.sleep(.5)
