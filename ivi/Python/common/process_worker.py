import atexit
import multiprocessing
from concurrent.futures import ProcessPoolExecutor
ctx = multiprocessing.get_context('spawn')

class ProcessWorker:
    max_workers = 4
    bvc_worker_pool = ProcessPoolExecutor(max_workers=max_workers, mp_context=ctx)
    @classmethod
    def restart_process_pool(cls):
        cls.bvc_worker_pool.shutdown(wait=False, cancel_futures=True)
        cls.bvc_worker_pool = ProcessPoolExecutor(max_workers=cls.max_workers, mp_context=ctx)

@atexit.register
def exit_handler():
    ProcessWorker.bvc_worker_pool.shutdown(wait=True, cancel_futures=True)