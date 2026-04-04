import rclpy
from rclpy.node import Node
import sys

from .ArdusimpleDevice import ArduSimpleDevice
from more_interfaces.msg import ArdusimpleValues

class GPSManagerNode(Node):
    def __init__(self):
        super().__init__('gps_manager')
        self.ardusimple_value_publisher_ = self.create_publisher(ArdusimpleValues, '/sensor/ardusimple_values', 10)
        self.ardu_simple_device = ArduSimpleDevice(None, "/dev/sensors/gps_config", self)
        

def main(args=None):
    rclpy.init(args=args)
    node = GPSManagerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()