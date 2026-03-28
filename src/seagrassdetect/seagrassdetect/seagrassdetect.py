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

class SeagrassDetectNode(Node):
    def __init__(self, ):
        super().__init__('seagrass_detect')
        self.seagrassDetect = SeagrassDetect()
        self.seagrassDetect.startLoop()

        

def main(args=None):
    rclpy.init(args=args)
    seagrass_detect_node = SeagrassDetectNode()
    rclpy.spin(seagrass_detect_node)
    seagrass_detect_node.destroy_node()
    rclpy.shutdown()