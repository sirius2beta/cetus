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
from .models.torch_utils import det_postprocess
from .models.utils import blob, letterbox, path_to_list
import os
import re
from ament_index_python.packages import get_package_share_directory

from more_interfaces.msg import MarinelinkPacket

send_result_interval = 0.1
shot_interval = 1

from scipy.spatial.transform import Rotation
import math
import numpy as np
import subprocess

def safe_float(value, default=0.0):
    if value is None or value == '':
        return default
    try:
        return float(value)
    except (ValueError, TypeError):
        return default

def distance(K, R, h, u, v):
    uv = np.array([[u], [v], [1]])
    Pc = np.linalg.solve(K, uv)  # K 逆矩陣計算 Pc = np.matmul(np.linalg.inv(K), uv)
    Pw = R.T @ Pc  # 轉換到世界座標 Pw = np.matmul(np.linalg.inv(R), Pc)
    
    scale = h / Pw[-1]  # 讓 Z = 0
    Pw_ground = -Pw * scale  

    #dist = np.linalg.norm(Pw_ground[:2])  # 只考慮 X, Y 平面上的距離
    return Pw_ground

def getR0(pitch, roll):
    """
    計算相機的旋轉矩陣
    :param pitch: 俯仰角（與水平面的夾角，向上為正）
    :param roll: 翻滾角（與水平面的夾角，順時針為正）
    :return: 旋轉矩陣 (3x3)
    """
    # 先繞 X 軸旋轉 -90°，使得相機的初始 Z 軸對應世界 Y 軸
    R_base = np.array([
        [ 0,  0,  1],
        [-1,  0,  0],
        [ 0, -1,  0]
    ], dtype=np.float64)

    # 2. 載具的姿態旋轉 (Roll 繞 X 軸, Pitch 繞 Y 軸)
    # 注意：這裡使用外在旋轉 'XY' 或是根據載具的歐拉角順序。通常航太標準是 ZYX (Yaw, Pitch, Roll)
    # 若假設先 Roll 再 Pitch，我們繞 X 軸轉 roll，繞 Y 軸轉 pitch
    R_attitude = Rotation.from_euler('yx', [pitch, roll], degrees=False).as_matrix()
    # 總旋轉矩陣 = 基準旋轉 * Pitch-Roll 旋轉
    return R_attitude @ R_base

class JetsonDetect():
    def __init__(self, node):
        self.node = node
        self.device_id = -1
        self.ready = False
        self.cap_send = None
        self.out_send = None
        self.width = 0
        self.height = 0
        self.streaming = False
        self.recording = False

        self.engine = ''
        self.encode_string = ''
        self.R0 = getR0(0, 0)

        current_dir = os.path.dirname(os.path.abspath(__file__))
        package_share_path = get_package_share_directory('jetsondetect')
        
        # create snapshot directory
        self.base_folder_path = f"/home/sirius2beta/GPlayerLogNew/snapshot/jetsondetect"
        self.base_directory = os.path.expanduser(self.base_folder_path)
        os.makedirs(self.base_directory, exist_ok=True)
        existing_folders = [f for f in os.listdir(self.base_directory) if f.startswith("record_")]
        indices = [int(re.search(r"record_(\d+)", f).group(1)) for f in existing_folders if re.search(r"record_(\d+)", f)]
        file_index = max(indices) + 1 if indices else 1
        
        self.image_directory = os.path.join(self.base_directory, f"record_{file_index:06d}")
        os.makedirs(self.image_directory, exist_ok=True)

        self.OS = 'None'
        try:
            cmd = " grep '^VERSION_CODENAME=' /etc/os-release"
            returned_value = subprocess.check_output(cmd,shell=True,stderr=subprocess.DEVNULL).replace(b'\t',b'').decode("utf-8") 
        except:
            self.node.get_logger().error("Failed to get OS information. Defaulting to 'None'.")
            returned_value = ''
        if(len(returned_value) > 1): 
            self.OS = returned_value.split('=')[1].strip()
            self.node.get_logger().info(f" # Operating System: {self.OS}")
        if self.OS == 'jammy': # Jetson orin nano
            self.engine = os.path.join(package_share_path, "model", "yolov8s_orin.engine")
            self.encode_string = 'x264enc bitrate=1000 tune=zerolatency speed-preset=superfast'
        elif self.OS == 'focal': # Jetson xavier
            self.engine = os.path.join(package_share_path, "model", "yolov8s_xavier.engine")
            self.encode_string = 'video/x-raw,format=I420 ! nvvideoconvert ! video/x-raw(memory:NVMM) ! nvv4l2h264enc'
        else:
            self.node.get_logger().error("JetsonDetect: Unsupported OS type")
            exit(0)
        

    def load_model(self):
            self.sendAIModelStatus()
            self.model = YOLO(self.engine, task='detect')
            self.ready = True
            self.sendAIModelStatus()

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
            self.sendAIModelStatus()

    def mavlinkValues_callback(self, msg):
        pitch = safe_float(msg.pitch)
        roll = safe_float(msg.roll)
        self.R0 = getR0(pitch, roll)

    def sendAIModelStatus(self):
        data = struct.pack("<B", 3) #cmd id
        data += struct.pack("<B", 1)
        data += struct.pack("<B", int(self.ready)) #video no


        self.node.result_publisher.publish(MarinelinkPacket(topic=1, payload=data))
        
    def sendDetectionResult(self, results):

        data = struct.pack("<B", 1) #cmd id
        if self.device_id == -1:
            return
        data += struct.pack("<B", int(self.device_id)) #video no
        count = 0
        if count < 14:
            for result in results:
                data += struct.pack("<B", result[0])
                data += struct.pack("<H", result[1])
                data += struct.pack("<H", result[2])
                data += struct.pack("<H", result[3])
                data += struct.pack("<H", result[4])
                data += struct.pack("<f", result[5])
                data += struct.pack("<f", result[6])
            count+=1

        self.node.result_publisher.publish(MarinelinkPacket(topic=6, payload=data))
        return len(data)
    def startLoop(self):
        try:
            self.load_model()
        except Exception as e:
            self.node.get_logger().error(f"JetsonDetect: Failed to load model: {e}")
            exit(0)
        self.node.get_logger().info("JetsonDetect: Model loaded successfully")
        threading.Thread(target=self.detectTask, daemon=True).start()
        self.node.get_logger().info(" [O] JetsonDetect loop started")
    
    
    def reopen_camera(self, device_id, video_pipeline):
        while not os.path.exists(f"/dev/cetusvideo{device_id}"):
            time.sleep(3)
        
        if self.cap_send is not None:
            self.cap_send.release()
            time.sleep(0.5) # Give the OS time to release the V4L2 handle
        if not (self.recording or self.streaming):
            return
        self.cap_send = cv2.VideoCapture(video_pipeline, cv2.CAP_GSTREAMER)
        time.sleep(1)

    def detectTask(self): # Thread that read data from oak camera
        K0 = np.array(
            [[1000, 0., 320],
            [0., 1000, 240],
            [0., 0., 1.]]
        )
        cam_height = 0.4
        packet_size = 0
        frame_count = 0
        self.last_send_time = time.time()
        self.last_shot_time = time.time()
        while True:  
            if not (self.recording or self.streaming):
                time.sleep(0.05)
                continue
            t1 = time.time()
            ret,frame = self.cap_send.read()
            if not ret or frame is None:
                print('JetsonDetect: empty frame')
                self.reopen_camera(self.device_id, self.video_pipeline)
                continue
            if time.time() - self.last_send_time > send_result_interval:
                self.last_send_time = time.time()
            if self.recording and (time.time() - self.last_shot_time) > shot_interval:
                self.last_shot_time = time.time()
                frame_count += 1
                file_name = f"{frame_count:07d}.jpg"
                cv2.imwrite(os.path.join(self.image_directory, file_name), frame)

            results = self.model.predict(frame, conf=0.5, verbose=False)
            count = 0
            boxes = results[0].boxes.xyxy
            cls_name = results[0].names
            classes = results[0].boxes.cls
            depth = 0
            detect_matrix = []
            test = False
            if test:
                x1, y1, x2, y2 = [100,100,200,200]
                detect_matrix.append([int(1), int(x1), int(y1), int(x2)-int(x1), int(y2)-int(y1), 10, 10])
                cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
            else:
                for box, clas in zip(boxes,classes):
                    x1, y1, x2, y2 = box
                    coord = distance(K0, self.R0, cam_height, int(x1)+(int(x2)-int(x1))/2, int(y2))
                    detect_matrix.append([int(clas), int(x1), int(y1), int(x2)-int(x1), int(y2)-int(y1), coord[0], coord[1]])
                    #cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
            t2 = time.time()
            
            if time.time() - self.last_send_time > send_result_interval:
                self.last_send_time = time.time()
                packet_size = self.sendDetectionResult(detect_matrix)
            latency = t2 - t1
            cv2.putText(frame, f"Time: {latency:.2f}s, packet: {packet_size} bytes",
            (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            
            if self.out_send.isOpened():
                self.out_send.write(frame)
            

        self.out_send.release()
        self.cap_send.release()
