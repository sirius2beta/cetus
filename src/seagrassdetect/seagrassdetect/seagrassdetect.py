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

from more_interfaces.msg import VideoFormat
from std_msgs.msg import String, Float32

class SeagrassDetectNode(Node):
    def __init__(self, ):
        super().__init__('seagrass_detect')
        self.seagrassDetect = SeagrassDetect(self)
        self.seagrassDetect.startLoop()
        self.subscriber = self.create_subscription(String, '/control/seagrass/command', self.seagrassDetect.video_format_callback, 10)
        self.publisher = self.create_publisher(String, '/seagrass_detect/img_name', 10)
        self.result_publisher = self.create_publisher(Float32, '/seagrass_detect/result', 10)
def main(args=None):
    rclpy.init(args=args)
    seagrass_detect_node = SeagrassDetectNode()
    rclpy.spin(seagrass_detect_node)
    seagrass_detect_node.destroy_node()
    rclpy.shutdown()