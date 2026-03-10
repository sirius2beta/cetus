import threading
import time

class Device():
    def __init__(self, device_type, dev_path=""):
        self.dev_path = dev_path
        self.isOpened = False
        self.device_type = device_type

    def start_loop(self):
        io_thread = threading.Thread(target = self._io_loop)
        io_thread.daemon = True
        io_thread.start()

    # getter
    def get(self): 
        pass
    # setter
    def set(self):
        pass

    # process command for control
    def processCMD(self, control_type, cmd):
        pass
    
    def _io_loop(self):
        time.sleep(1)
