import os
from ament_index_python.packages import get_package_share_directory
import time
import csv
import cv2
import math
import subprocess
import torch
from torch2trt import TRTModule
from .unet import Unet
import re
from std_msgs.msg import String, Float32

import queue as _queue
import numpy as np
from PIL import Image
from datetime import datetime, timezone, timedelta
from pathlib import Path
import multiprocessing
import threading
import logging

class SeagrassDetect():
    def __init__(self,node=None):
        self.video_no = -1
        self.ready = False
        self.enabled = True
        self.format_setted = False
        self.is_playing = False
        self.node = node
        self.streaming = False
        self.recording = False
        self.os = 'None'
        self.width = 0
        self.height = 0
        self.fps = 0
        self.ip = ""
        self.port = 0
        

        self.model = None
        self.out_send = None
        self.cap_send = None
        self.video_pipeline = ""
        self.device_id = -1
        try:
            cmd = " grep '^VERSION_CODENAME=' /etc/os-release"
            returned_value = subprocess.check_output(cmd,shell=True,stderr=subprocess.DEVNULL).replace(b'\t',b'').decode("utf-8")
            if(len(returned_value) > 1):
                self.os = returned_value.split('=')[1].strip()
                logging.info(f"SeagrassDetect: Operating System: {self.os}")
        except:
            logging.error("SeagrassDetect: Failed to get OS information. Defaulting to 'None'.")
            return None
        self.encode_string = {
            'jammy': 'x264enc tune=zerolatency speed-preset=superfast',
            'focal': 'video/x-raw,format=I420 ! nvvideoconvert ! video/x-raw(memory:NVMM) ! nvv4l2h264enc'
        }.get(self.os, None)
        
        self.base_folder_path = "/home/sirius2beta/GPlayerLogNew/image"
        self.base_directory = os.path.expanduser(self.base_folder_path)
        os.makedirs(self.base_directory, exist_ok=True)

        # 找出所有 seagrass_xxxxx 資料夾
        existing_folders = [f for f in os.listdir(self.base_directory) if f.startswith("seagrass_")]
        indices = [int(re.search(r"seagrass_(\d+)", f).group(1)) for f in existing_folders if re.search(r"seagrass_(\d+)", f)]
        file_index = max(indices) + 1 if indices else 1
        
        self.image_directory = os.path.join(self.base_directory, f"seagrass_{file_index:06d}")
        os.makedirs(self.image_directory, exist_ok=True)


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
                self.cap_send = cv2.VideoCapture(self.video_pipeline, cv2.CAP_GSTREAMER)

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
            self.streaming = False
        elif msg[0] == "r":
            self.recording = True
        elif msg[0] == "s":
            self.recording = False



    def startLoop(self):
        if not self.encode_string:
            print("SeagrassDetect: Unsupported OS type")
            return
        try:
            self.load_model()
        except Exception as e:
            print(f"SeagrassDetect: Failed to load model: {e}")
            return
        threading.Thread(target=self.detectTask, daemon=True).start()
        print(" [O] SeagrassDetect initialized")

 

    def load_model(self):
        current_dir = os.path.dirname(os.path.abspath(__file__))
        package_share_path = get_package_share_directory('seagrassdetect')
        trt_path = os.path.join(package_share_path, "model", "seagrass_model_resnet50_trt.pth")
        model_path = os.path.join(package_share_path, "model", "seagrass_model_resnet50.pth")
        self.model = Unet(model_path=model_path, num_classes=2, backbone="resnet50",
                    input_shape=[512, 512], mix_type=0, cuda=True)

        if isinstance(self.model.net, torch.nn.DataParallel):
            self.model.net = self.model.net.module

        trt_model = TRTModule()
        trt_model.load_state_dict(torch.load(trt_path))
        self.model.net = trt_model
        print("SeagrassDetect: ✅ TensorRT model loaded!")
        torch.cuda.empty_cache()
        self.ready = True



    def reopen_camera(self, device_id, pipeline):
        while not os.path.exists(f"/dev/cetusvideo{device_id}"):
            time.sleep(3)
        cap = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)
        time.sleep(1)
        return cap

    
    def detectTask(self):
        last_infer_time = time.time()
        frame_count = 0
        while True:
            if not self.ready:
                time.sleep(0.1)
                continue
            if not (self.recording or self.streaming):
                time.sleep(0.05)
                continue

            if time.time() - last_infer_time < 0.2:
                time.sleep(0.02)
                continue
            last_infer_time = time.time()

            ret, frame = self.cap_send.read()

            file_name = ""
            if not ret or frame is None:
                print("SeagrassDetect: ⚠️ Camera disconnected...")
                self.cap_send = self.reopen_camera(self.device_id, self.video_pipeline)
                continue
            
            t1 = time.time()
            if self.recording:
                frame_count += 1
                file_name = f"{frame_count:07d}.jpg"
                cv2.imwrite(os.path.join(self.image_directory, file_name), frame)
                self.node.publisher.publish(String(data=os.path.basename(self.image_directory) + "/" + file_name))
            frame = cv2.resize(frame, (640, 480))
            image_pil = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            result_pil, mask = self.model.detect_image(image_pil, return_mask=True)

            seagrass_pixels = np.sum(mask == 0)
            ratio = seagrass_pixels / mask.size * 100
            latency = time.time() - t1

            result_bgr = cv2.cvtColor(np.array(result_pil), cv2.COLOR_RGB2BGR)
            result_bgr = cv2.resize(result_bgr, (int(self.width), int(self.height)))
            frame = cv2.resize(frame, (int(self.width), int(self.height)))
            cv2.putText(result_bgr, f"Seagrass: {ratio:.2f}%, Time: {latency:.2f}s",
                        (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            self.node.result_publisher.publish(Float32(data=ratio))
            
            if self.out_send.isOpened() and self.streaming:
                self.out_send.write(result_bgr)

        

    
