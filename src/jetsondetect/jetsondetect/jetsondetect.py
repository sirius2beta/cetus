import rclpy
from rclpy.node import Node
from ament_index_python.packages import get_package_share_directory
import os
import time
import csv
import cv2
import math
import torch
from torch2trt import TRTModule
from .unet import Unet
from .SeagrassDetect import SeagrassDetect

import numpy as np
from PIL import Image
from datetime import datetime, timezone, timedelta
from pathlib import Path
import multiprocessing
import threading
import logging

from more_interfaces.msg import VideoFormat, MarinelinkPacket
from std_msgs.msg import String, Float32

class JetsonDetectNode(Node):
    def __init__(self, ):
        super().__init__('jetson_detect')
        self.JetsonDetect = JetsonDetect(self)
        self.JetsonDetect.startLoop()
        self.subscriber = self.create_subscription(String, '/control/jetsondetect/command', self.JetsonDetect.video_format_callback, 10)
        self.result_publisher = self.create_publisher(MarinelinkPacket, '/marinlink_tosend', 10)

def main(args=None):
    rclpy.init(args=args)
    jetson_detect_node = JetsonDetectNode()
    rclpy.spin(jetson_detect_node)
    jetson_detect_node.destroy_node()
    rclpy.shutdown()