import rclpy
import sys

from rclpy.node import Node
from .ArdusimpleDevice import ArduSimpleDevice
from more_interfaces.msg import ArdusimpleValue

class GPSManagerNode(Node):
    def __init__(self):
        super().__init__('gps_manager')
        self.ardusimple_value_publisher_ = self.create_publisher(ArdusimpleValue, '/sensor/ardusimple_value', 10)
        self.ardu_simple_device = ArduSimpleDevice(None, "/dev/sensors/gps_data", self)
        

def main(args=None):
    rclpy.init(args=args)
    node = GPSManagerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()