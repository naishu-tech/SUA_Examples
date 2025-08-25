import platform
import os
import sys
import multiprocessing as mp
from functools import partial
from common.utils import SingletonMeta
from Tools.logging import logging
from ivi.iviATTR import *
from ivi.iviVAL import *

if platform.system().lower() == 'windows':
    sys.path.append(os.path.abspath('./device/ivi/windows'))
else:
    sys.path.append('/lib')


class IviDriver(metaclass=SingletonMeta):
    _system_os = platform.system().lower()  # windows or linux

    def __init__(self):
        self.module = {}

    def import_module(self, module_list: list):
        for module_name in module_list:
            if module_name not in self.module or self.module[module_name] is None:
                module_path = f"{module_name}"
                try:
                    self.module[module_name] = __import__(module_path, fromlist=(f'{module_name}',))
                except Exception as e:
                    logging.error(f'{module_path} does not exist, {e}')
                    self.module[module_name] = None

    def _get_module_name(self) -> list:
        return [module_name for module_name in self.module]

    def _get_module(self, module_name: str) -> list:
        return self.module[module_name]


class VI:

    def __init__(self):
        self._ivi_driver = ivi_driver
        self._visession = {}
        self.module_dict = {"iviFgen": self._ivi_driver._get_module('iviFgen'),
                            "iviDigitizer": self._ivi_driver._get_module('iviDigitizer'),
                            "iviSyncATrig": self._ivi_driver._get_module('iviSyncATrig'),
                            "iviPXIMainBoard": self._ivi_driver._get_module('iviPXIMainBoard'),
                            "iviSUATools_vi": self._ivi_driver._get_module('iviSUATools_vi')}

    def __getattr__(self, item: str):
        if "_VAL_" in item or "_ATTR_" in item or "_STATE_" in item or "_ERROR_" in item:
            return eval(item)
        module_name = item.split("_")[0]
        module = self.module_dict.get(module_name)
        func = getattr(module, item)
        if item.endswith('Initialize'):
            return func
        if callable(func):
            func = partial(func, self._visession.get(module_name))
        return func


if mp.current_process().name == 'MainProcess':
    ivi_driver: IviDriver = IviDriver()
    ivi_driver.import_module(['iviFgen', 'iviDigitizer', 'iviSyncATrig', 'iviPXIMainBoard', 'iviSUATools_vi'])