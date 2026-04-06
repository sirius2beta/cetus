import os
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

# NMEA 0183 requires GGA, RMC, GST, HDT, PTNL(AVR) sentences to get all the data we need.

def safe_float(value, default=0.0):
    if value is None or value == '':
        return default
    try:
        return float(value)
    except (ValueError, TypeError):
        return default

class ArduSimpleDevice(Device):           
    def __init__(self, device_type, dev_path="", node=None):
        super().__init__(device_type, dev_path)
        self.node = node
        self.node.get_logger().info("ArdusimpleDevice: Connected to Ardusimple.") 
        self.data_lock = Lock()

        self.date = ""
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
            os._exit(1) # 退出碼非 0 會讓 Launch 知道這是不正常退出
            return
        
        threading.Thread(target = self.reader, daemon = True).start() # start the reader thread
        self.timer = self.node.create_timer(0.1, self.publish_callback)
    
    def reader(self):
        while True:
            try:
                raw_data = self.ser.readline()
                decode_data = raw_data.decode('utf-8').strip().lstrip('$')
                fields = decode_data.split(',')
                if('*' in fields[-1]):
                    checksum_data = fields[-1].split('*')
                    fields[-1] = checksum_data[0]
                    checksum = checksum_data[1]
                else:
                    checksum = None
                with self.data_lock:
                    if(fields[0] == "GPGST" or fields[0] == "GLGST" or fields[0] == "GNGST"):
                        # fields[0] = "GPGST"
                        # fields[1] = utc of position fix
                        # fields[2] = RMS value of the pseudorange residuals
                        # fields[3] = Standard deviation of semi-major axis of error ellipse
                        # fields[4] = Standard deviation of semi-minor axis of error ellipse
                        # fields[5] = Orientation of semi-major axis of error ellipse (degrees from true
                        # fields[6] = Standard deviation of latitude error
                        # fields[7] = Standard deviation of longitude error
                        # fields[8] = Standard deviation of altitude error
                        # fields[9] = Checksum

                        self.lat_acc = safe_float(fields[6], 0.0)
                        self.lon_acc = safe_float(fields[7], 0.0)
                        self.alt_acc = safe_float(fields[8], 0.0)

                    if(fields[0] == "GPRMC" or fields[0] == "GMRMC"):
                        # fields[0] = "GPRMC"
                        # fields[1] = utc of position fix
                        # fields[2] = status (A=active, V=void)
                        # fields[3] = Latitude
                        # fields[4] = N or S
                        # fields[5] = Longitude
                        # fields[6] = E or W
                        # fields[7] = Speed over ground in knots
                        # fields[8] = Track angle in degrees
                        # fields[9] = Date (ddmmyy)
                        # fields[10] = Magnetic variation (degrees)
                        # fields[11] = E or W (magnetic variation direction)
                        # fields[12] = Mode indicator (A=autonomous, D=differential
                        # fields[13] = Checksum
                        self.speed = safe_float(fields[7], 0.0)
                        self.date = fields[9]
                    
                    if(fields[0] == "GPHDT" or fields[0] == "GLHDT" or fields[0] == "GNHDT"):
                        # fields[0] = "GPHDT"
                        # fields[1] = heading in degrees
                        self.yaw = safe_float(fields[1], 0.0)
                    
                    if(fields[0] == "PTNL" and fields[1] == "AVR"):
                        # fields[0] = "PTNL"
                        # fields[1] = "AVR"
                        # fields[2] = utc of position fix
                        # fields[3] = yaw
                        # fields[4] = N/A
                        # fields[5] = tilt
                        # fields[6] = N/A
                        # fields[8] = range
                        # fields[9] = fix type: 0 = no fix, 1 = autonomous, 2 = RTK float, 3 = RTK fixed, 4 = DGPS
                        self.yaw = safe_float(fields[3], 0.0)
                        self.tilt = safe_float(fields[5], 0.0)                    

                    if(fields[0] == "GPGGA"):
                        # fields[0] = "GPGGA"
                        # fields[1] = utc of position fix
                        # fields[2] = Latitude
                        # fields[3] = N or S
                        # fields[4] = Longitude
                        # fields[5] = E or W
                        # fields[6] = GPS Quality
                        # fields[7] = Number of satellites being tracked
                        # fields[8] = HDOP
                        # fields[9] = Orthometric height
                        # fields[10] = M (unit of orthometric height)
                        # fields[11] = Geoid separation
                        # fields[12] = M (unit of geoid separation)
                        # fields[13] = Age of differential GPS data (empty if not used)
                        # fields[14] = Differential reference station ID (empty if not used)
                        # fields[15] = Checksum
                        
                        self.utc_time = fields[1]
                        self.lat = safe_float(fields[2], 0.0)
                        self.lon = safe_float(fields[4], 0.0)
                        self.alt = safe_float(fields[9], 0.0)
                        self.undulation = safe_float(fields[11], 0.0)
                        self.HDOP = safe_float(fields[8], 0.0)
                        self.VDOP = safe_float(fields[8], 0.0) # G
            
            except Exception as e:
                print(f"解析過程中斷: {e}")
                os._exit(1)
        
    def publish_callback(self):
        while True: 
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