import rclpy
from rclpy.node import Node
import gi
import glob
import logging
import subprocess
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GLib, GObject
from video_manager.VideoManager import VideoManager
from more_interfaces.msg import MavlinkPacket, MarinelinkPacket

class VideoControl(Node):
    def __init__(self):
        super().__init__('video_manager')
        self.subscriber_ = self.create_subscription(MarinelinkPacket, 'video/cmd', self.marinelink_callback, 10)
        

    def marinelink_callback(self, msg):
        self.get_logger().info(f'Received MarinelinkPacket: {msg}')


def main(args=None):
    rclpy.init(args=args)
    video_manager = VideoControl()
    rclpy.spin(video_manager)
    video_manager.destroy_node()
    rclpy.shutdown()