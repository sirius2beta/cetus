import rclpy
from rclpy.node import Node
import glob
import logging
import struct
import subprocess
import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GLib, GObject

from video_manager.VideoManager import VideoManager
from more_interfaces.msg import MavlinkPacket, MarinelinkPacket
from std_msgs.msg import String

class VideoControl(Node):
    def __init__(self):
        super().__init__('video_manager')
        
        self.subscriber_ = self.create_subscription(MarinelinkPacket, '/control/video', self.marinelink_callback, 10)
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        self.seagrassCommandPublisher = self.create_publisher(String, '/control/seagrass/command', 10)
        self.jetsonDetectCommandPublisher = self.create_publisher(String, '/control/jetsondetect/command', 10)
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
            msg += struct.pack("<B", 0)  # command type
            # videoIndex running on port 5201
            viewport = int(payload_bytes[0])
            port = 5201 + viewport
            videoIndex = 0
            if port in self.videoManager.portOccupied:
                videoNo = self.videoManager.portOccupied[port]
                
                # iterate videomanager pipelines keys as videoNo to find the videoIndex
                for key in self.videoManager.pipelines:
                    if key == videoNo:
                        break
                    else:
                        videoIndex += 1
                    
            msg += struct.pack("<B", viewport)  # viewport
            msg += struct.pack("<B", videoIndex)  # video index

            for form in formatList:
                for video in formatList[form]:
                    videoIndex = video[0]
                    msg += struct.pack("<2B", videoIndex, form)
            self.get_logger().info(f"Publishing format list with {len(formatList)} formats")
            self.publisher_.publish(MarinelinkPacket(topic=1, payload=msg))
        elif msg.topic == 2:
            try:
                self.get_logger().info("[PLAY COMMAND (video command) future combine)]")
                self.videoManager.handleMsg(payload_bytes, msg.address)
                operation = int(payload_bytes[0])
                self.get_logger().info(f"[PLAY (video command)] topic {operation}")
            except Exception as e:
                self.get_logger().warning(f"PLAY packet parse error: {e}")


def main(args=None):
    rclpy.init(args=args)
    video_manager = VideoControl()
    rclpy.spin(video_manager)
    video_manager.destroy_node()
    rclpy.shutdown()