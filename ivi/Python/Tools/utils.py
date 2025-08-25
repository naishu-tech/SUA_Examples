import json
import os
import base64
import pathlib
import sys
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.backends import default_backend
from common.utils import GlobalData
from tools.logging import logging


def load_software_version(is_debug: bool):
    if is_debug:
        # 调试模式下，使用git版本号, 并生成文件进行存储
        try:
            import git
            _git = git.Git()
            software_version = _git.describe('--tags')
        except Exception as e:
            software_version = 'Unknown'
        with open('./Version', "w", encoding='utf-8') as f:
            f.write(software_version)
    else:
        # 非调试模式下，使用文件存储的版本号
        file_path = pathlib.Path(sys._MEIPASS) / 'Version'
        try:
            with open(file_path, "r", encoding='utf-8') as f:
                software_version = f.read()
        except Exception as e:
            software_version = 'Unknown'
    GlobalData().set_data('device_SoftWareVersion', software_version)


def load_device_info():
    info = {}
    DeviceInfo_path = GlobalData().get_data("conf_path") + 'DeviceInfo'
    if os.path.exists(DeviceInfo_path):
        try:
            with open(DeviceInfo_path, "rb") as f:
                _info = f.read()
                _info = aes_decode(_info)
                _info = json.loads(_info)
            info['device_Name'] = _info.get('device_Name', '')
            info['device_Model'] = _info.get('device_Model', '')
            info['device_SerialNumber'] = _info.get('device_SerialNumber', '')
        except Exception as e:
            logging.error(e)
    else:
        info['device_Name'] = 'TestDevice'
        info['device_Model'] = 'Device_Model'
        info['device_SerialNumber'] = 'Device_SerialNumber'
    GlobalData().set_data('device_Name', info.get('device_Name'))
    GlobalData().set_data('device_Model', info.get('device_Model'))
    GlobalData().set_data('device_SerialNumber', info.get('device_SerialNumber'))
    # load_software_version()


def dump_device_info():
    info = {}
    DeviceInfo_path = GlobalData().get_data("conf_path") + 'DeviceInfo'
    info['device_Model'] = GlobalData().get_data('device_Model')
    info['device_Name'] = GlobalData().get_data('device_Name')
    info['device_SerialNumber'] = GlobalData().get_data('device_SerialNumber')
    # info['device_SoftWareVersion'] = GlobalData().get_data('device_SoftWareVersion')
    info = json.dumps(info)
    info = aes_encode(info)
    try:
        with open(DeviceInfo_path, "wb") as f:
            f.write(info)
    except Exception as e:
        logging.error(e)


def aes_encode(ori_data: str, aes_key: str = 'Superlab34444444'):
    aes_key = aes_key.encode()
    ori_data = ori_data.encode()
    data_padder = padding.PKCS7(128).padder()
    ori_data = data_padder.update(ori_data) + data_padder.finalize()
    backend = default_backend()
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(aes_key), backend=backend)
    encryptor = cipher.encryptor()
    ct = encryptor.update(ori_data)
    ct_out = base64.b64encode(ct)
    return ct_out


def aes_decode(aes_data: bytes, aes_key: str = 'Superlab34444444'):
    aes_key = aes_key.encode()
    aes_data = base64.b64decode(aes_data)
    backend = default_backend()
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(aes_key), backend=backend)
    decryptor = cipher.decryptor()
    _res = decryptor.update(aes_data)
    padding_len = _res[-1]
    return _res[:-padding_len]