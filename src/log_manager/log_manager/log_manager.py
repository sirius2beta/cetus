import rclpy
from rclpy.node import Node

import math
from datetime import datetime, timedelta
import struct
import time

from more_interfaces.msg import MavlinkValues, MarinelinkPacket, AquaValues, WinchStatus, ArdusimpleValues, KBestValues
from septentrio_gnss_driver.msg import PVTGeodetic
from gps_msgs.msg import GPSFix
from std_msgs.msg import String
from .DataLogger import DataLogger
from .config import Config

node1_control_type = 2 # sonar control type: 2
node2_control_type = 0 # winch control type: 0

def gps_time_to_utc(wnc, tow):
    gps_start = datetime(1980, 1, 6)
    total_seconds = wnc * 7 * 86400 + tow/1000
    utc_time = gps_start + timedelta(seconds=total_seconds)

    LEAP_SECONDS = 18 
    utc_time -= timedelta(seconds=LEAP_SECONDS)

    return utc_time

def position_accuracy(cov_latlat, cov_lonlon, cov_heightheight):
    def safe_sqrt(x):
        return math.sqrt(x) if x >= 0 else math.sqrt(max(0, x))
    
    sigma_lat = safe_sqrt(cov_latlat)
    sigma_lon = safe_sqrt(cov_lonlon)
    sigma_h   = safe_sqrt(cov_heightheight)
    
    return sigma_lat, sigma_lon, sigma_h

class LogManager(Node):
    def __init__(self):
        super().__init__('log_manager')
        
        self.subscriber_ = self.create_subscription(
            MavlinkValues, 
            '/sensor/mavlink_values', 
            self.mavlinkValues_callback, 
            10
        )
        self.aquastatus_subscriber_ = self.create_subscription(
            AquaValues,
            '/sensor/aqua_values',
            self.aquastatus_callback,
            10
        )
        self.winchstatus_subscriber_ = self.create_subscription(
            WinchStatus,
            '/sensor/winch_status',
            self.winchstatus_callback,
            10
        )
        self.ardusimple_subscriber_ = self.create_subscription(
            ArdusimpleValues,
            '/sensor/ardusimple_values',
            self.ardusimple_callback,
            10
        )
        self.kbest_subscriber_ = self.create_subscription(
            KBestValues,
            '/sensor/kbest_values',
            self.kbest_callback,
            10
        )
        self.seagrass_image_subscriber_ = self.create_subscription(
            String,
            '/seagrass_detect/img_name',
            self.seagrass_image_callback,
            10
        )
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        #self.get_logger().info('LogManager has started and is listening to /sensor/mavlink_values')
        
        self.config = Config()
        self.sensor_group_list = self.config.sensor_group_list
        self.data_logger = DataLogger()

    def seagrass_image_callback(self, msg):
        self.data_logger.log_data.seagrass_image_name = msg.data

    def mavlinkValues_callback(self, msg):
        #self.get_logger().info(f'Received MavlinkValues - Yaw: {msg.yaw}, Pitch: {msg.pitch}, Roll: {msg.roll}')
        self.data_logger.log_data.fix_type = msg.fix_type
        self.data_logger.log_data.yaw = msg.yaw
        self.data_logger.log_data.pitch = msg.pitch
        self.data_logger.log_data.roll = msg.roll
        self.data_logger.log_data.speed = msg.groundspeed
        self.data_logger.log_data.depth = msg.depth

        self.sensor_group_list[4].get_sensor(0).data = msg.fix_type
        self.sensor_group_list[4].get_sensor(1).data = msg.lon
        self.sensor_group_list[4].get_sensor(2).data = msg.lat
        self.sensor_group_list[4].get_sensor(3).data = msg.alt
        self.sensor_group_list[4].get_sensor(4).data = msg.yaw
        self.sensor_group_list[4].get_sensor(5).data = msg.pitch
        self.sensor_group_list[4].get_sensor(6).data = msg.roll
        self.sensor_group_list[4].get_sensor(7).data = msg.groundspeed
        
        self.sensor_group_list[3].get_sensor(0).data = msg.depth
        self.sensor_group_list[3].get_sensor(1).data = msg.voltage_battery
        self.sensor_group_list[3].get_sensor(2).data = msg.current_battery
        self.sensor_group_list[3].get_sensor(3).data = msg.battery_remaining    
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[4].pack()))
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[3].pack()))
 
    def aquastatus_callback(self, msg):
        self.data_logger.log_data.temperature = msg.temperature
        self.data_logger.log_data.pressure = msg.pressure
        self.data_logger.log_data.aqua_depth = msg.depth
        self.data_logger.log_data.level_depth_to_water = msg.level_depth_to_water
        self.data_logger.log_data.level_surface_elevation = msg.level_surface_elevation
        self.data_logger.log_data.actual_conductivity = msg.actual_conductivity
        self.data_logger.log_data.specific_conductivity = msg.specific_conductivity
        self.data_logger.log_data.resistivity = msg.resistivity
        self.data_logger.log_data.salinity = msg.salinity
        self.data_logger.log_data.total_dissolved_solids = msg.total_dissolved_solids
        self.data_logger.log_data.density_of_water = msg.density_of_water
        self.data_logger.log_data.barometric_pressure = msg.barometric_pressure
        self.data_logger.log_data.ph = msg.ph
        self.data_logger.log_data.ph_mv = msg.ph_mv
        self.data_logger.log_data.orp = msg.orp
        self.data_logger.log_data.dissolved_oxygen_concentration = msg.dissolved_oxygen_concentration
        self.data_logger.log_data.dissolved_oxygen_saturation = msg.dissolved_oxygen_saturation
        self.data_logger.log_data.turbidity = msg.turbidity
        self.data_logger.log_data.oxygen_partial_pressure = msg.oxygen_partial_pressure
        self.data_logger.log_data.external_voltage = msg.external_voltage
        self.data_logger.log_data.battery_capacity_remaining = msg.battery_capacity_remaining

        self.sensor_group_list[1].get_sensor(0).data = msg.temperature
        self.sensor_group_list[1].get_sensor(1).data = msg.pressure
        self.sensor_group_list[1].get_sensor(2).data = msg.depth
        self.sensor_group_list[1].get_sensor(3).data = msg.level_depth_to_water
        self.sensor_group_list[1].get_sensor(4).data = msg.level_surface_elevation
        self.sensor_group_list[1].get_sensor(5).data = msg.actual_conductivity
        self.sensor_group_list[1].get_sensor(6).data = msg.specific_conductivity
        self.sensor_group_list[1].get_sensor(7).data = msg.resistivity
        self.sensor_group_list[1].get_sensor(8).data = msg.salinity
        self.sensor_group_list[1].get_sensor(9).data = msg.total_dissolved_solids
        self.sensor_group_list[1].get_sensor(10).data = msg.density_of_water
        self.sensor_group_list[1].get_sensor(11).data = msg.barometric_pressure
        self.sensor_group_list[1].get_sensor(12).data = msg.ph
        self.sensor_group_list[1].get_sensor(13).data = msg.ph_mv
        self.sensor_group_list[1].get_sensor(14).data = msg.orp
        self.sensor_group_list[1].get_sensor(15).data = msg.dissolved_oxygen_concentration
        self.sensor_group_list[1].get_sensor(16).data = msg.dissolved_oxygen_saturation
        self.sensor_group_list[1].get_sensor(17).data = msg.turbidity
        self.sensor_group_list[1].get_sensor(18).data = msg.oxygen_partial_pressure
        self.sensor_group_list[1].get_sensor(19).data = msg.external_voltage
        self.sensor_group_list[1].get_sensor(20).data = msg.battery_capacity_remaining
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[1].pack()))
        #self.get_logger().info(f"Updated AquaValues - Temp: {msg.temperature}")
    
    def winchstatus_callback(self, msg):
        step = msg.step
        tension = msg.tension
        status = msg.status

        data = struct.pack("<B", node2_control_type)
        data += struct.pack("<B", 8)
        data += struct.pack("<i", step)
        data += struct.pack("<i", tension)
        data += struct.pack("<B", status)
        self.publisher_.publish(MarinelinkPacket(topic=5, payload=data))
        #self.get_logger().info(f"Updated WinchStatus - Step: {step}, Tension: {tension}, Status: {status}")
    
    def ardusimple_callback(self, msg):
        self.data_logger.log_data.timestamp = msg.utc_time
        self.data_logger.log_data.lat = msg.latitude
        self.data_logger.log_data.lon = msg.longitude
        self.data_logger.log_data.alt = msg.height
        self.data_logger.log_data.HDOP = msg.hdop
        self.data_logger.log_data.VDOP = msg.vdop
        self.data_logger.log_data.lon_acc = msg.lon_acc
        self.data_logger.log_data.lat_acc = msg.lat_acc
        self.data_logger.log_data.alt_acc = msg.alt_acc
        self.data_logger.log_data.gps_speed = msg.speed
        self.data_logger.log_data.gps_tilt = msg.tilt
        self.data_logger.log_data.gps_yaw = msg.yaw
        self.data_logger.log_data.gps_orthometric_height = msg.height - msg.undulation
        self.data_logger.log_data.geoid_separation = msg.undulation
    
    def kbest_callback(self, msg):
        self.sensor_group_list[5].get_sensor(0).data = msg.kbest_boat_rssi
        self.sensor_group_list[5].get_sensor(1).data = msg.kbest_ground_rssi
        self.sensor_group_list[5].get_sensor(2).data = msg.tx_rate
        self.sensor_group_list[5].get_sensor(3).data = msg.rx_rate
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[5].pack()))
        self.data_logger.log_data.kbest_boat_rssi = msg.kbest_boat_rssi
        self.data_logger.log_data.kbest_ground_rssi = msg.kbest_ground_rssi
    
    def seagrass_image_callback(self, msg):
        self.data_logger.log_data.seagrass_image_name = msg.data
def main(args=None):
    rclpy.init(args=args)
    log_manager = LogManager()
    try:
        rclpy.spin(log_manager)
    except KeyboardInterrupt:
        pass
    finally:
        log_manager.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()