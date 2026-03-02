import rclpy
from rclpy.node import Node
import gi
import glob
import logging
import struct
import subprocess
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GLib, GObject
from video_manager.VideoManager import VideoManager
from more_interfaces.msg import MavlinkPacket, MarinelinkPacket

class VideoControl(Node):
    def __init__(self):
        super().__init__('video_manager')
        
        self.subscriber_ = self.create_subscription(MarinelinkPacket, '/video/cmd', self.marinelink_callback, 10)
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        self.videoManager = VideoManager(self)
        

    def marinelink_callback(self, msg):
        self.get_logger().info(f'Received MarinelinkPacket: {msg}')
        payload_bytes = bytes(msg.payload)
        if msg.topic == 1:
            self.get_logger().info("[FORMAT]")
            formatList = self.videoManager.get_videoFormatList_legacy()
            if not formatList:
                self.get_logger().info("No video format available")
                return
            msg = b''
            for form in formatList:
                for video in formatList[form]:
                    videoIndex = video[0]
                    msg += struct.pack("<2B", videoIndex, form)
            self.get_logger().info(f"Publishing format list with {len(formatList)} formats")
            self.publisher_.publish(MarinelinkPacket(topic=1, payload=msg))
        elif msg.topic == 2:
            try:
                self.get_logger().info("[PLAY]")
                self.videoManager.handleMsg(payload_bytes, msg.address)
                operation = int(payload_bytes[0])
                self.get_logger().info(f"[PLAY] ok {operation}")
            except Exception as e:
                self.get_logger().warning(f"PLAY packet parse error: {e}")
        elif msg.topic == 3:
            self.get_logger().info("[QUIT]")
            try:
                video = int(payload_bytes[5:].decode())
                self.videoManager.stop(video)
                self.get_logger().info(f"Stopped video {video}")
            except Exception as e:
                self.get_logger().warning(f"QUIT packet parse error: {e}")



def main(args=None):
    rclpy.init(args=args)
    video_manager = VideoControl()
    rclpy.spin(video_manager)
    video_manager.destroy_node()
    rclpy.shutdown()