import logging

logging = logging


def __init():
    if len(logging.root.handlers) > 0:
        logging.root.handlers.clear()
    console = logging.StreamHandler()  # 定义console handler
    console.setLevel(logging.DEBUG)  # 定义该handler级别
    
    # 创建自定义格式化器，为ERROR级别添加文件路径和行号
    class CustomFormatter(logging.Formatter):
        def format(self, record):
            # 基础格式
            base_format = '[%(asctime)s] BVC %(levelname)s: %(message)s'
            
            # 如果是ERROR级别，添加文件路径和行号
            if record.levelno >= logging.ERROR:
                error_format = '[%(asctime)s] BVC %(levelname)s: File "%(pathname)s", line %(lineno)d, %(message)s'
                formatter = logging.Formatter(error_format)
                return formatter.format(record)
            else:
                formatter = logging.Formatter(base_format)
                return formatter.format(record)
    
    formatter = CustomFormatter()
    console.setFormatter(formatter)
    logging.base_formatter_str = '[%(asctime)s] BVC %(levelname)s: %(message)s'
    logging.getLogger().addHandler(console)  # 实例化添加handler
    logging.getLogger().setLevel(logging.INFO)


__init()
