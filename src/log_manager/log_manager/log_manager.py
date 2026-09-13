import rclpy
from rclpy.node import Node

import math
import json
from datetime import datetime, timedelta
import struct
import time

from more_interfaces.msg import MavlinkValues, MarinelinkPacket, AquaValues, WinchStatus, ArdusimpleValues, KBestValues
from septentrio_gnss_driver.msg import PVTGeodetic
from gps_msgs.msg import GPSFix
from std_msgs.msg import String, Float32
from rclpy.qos import DurabilityPolicy, QoSProfile, ReliabilityPolicy
from .DataLogger import DataLogger
from .config import Config
from .replay import LogReplay

node1_control_type = 2 # sonar control type: 2
node2_control_type = 0 # winch control type: 0

# 把 self 拿掉
def nmea_to_decimal(nmea_val):
    """
    將 NMEA GGA 的 DDMM.MMMMM 格式轉換為十進制度 (Decimal Degrees)
    """
    if not nmea_val:
        return 0.0
    
    sign = -1 if nmea_val < 0 else 1
    val = abs(nmea_val)
    
    degrees = int(val / 100)
    minutes = val - (degrees * 100)
    decimal_degrees = sign * (degrees + (minutes / 60.0))
    
    return decimal_degrees

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
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        self.config = Config()
        self.sensor_group_list = self.config.sensor_group_list
        self.simulation_mode = self.declare_parameter('simulation_mode', False).value
        self.replay_log = self.declare_parameter('replay_log', '').value

        if self.simulation_mode:
            if not self.replay_log:
                raise ValueError('simulation_mode requires the replay_log parameter')
            self.data_logger = None
            self.replay = LogReplay(self.replay_log)
            self.replay_index = 0
            replay_qos = QoSProfile(
                depth=1,
                reliability=ReliabilityPolicy.RELIABLE,
                durability=DurabilityPolicy.TRANSIENT_LOCAL)
            self.replay_row_publisher = self.create_publisher(
                String, '/simulation/current_log_row', replay_qos)
            # rclpy timers require a positive period.  This only delays the
            # first sample by 1 ms; subsequent samples use recorded timing.
            self.replay_timer = self.create_timer(0.001, self.replay_next)
            self.get_logger().info(
                'Simulation mode: replaying {} rows from {}; database logging and live flight data are disabled.'
                .format(len(self.replay.rows), self.replay_log))
            return

        self.subscriber_ = self.create_subscription(MavlinkValues, '/sensor/mavlink_values', self.mavlinkValues_callback, 10)
        self.aquastatus_subscriber_ = self.create_subscription(AquaValues, '/sensor/aqua_values', self.aquastatus_callback, 10)
        self.winchstatus_subscriber_ = self.create_subscription(WinchStatus, '/sensor/winch_status', self.winchstatus_callback, 10)
        self.ardusimple_subscriber_ = self.create_subscription(ArdusimpleValues, '/sensor/ardusimple_values', self.ardusimple_callback, 10)
        self.kbest_subscriber_ = self.create_subscription(KBestValues, '/sensor/kbest_values', self.kbest_callback, 10)
        self.seagrass_image_subscriber_ = self.create_subscription(String, '/seagrass_detect/img_name', self.seagrass_image_callback, 10)
        self.seagrass_result_subscriber_ = self.create_subscription(Float32, '/seagrass_detect/result', self.seagrass_result_callback, 10)
        self.data_logger = DataLogger()

    @staticmethod
    def _number(row, name, default=0):
        value = row.get(name)
        return default if value is None else value

    @staticmethod
    def _coordinate(value):
        """Convert decimal-degree legacy log coordinates to MAVLink E7 values."""
        value = float(value)
        return int(round(value * 10000000)) if abs(value) <= 180 else int(value)

    def replay_next(self):
        self.replay_timer.cancel()
        row = self.replay.rows[self.replay_index]
        self._publish_replay_row(row)
        delay = self.replay.delay_after(self.replay_index)
        self.replay_index += 1
        if delay is None:
            self.get_logger().info('Simulation replay completed.')
            return
        self.replay_timer = self.create_timer(max(delay, 0.001), self.replay_next)

    def _publish_replay_row(self, row):
        # Publish the unmodified database row so all demo clients present the
        # same sample currently being sent through the simulated sensors.
        self.replay_row_publisher.publish(String(data=json.dumps(row)))

        mav0 = self.sensor_group_list[3]
        mav1 = self.sensor_group_list[4]
        aqua = self.sensor_group_list[1]
        kbest = self.sensor_group_list[5]

        mav0.get_sensor(0).data = int(self._number(row, 'depth'))
        mav0.get_sensor(1).data = 0
        mav0.get_sensor(2).data = 0
        mav0.get_sensor(3).data = 0
        mav1.get_sensor(0).data = int(self._number(row, 'fix_type'))
        mav1.get_sensor(1).data = self._coordinate(self._number(row, 'lon'))
        mav1.get_sensor(2).data = self._coordinate(self._number(row, 'lat'))
        mav1.get_sensor(3).data = int(self._number(row, 'alt'))
        mav1.get_sensor(4).data = int(self._number(row, 'yaw'))
        mav1.get_sensor(5).data = float(self._number(row, 'pitch'))
        mav1.get_sensor(6).data = float(self._number(row, 'roll'))
        mav1.get_sensor(7).data = float(self._number(row, 'speed'))

        aqua_fields = (
            'temperature', 'pressure', 'aqua_depth', 'level_depth_to_water',
            'level_surface_elevation', 'actual_conductivity', 'specific_conductivity',
            'resistivity', 'salinity', 'total_dissolved_solids', 'density_of_water',
            'barometric_pressure', 'ph', 'ph_mv', 'orp',
            'dissolved_oxygen_concentration', 'dissolved_oxygen_saturation',
            'turbidity', 'oxygen_partial_pressure', 'external_voltage',
            'battery_capacity_remaining')
        for index, field in enumerate(aqua_fields):
            aqua.get_sensor(index).data = float(self._number(row, field))
        kbest.get_sensor(0).data = int(self._number(row, 'kbest_boat_rssi'))
        kbest.get_sensor(1).data = int(self._number(row, 'kbest_ground_rssi'))
        kbest.get_sensor(2).data = 0.0
        kbest.get_sensor(3).data = 0.0

        for group in (mav1, mav0, aqua, kbest):
            self.publisher_.publish(MarinelinkPacket(topic=4, payload=group.pack()))


    
    def seagrass_result_callback(self, msg):
        self.data_logger.log_data.seagrass_coverage_ratio = msg.data
    
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
        self.data_logger.log_data.gps_date = msg.date
        self.data_logger.log_data.gps_timestamp = msg.utc_time
        self.data_logger.log_data.lat = nmea_to_decimal(msg.latitude)
        self.data_logger.log_data.lon = nmea_to_decimal(msg.longitude)
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
