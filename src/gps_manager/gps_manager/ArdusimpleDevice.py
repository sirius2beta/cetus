import time
import serial
import threading
from threading import Lock
from pysbf2.sbfreader import SBFReader
from pysbf2.sbftypes_core import SBF_PROTOCOL, NMEA_PROTOCOL
import math
from datetime import datetime, timedelta
import rclpy


from .Device import Device
from more_interfaces.msg import ArdusimpleValues

def safe_float(value, default=0.0):
    if value is None or value == '':
        return default
    try:
        return float(value)
    except (ValueError, TypeError):
        return default

def gps_time_to_utc(wnc, tow):
    gps_start = datetime(1980, 1, 6)
    total_seconds = wnc * 7 * 86400 + tow/1000
    utc_time = gps_start + timedelta(seconds=total_seconds)

    # 注意：GPS 時間比 UTC 多 18 秒（截至目前），要減去 leap seconds
    LEAP_SECONDS = 18  # 根據當前標準，可能會變
    utc_time -= timedelta(seconds=LEAP_SECONDS)

    return utc_time

def position_accuracy(cov_latlat, cov_lonlon, cov_heightheight):
    def safe_sqrt(x):
        # 如果因浮點誤差導致略小於0，強制設為0
        return math.sqrt(x) if x >= 0 else math.sqrt(max(0, x))
    
    sigma_lat = safe_sqrt(cov_latlat)
    sigma_lon = safe_sqrt(cov_lonlon)
    sigma_h   = safe_sqrt(cov_heightheight)
    
    return sigma_lat, sigma_lon, sigma_h

class ArduSimpleDevice(Device):           
    def __init__(self, device_type, dev_path="", node=None):
        super().__init__(device_type, dev_path)
        self.node = node
        self.data_lock = Lock()

        self.utc_time = ""
        self.lon = 0.0
        self.lat = 0.0
        self.alt = 0.0
        self.undulation = 0.0
        self.lon_acc = 0.0
        self.lat_acc = 0.0
        self.alt_acc = 0.0
        self.HDOP = 0.0
        self.VDOP = 0.0
        self.tilt = 0.0
        self.yaw = 0.0
        self.speed = 0.0
        try:
            self.ser = serial.Serial(port=self.dev_path, baudrate=115200, timeout=2)
        except Exception as e:
            node.get_logger().error(f"無法開啟串口 {self.dev_path}: {e}")
            return

        threading.Thread(target = self.reader, daemon = True).start() # start the reader thread
        threading.Thread(target = self.log_data, daemon = True).start() # start the logger thread
        self.node = node
        self.node.get_logger().info("ArdusimpleDevice: Connected to Ardusimple.") 
        
    def reader(self):
        try:
            reader = SBFReader(self.ser, protfilter=SBF_PROTOCOL | NMEA_PROTOCOL)
            for raw, msg in reader:
                # 使用鎖來保護數據寫入
                with self.data_lock:
                    if msg.identity == 'PVTGeodetic':
                        utc = gps_time_to_utc(msg.WNc, msg.TOW)
                        self.utc_time = utc + timedelta(hours=8)
                        self.lat = math.degrees(msg.Latitude)
                        self.lon = math.degrees(msg.Longitude)
                        self.alt = msg.Height
                        self.undulation = msg.Undulation
                        
                    elif msg.identity == 'DOP':
                        self.HDOP = float(msg.HDOP) if msg.HDOP is not None else 0.0
                        self.VDOP = float(msg.VDOP) if msg.VDOP is not None else 0.0
                        
                    elif msg.identity == 'PosCovGeodetic':
                        # 計算精度
                        self.lat_acc, self.lon_acc, self.alt_acc = position_accuracy(
                            msg.Cov_latlat, msg.Cov_lonlon, msg.Cov_hgthgt
                        )   
                    elif msg.identity == 'PTNLAVR': # 這通常是 NMEA
                        self.tilt = safe_float(msg.tilt, 0.0)
                        self.yaw = safe_float(msg.yaw, 0.0)
                        
                    elif msg.identity == 'GPRMC':
                        self.speed = safe_float(msg.spd, 0.0)
        except Exception as e:
            self.node.get_logger().error(f"解析過程中斷: {e}")
        
    def log_data(self):
        while True: 
            # 使用鎖來保護數據讀取
            with self.data_lock:
                msg = ArdusimpleValues()
                msg.utc_time = str(self.utc_time)
                msg.latitude = self.lat
                msg.longitude = self.lon
                msg.height = self.alt
                msg.undulation = self.undulation
                msg.hdop = self.HDOP
                msg.vdop = self.VDOP
                msg.lat_acc = self.lat_acc
                msg.lon_acc = self.lon_acc
                msg.alt_acc = self.alt_acc
                msg.tilt = self.tilt
                msg.yaw = self.yaw
                msg.speed = self.speed
            
            # 發布數據
            self.node.ardusimple_value_publisher_.publish(msg)
            time.sleep(0.2) # 提升到 10Hz，更適合無人船控制