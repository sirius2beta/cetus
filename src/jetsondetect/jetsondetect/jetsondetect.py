import rclpy
from rclpy.node import Node
from ament_index_python.packages import get_package_share_directory
import os

from more_interfaces.msg import MarinelinkPacket, MavlinkValues

from std_msgs.msg import String, Float32
from .JetsonDetect import JetsonDetect

class JetsonDetectNode(Node):
    def __init__(self, ):
        super().__init__('jetson_detect')
        self.JetsonDetect = JetsonDetect(self)
        
        self.subscriber = self.create_subscription(String, '/control/jetsondetect/command', self.JetsonDetect.video_format_callback, 10)
        self.result_publisher = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        self.mavlink_subscriber = self.create_subscription(
            MavlinkValues, 
            '/sensor/mavlink_values', 
            self.JetsonDetect.mavlinkValues_callback, 
            10
        )
        
        self.JetsonDetect.startLoop()
def main(args=None):
    rclpy.init(args=args)
    jetson_detect_node = JetsonDetectNode()
    rclpy.spin(jetson_detect_node)
    jetson_detect_node.destroy_node()
    rclpy.shutdown()