import threading


class SingletonMeta(type):
    _instances = {}
    _lock = threading.RLock()

    def __call__(cls, *args, **kwargs):
        with cls._lock:
            if cls.__name__ not in cls._instances:
                instance = super().__call__(*args, **kwargs)
                cls._instances[cls.__name__] = instance
        return cls._instances[cls.__name__]


class GlobalData(metaclass=SingletonMeta):
    _data = {}

    def __init__(self):
        pass

    def get_data(self, key):
        return self._data.get(key, None)

    def set_data(self, key, value):
        self._data[key] = value

    def _get_all(self):
        return self._data


def frequency_to_number(frequency_str):
    """
    将带有单位的频率字符串转换为数值。
    
    支持的单位：
    - G (Giga) -> 10^9
    - M (Mega) -> 10^6
    - K (Kilo) -> 10^3
    
    参数:
    - frequency_str (str): 频率字符串，例如 "1G", "1000M", "1000000"
    
    返回:
    - int: 转换后的数值
    """
    units = {
        'G': 1_000_000_000,
        'M': 1_000_000,
        'K': 1_000
    }
    
    # 去除字符串中的空格
    frequency_str = frequency_str.strip()
    
    # 检查是否有单位
    if frequency_str[-1].upper() in units:
        unit = frequency_str[-1].upper()
        value = frequency_str[:-1]
    else:
        unit = None
        value = frequency_str
    
    # 将字符串转换为数值
    try:
        value = float(value)
    except ValueError:
        raise ValueError(f"无效的频率字符串: {frequency_str}")
    
    # 根据单位进行转换
    if unit:
        value *= units[unit]
    
    return int(value)

def number_to_frequency(number):
    """
    将数字转换为频率，并使用最大的单位表示。

    :param number: 输入的数字
    :return: 转换后的频率字符串
    """
    units = ["G"]
    unit_factors = [1e9]

    # units = ["H", "k", "M", "G"]
    # unit_factors = [1, 1e3, 1e6, 1e9]

    if number == 0:
        return "0Hz"

    for i, factor in enumerate(unit_factors):
        if number < unit_factors[i + 1] if i + 1 < len(unit_factors) else True:
            value = number / factor
            return f"{value:.2f}{units[i]}"
