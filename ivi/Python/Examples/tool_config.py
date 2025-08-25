import json
import yaml
import os
import platform
import sys
import pathlib
from typing import Dict, List
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../")
from common.model import Board
from ivi import VI
from Tools.logging import logging

def DUCConfigAWG(vi, DUC_Enable, channelName, DUC_NCO_Frequency):
    """配置AWG的DUC"""
    s = VI.iviFgen_SetAttributeViUInt32(vi, "0", VI.IVIFGEN_ATTR_DAC_DUC_ENABLE, DUC_Enable)
    s = VI.iviFgen_SetAttributeViReal64(vi, "-1", VI.IVIFGEN_ATTR_DAC_NCO_FREQUENCY, DUC_NCO_Frequency)
    s = VI.iviFgen_SetAttributeViInt32(vi, "0", VI.IVIFGEN_ATTR_DAC_DUC_EXE, 1)
    result = vi.iviFgen_GetAttributeViUInt32(vi, "0", VI.IVIFGEN_ATTR_DAC_DUC_RESULT)
    print(f"IVIFGEN_ATTR_DAC_DUC_RESULT result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS

def triggerConfigAWG(VI, triggerSource):
    """配置AWG的触发源"""
    s = VI.iviFgen_SetAttributeViUInt32(vi, "0", VI.IVIFGEN_ATTR_TRIGGER_SOURCE, triggerSource)
    s = VI.iviFgen_SetAttributeViUInt32(vi, "0", VI.IVIFGEN_ATTR_TRIGGER_SOURCE_EXE, 0)
    result = VI.iviFgen_GetAttributeViUInt32(vi, "0", VI.IVIFGEN_ATTR_TRIGGER_SOURCE_RESULT)
    print(f"triggerConfigAWG result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS

def internalTriggerConfigSAT(vi, triggerSource, triggerPeriod=6400, triggerRepetSize=4294967295, triggerPulseWidth=1600):
    """配置SAT的内部触发"""
    s = VI.iviSyncATrig_SetAttributeViUInt32(vi, "0", VI.IVISYNCATRIG_ATTR_TRIGGER_SOURCE, triggerSource)
    s = VI.iviSyncATrig_SetAttributeViUInt32(vi, "0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PERIOD, triggerPeriod)
    s = VI.iviSyncATrig_SetAttributeViUInt32(vi, "0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepetSize)
    s = VI.iviSyncATrig_SetAttributeViUInt32(vi, "0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth)
    s = VI.iviSyncATrig_SetAttributeViUInt32(vi, "0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_EXE, 0)
    result = VI.iviSyncATrig_GetAttributeViUInt32(vi, "0", vi.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_RESULT)
    print(f"internalTriggerConfigSAT result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS

def DDConfigDAQ(vi, DDC_Enable, channelName, DDC_NCO_Frequency):
    """配置DAQ的DDC"""
    s = vi.iviDigitizer_SetAttributeViUInt32("0", vi.IVIDIGITIZER_ATTR_ADC_DDC_ENABLE, DDC_Enable)
    s = vi.iviDigitizer_SetAttributeViReal64("-1", vi.IVIDIGITIZER_ATTR_ADC_NCO_FREQUENCY, DDC_NCO_Frequency)
    s = vi.iviDigitizer_SetAttributeViInt32("0", vi.IVIDIGITIZER_ATTR_ADC_DDC_EXE, 0)
    result = vi.iviDigitizer_GetAttributeViUInt32("0", vi.IVIDIGITIZER_ATTR_ADC_DDC_RESULT)
    print(f"DDConfigDAQ result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS

def triggerConfigDAQ(vi, triggerSource):
    """配置DAQ的触发源"""
    s = vi.iviDigitizer_SetAttributeViUInt32("0", vi.IVIDIGITIZER_ATTR_TRIGGER_SOURCE, triggerSource)
    s = vi.iviDigitizer_SetAttributeViInt32("0", vi.IVIDIGITIZER_ATTR_TRIGGER_CONFIG_EXECUTE, 0)
    result = vi.iviDigitizer_GetAttributeViUInt32("0", vi.IVIDIGITIZER_ATTR_TRIGGER_SOURCE_RESULT)
    print(f"triggerConfigDAQ result is {result}")
    if result:
        return vi.VI_STATE_FAIL
    else:
        return vi.VI_STATE_SUCCESS

def internalTriggerConfigSAT(vi, triggerSource, triggerPeriod=6400, triggerRepetSize=4294967295, triggerPulseWidth=1600):
    """配置SAT的内部触发"""
    s = vi.iviSyncATrig_SetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_TRIGGER_SOURCE, triggerSource)
    s = vi.iviSyncATrig_SetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PERIOD, triggerPeriod)
    s = vi.iviSyncATrig_SetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepetSize)
    s = vi.iviSyncATrig_SetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth)
    s = vi.iviSyncATrig_SetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_EXE, 0)
    result = vi.iviSyncATrig_GetAttributeViUInt32("0", VI.IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_RESULT)
    print(f"internalTriggerConfigSAT result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS

def sampleConfigDAQ(vi, sampleEnableChannel, sampleEnable, sampleStrageDepthChannel, sampleStrageDepth,
                   sampleLenPreChannel, sampleLenPre, sampleTimesChannel, sampleTimes,
                   sampleLogicalExtractionMultipleChannel, sampleLogicalExtractionMultiple,
                   sampleCollectDataTruncationChannel, sampleCollectDataTruncation,
                   sampleCollectDataTypeChannel, sampleCollectDataType):
    """配置DAQ的采样参数"""
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleEnableChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_ENABLE, sampleEnable)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleStrageDepthChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_STRAGE_DEPTH, sampleStrageDepth)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleLenPreChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_POINT_PRE, sampleLenPre)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleTimesChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_TIMES, sampleTimes)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleLogicalExtractionMultipleChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_LOGICAL_EXTRACTION_MULTIPLE, sampleLogicalExtractionMultiple)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleCollectDataTruncationChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TRUNCATION, sampleCollectDataTruncation)
    s = vi.iviDigitizer_SetAttributeViUInt32(sampleCollectDataTypeChannel, vi.IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TYPE, sampleCollectDataType)
    s = vi.iviDigitizer_SetAttributeViUInt32("0", vi.IVIDIGITIZER_ATTR_SAMPLE_CONFIG_EXECUTE, 0)
    result = vi.iviDigitizer_GetAttributeViUInt32("0", VI.IVIDIGITIZER_ATTR_SAMPLE_CONFIG_RESULT)
    print(f"sampleConfigDAQ result is {result}")
    if result:
        return VI.VI_STATE_FAIL
    else:
        return VI.VI_STATE_SUCCESS



class BVCManager:
    boards: List[Board] = []
    if getattr(sys, 'frozen', False):
        config_dir_path = pathlib.Path(sys._MEIPASS) / '../conf/config.d'

    else:
        config_dir_path = pathlib.Path('../conf/config.d')
    def __init__(self, conf_path):
        self.modes: Dict[str, pathlib.Path] = {}
        self.conf_path = conf_path
        try:
            config_path = self.conf_path + "config.yaml"
            with open(config_path, "r", encoding="UTF-8") as f:
                self.config = yaml.load(f.read(), Loader=yaml.FullLoader)
        except Exception as e:
            logging.error(msg=f"Load config field: {e}")
            self.config = {'http': {'ip': '0.0.0.0', 'port': 8000}, 'socket': {'tcp': {'ip': '0.0.0.0', 'port': 5555}}}


        self.resource_db_path = self.conf_path + self.config.get("ivi", None).get("resourceDB_path", None)
        self.sim_mode = self.config.get("ivi", None).get("sim", None)
        self.template: Dict = {}

        if self.sim_mode:
            # 生成模拟模式
            self._generate_resource_db_template()
            # 写入到配置文件
            self._init_simulation_mode()
        else:
            # 扫描在线板卡信息
            self.available_boards: List[str] = self.scan_online_board()
            # 生成基础DB配置文件
            self._generate_resource_db_template()
            # 写入到配置文件
            self._init_simulation_mode()

            # 初始化IVI
            self._init_ivi()
            # 更新DB配置文件
            self._init_simulation_mode()


    # 扫描在线板卡信息
    def scan_online_board(self) -> "List[str]":
        if platform.system().lower() == 'windows':
            logging.warning(f'scan_online_board(): Windows operating system is currently not supported!')
            return []
        boards = set()
        prefix = f'xdma'
        folder = pathlib.Path('/dev')
        for file in folder.glob(f"{prefix}*"):
            bid = file.name.partition('_')[0].replace(prefix, '')
            if not bid.isdigit():
                continue
            target = f'xdma://{bid}'
            if target not in boards:
                boards.add(target)
                logging.info(f'{self.__class__.__name__}: Scan to potentially supported devices: {target}')
        return list(sorted(boards))

    def _init_simulation_mode(self):
        try:
            if not os.path.exists(os.path.dirname(self.resource_db_path)):
                os.makedirs(os.path.dirname(self.resource_db_path), exist_ok=True)
            with open(self.resource_db_path, 'w') as f:
                json.dump(self.template, f, indent=4)
            logging.info(f"Generated new resourceDB template at: {self.resource_db_path}")
        except Exception as e:
            logging.error(f"Failed to load/generate resourceDB: {str(e)}")
            raise

    def _generate_resource_db_template(self):
        """Generate resourceDB template based on mode"""
        if self.sim_mode:
            self.sim_resourceDB_path = self.conf_path + self.config.get("ivi", None).get("sim_resourceDB_path", None)
            if self.sim_resourceDB_path and os.path.exists(self.sim_resourceDB_path):
                self.template: Dict = {}
                with open(self.sim_resourceDB_path, 'r') as src_file:
                    self.template = json.load(src_file)
                module_list = self.template.keys()
                for module in module_list:
                    ivi_path = self.conf_path + f"S{self.template[module]['sim_info']['IVIBASE_ATTR_FIRMWARE_SLOT_POSITION']}_" + self.template[module]['config']
                    icd_path = self.conf_path + f"S{self.template[module]['sim_info']['IVIBASE_ATTR_FIRMWARE_SLOT_POSITION']}_" + self.template[module]['param']['icd_path']
                    if not os.path.exists(ivi_path):
                        ivi_path = self.conf_path + self.template[module]['config']
                    if not os.path.exists(icd_path):
                        icd_path = self.conf_path + self.template[module]['param']['icd_path']
                    self.template[module]['config'] = ivi_path
                    self.template[module]['param']['icd_path'] = icd_path
                logging.info(f"Copied resourceDB from: {self.sim_resourceDB_path}")
            else:
                logging.error(f"{self.sim_resourceDB_path} is Null")
                self.template: Dict = {}
                self.template = {
                    "PXI::0::INSTR": {
                        "config": f"{self.conf_path}ivi_4901XXXX-22.json",
                        "ds_target": "sim://0",
                        "cr_target": "sim://0",
                        "cs_target": "sim://0",
                        "work_mode": {
                            "AWG": 3,
                            "DAQ": 0
                        },
                        "sim_info": {
                            "enable": True,
                            "IVIBASE_ATTR_FIRMWARE_SLOT_POSITION": 0,
                            "IVIBASE_ATTR_MODULE_DEVICE_ID_STR": "M4901",
                            "IVIBASE_ATTR_FPGA_VERSION_STR": "Ver-F00C44A0D010-AD370306-22a24901",
                            "IVIDIGITIZER_ATTR_SIM_SOURCE_RATE": 100000000.0,
                            "IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE_MAX": 4000000000.0,
                            "IVIFGEN_ATTR_DAC_SAMPLE_RATE_MAX": 8000000000.0,
                            "IVIDIGITIZER_ATTR_TEST_SAMPLE_STATE": 3,
                            "IVIBASE_ATTR_MODULE_SERIAL_NUM": 1232991696,
                            "IVIBASE_ATTR_MODULE_DEVICE_ID": 4624,
                            "IVIBASE_ATTR_FPGA_FUNCTION_IDENTIFICATION": 0,
                            "IVIBASE_ATTR_FPGA_FIRMWARE_VERSION": 303039256,
                            "IVIBASE_ATTR_BASIC_CAPABILITY_IDENTIFICATION": 16843008
                        },
                        "param": {
                            "icd_path": f"{self.conf_path}icd.json"
                        }
                    },
                    "PXI::1::INSTR": {
                        "config": f"{self.conf_path}ivi_1210XXXX-00.json",
                        "ds_target": "sim://0",
                        "cr_target": "sim://0",
                        "cs_target": "sim://0",
                        "work_mode": {
                            "AWG": 3,
                            "DAQ": 0
                        },
                        "sim_info": {
                            "enable": True,
                            "IVIBASE_ATTR_FIRMWARE_SLOT_POSITION": 2,
                            "IVIBASE_ATTR_MODULE_DEVICE_ID_STR": "M4901",
                            "IVIBASE_ATTR_FPGA_VERSION_STR": "Ver-F00C44A0D010-AD370306-22a24901",
                            "IVIDIGITIZER_ATTR_SIM_SOURCE_RATE": 100000000.0,
                            "IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE_MAX": 4000000000.0,
                            "IVIFGEN_ATTR_DAC_SAMPLE_RATE_MAX": 8000000000.0,
                            "IVIDIGITIZER_ATTR_TEST_SAMPLE_STATE": 3,
                            "IVIBASE_ATTR_MODULE_SERIAL_NUM": 1232991696,
                            "IVIBASE_ATTR_MODULE_DEVICE_ID": 18689,
                            "IVIBASE_ATTR_FPGA_FUNCTION_IDENTIFICATION": 287440896,
                            "IVIBASE_ATTR_FPGA_FIRMWARE_VERSION": 1224806449,
                            "IVIBASE_ATTR_BASIC_CAPABILITY_IDENTIFICATION": 16843008
                        },
                        "param": {
                            "icd_path": f"{self.conf_path}icd.json"
                        }
                    }
                }
        else:
            for board in self.available_boards:
                board_num = board.split('://')[-1]
                board_name = f"PXI::{board_num}::INSTR"
                self.template[board_name] = {
                    "config": f"{self.conf_path}ivi.json",
                    "ds_target": f"sim://{board_num}",
                    "cr_target": f"xdma://{board_num}",
                    "cs_target": f"xdma://{board_num}",
                    "work_mode": {
                        "AWG": 3,
                        "DAQ": 0
                    },
                    "param": {
                        "cmd_sent_base": 0,
                        "cmd_recv_base": 31457280,
                        "cmd_irq_base": 32505900,
                        "cmd_sent_down_base": 32505904,
                        "icd_path": f"{self.conf_path}icd.json"
                    }
                }

    def _init_ivi(self):
        if not os.path.exists(self.resource_db_path):
            msg = f'Configuration file not found in this path: {self.resource_db_path}'
            logging.error(msg=msg)
            raise RuntimeError(msg)
        try:
            with open(self.resource_db_path, 'rb') as fp:
                main_conf = json.load(fp)
            # main_conf.pop('VER')
        except Exception as e:
            msg = f'The configuration {self.resource_db_path} is written incorrectly: {e.args}'
            logging.error(msg=msg)
            raise RuntimeError(msg)
        self.template: Dict = {}
        self.modes = self._collect_board_config()
        self._check_board(main_conf)

    def _collect_board_config(self) -> Dict[str, pathlib.Path]:
        print(self.config_dir_path)

        if not self.config_dir_path.exists():
            msg = f"The configuration file folder does not exist: {self.config_dir_path}"
            logging.error(msg=msg)
            raise RuntimeError(msg)
        res = {}
        for path in self.config_dir_path.iterdir():
            if path.is_dir():
                res[path.name] = path
            else:
                res[path.name.split('.')[0]] = path
        return res

    def _check_board(self, main_config: dict):
        self._board_configs: Dict = {}
        for idx, module in enumerate([module for module in main_config]):
            # 初始化板卡并获得固件配置文件
            board_num = module.split('::')[1]
            board_mode, work_mode_AWG, work_mode_DAQ, slot = self._get_board_mode(module)
            if board_mode == "ffffXXXX-ff" or board_mode == "0000XXXX-00":
                continue
            self._load_board_mode(board_mode, board_num, int(work_mode_AWG), int(work_mode_DAQ), slot)
            # self._board_configs.update(template)

    def _get_board_mode(self, logical_name:str) -> tuple[str, str, str]:
        vi = VI()
        vi._visession['iviBase'] = vi.iviBase_Initialize(logicalName=logical_name, IDQuery=0, resetDevice=1, resourceDBPath=self.resource_db_path)
        slot = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_FIRMWARE_SLOT_POSITION)
        self.capability_ident = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_FPGA_FUNCTION_IDENTIFICATION)
        self.firmware_ident = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_FPGA_FIRMWARE_VERSION)
        capa_str = hex(self.capability_ident)[2:]
        capa_str = capa_str.zfill(8)
        fpga_capability = f'F{capa_str[:2]}C{capa_str[2:4]}A{capa_str[4]}D{capa_str[5]}I{capa_str[6]}'

        fpga_firmware = f'{hex(self.firmware_ident)[2:].zfill(8)}'

        if fpga_firmware == 'ffffffff':
            self.capability_ident = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_A7_FPGA_FUNCTION_IDENTIFICATION)
            self.firmware_ident = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_A7_FPGA_FIRMWARE_VERSION)
            slot = vi.iviBase_GetAttributeViUInt32(channel='0', attributeID=vi.IVIBASE_ATTR_A7_FIRMWARE_SLOT_POSITION)
            capa_str = hex(self.capability_ident)[2:]
            capa_str = capa_str.zfill(8)
            fpga_capability = f'F{capa_str[:2]}C{capa_str[2:4]}A{capa_str[4]}D{capa_str[5]}I{capa_str[6]}'

            fpga_firmware = f'{hex(self.firmware_ident)[2:].zfill(8)}'
        vi.iviBase_Close()

        return (fpga_firmware if fpga_firmware in self.modes else (fpga_firmware[:4] + 'XXXX')) + '-' + fpga_capability[4:6], fpga_capability[9], fpga_capability[7], slot

    def _load_board_mode(self, board_mode, board_num, work_mode_AWG, work_mode_DAQ, slot):
        if board_mode not in self.modes:
            # logging.warning(f"板卡{bvc_board.ds_target}_{bvc_board.cs_target}查询到模式{board_mode}，但无对应的配置文件")
            board_mode = 'default'
        conf_path = self.modes[board_mode]
        try:
            with open(conf_path, 'rb') as fp:
                board_conf = json.load(fp)
        except Exception as e:
            logging.error(f'当前配置文件路径为：{conf_path}，配置文件解析失败，格式有误')
            raise e
        board_name = f"PXI::{board_num}::INSTR"  # Extract board number from xdma://X

        ivi_path = self.conf_path + f"S{slot}_" + board_conf['ivi']
        icd_path = self.conf_path + f"S{slot}_" + "icd.json"
        if not os.path.exists(ivi_path):
            ivi_path = self.conf_path + board_conf['ivi']
        if not os.path.exists(icd_path):
            icd_path = self.conf_path + "icd.json"

        self.template[board_name] = {
            "config": ivi_path,
            "ds_target": f"{board_conf['ds_target']}://{board_num}",
            "cr_target": f"{board_conf['cr_target']}://{board_num}",
            "cs_target": f"{board_conf['cs_target']}://{board_num}",
            "work_mode": {
                "AWG": work_mode_AWG,
                "DAQ": work_mode_DAQ
            },
            "param": board_conf['default_param'].copy()
        }
        self.template[board_name]["param"]["icd_path"] = icd_path

if __name__ == '__main__':
    bvcmanager = BVCManager("../conf/")
    print(bvcmanager.resource_db_path)