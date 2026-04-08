import argparse
from pathlib import Path
import threading
import time
import multiprocessing 
import math
import struct
from scipy.spatial.transform import Rotation
import cv2
import numpy as np
import queue as _queue  # 用於捕捉 Full
import logging
from ultralytics import YOLO
from models.torch_utils import det_postprocess
from models.utils import blob, letterbox, path_to_list
import os
from ament_index_python.packages import get_package_share_directory

from config import CLASSES, COLORS
from GTool import GTool
from distance import distance, getR0
from more_interfaces.msg import MarinelinkPacket

send_result_interval = 0.1

class JetsonDetect():
    def __init__(self, node):
        self.video_no = -1
        self.enabled = True
        self.cap_send = None
        self.out_send = None
        self.w = 0
        self.h = 0
        self.streaming = False
        self.engine = ''
        self.encode_string = ''

        current_dir = os.path.dirname(os.path.abspath(__file__))
        package_share_path = get_package_share_directory('jetsondetect')
        model_path = os.path.join(package_share_path, "model", "seagrass_model_resnet50.pth")
        if os == 'jammy': # Jetson orin nano
            self.engine = os.path.join(current_dir, "engine", "yolov8s_orin.engine")
            self.encode_string = 'x264enc tune=zerolatency speed-preset=superfast'
        elif os == 'focal': # Jetson xavier
            self.engine = os.path.join(current_dir, "engine", "yolov8s_xavier.engine")
            self.encode_string = 'video/x-raw,format=I420 ! nvvideoconvert ! video/x-raw(memory:NVMM) ! nvv4l2h264enc'
        else:
            exit(0)
        
    def video_format_callback(self, stringmsg):
        msg = stringmsg.data
        self.node.get_logger().info(f"SeagrassDetect: Received message: {msg}")
        if msg[0] == "f":
            device_id, vformat, width, height, fps, encoder, ip, port = msg.split(" ")[1:]
            if self.device_id == device_id and self.width == width and self.height == height and self.fps == fps and self.ip == ip and self.port == port:
                print("SeagrassDetect: Video format unchanged, skipping reinitialization.")
            else:    
                self.device_id = device_id
                self.width = width
                self.height = height
                self.fps = fps
                self.ip = ip
                self.port = port   
            
                if vformat == "MJPG":
                    self.video_pipeline = (
                        f'v4l2src device=/dev/cetusvideo{self.device_id} ! '
                        f'image/jpeg, width={self.width}, height={self.height}, framerate={self.fps}/1 ! '
                        f'jpegdec ! videoconvert ! appsink drop=True max-buffers=1'
                    )
                else: # default to YUYV
                    self.video_pipeline = (
                        f'v4l2src device=/dev/cetusvideo{self.device_id} ! '
                        f'video/x-raw, format=YUY2, width={self.width}, height={self.height}, framerate={self.fps}/1 ! '
                        f'videoconvert ! appsink drop=True max-buffers=1'
                    )
                if self.cap_send is not None:
                    self.cap_send.release()
                self.cap_send = cv2.VideoCapture(self.video_pipeline, cv2.CAP_GSTREAMER)
            if self.out_send is not None:
                self.out_send.release()
            self.out_send = cv2.VideoWriter(
                f'appsrc ! videoconvert ! {self.encode_string} ! rtph264pay pt=96 config-interval=1 ! udpsink host={self.ip} port={self.port}',
                cv2.CAP_GSTREAMER, 0, int(self.fps), (int(self.width), int(self.height)), True
            )


        elif msg[0] == "p":
            self.out_send = cv2.VideoWriter(
                f'appsrc ! videoconvert ! {self.encode_string} ! rtph264pay pt=96 config-interval=1 ! udpsink host={self.ip} port={self.port}',
                cv2.CAP_GSTREAMER, 0, int(self.fps), (int(self.width), int(self.height)), True
            )
            self.streaming = True
        
        elif msg[0] == "x":
            if self.cap_send is not None:
                self.cap_send.release()
            if self.out_send is not None:
                self.out_send.release()
            self.streaming = False
        elif msg[0] == "r":
            self.recording = True
        elif msg[0] == "s":
            self.recording = False
        elif msg[0] == "i":
            pitch = float(msg[1])  # 直接使用 pitch
            roll = float(msg[2])   # 直接使用 roll
            R0 = getR0(pitch, roll)
            #print(R0)
    
    def updateIMU(self, msg): #[pitch, roll]
        msg.insert(0, "i")
        self.in_conn.put(msg)
   
    def sendDetectionResult(self, results):
        data = struct.pack("<B", 1) #cmd id
        if self.video_no == -1:
            return
        data += struct.pack("<B", int(self.video_no)) #video no
        for result in results:
            data += struct.pack("<B", result[0])
            data += struct.pack("<H", result[1])
            data += struct.pack("<H", result[2])
            data += struct.pack("<H", result[3])
            data += struct.pack("<H", result[4])
            data += struct.pack("<f", result[5])
            data += struct.pack("<f", result[6])
        self.node.result_publisher.publish(MarinelinkPacket(topic=6, payload=data))

    def startLoop(self):
        if not self.encode_string:
            print("JetsonDetect: Unsupported OS type")
            return
        try:
            self.load_model()
        except Exception as e:
            print(f"JetsonDetect: Failed to load model: {e}")
            return
        threading.Thread(target=self.detectTask, daemon=True).start()
        print(" [O] JetsonDetect initialized")
    
    def load_model(self):
        self.model = YOLO(self.engine)

    def reopen_camera(self, device_id, video_pipeline):
        while not os.path.exists(f"/dev/cetusvideo{device_id}"):
            time.sleep(3)
        if self.cap_send is not None:
            self.cap_send.release()
        self.cap_send = cv2.VideoCapture(video_pipeline, cv2.CAP_GSTREAMER)
        time.sleep(1)

    def detectTask(self): # Thread that read data from oak camera
        K0 = np.array(
            [[1000, 0., 320],
            [0., 1000, 240],
            [0., 0., 1.]]
        )
        cam_height = 0.4
        R0 = getR0(0, 0)
        self.last_send_time = time.time()
        while True:  
            if not (self.recording or self.streaming):
                time.sleep(0.05)
                continue

            ret,frame = self.cap_send.read()
            if not ret or frame is None:
                print('JetsonDetect: empty frame')
                self.reopen_camera(self.device_id, self.video_pipeline)
                continue
            
            results = self.model.predict(frame, conf=0.5, verbose=False)
            count = 0
            boxes = results[0].boxes.xyxy
            cls_name = results[0].names
            classes = results[0].boxes.cls
            depth = 0
            detect_matrix = []
            for box, clas in zip(boxes,classes):
                x1, y1, x2, y2 = box
                coord = distance(K0, R0, cam_height, int(x1)+(int(x2)-int(x1))/2, int(y2))
                detect_matrix.append([int(clas), int(x1), int(y1), int(x2)-int(x1), int(y2)-int(y1), coord[0], coord[1]])

            if self.out_send.isOpened():
                self.out_send.write(frame)
            if time.time() - self.last_send_time > send_result_interval:
                self.last_send_time = time.time()
                self.sendDetectionResult(detect_matrix)

        self.out_send.release()
        self.cap_send.release()
