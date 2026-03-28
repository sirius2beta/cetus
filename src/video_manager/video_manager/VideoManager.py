import gi
import glob
import logging
import subprocess
import threading
import re
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GLib, GObject


import video_manager.VideoFormat as VideoFormat

def getOS():
	try:
		cmd = " grep '^VERSION_CODENAME=' /etc/os-release"
		returned_value = subprocess.check_output(cmd,shell=True,stderr=subprocess.DEVNULL).replace(b'\t',b'').decode("utf-8") 
		if(len(returned_value) > 1): 
			return returned_value.split('=')[1].strip()
		else:
			return 'None'
	except:
		print("Failed to get OS information. Defaulting to 'None'.")
		return 'None'


class VideoManager():
	def __init__(self, node):
		self.node = node
		self.sys = 'buster'


		# pipeline 管理：key=cam index, value=dict
		self.pipelines = {}  # { cam: {"pipeline": Gst.Pipeline, "state": bool, "port": int, "encoder": str} }
		self.camera_format = []
		self.videoFormatList = {}
		self.videoWithYUYV = []
		self.portOccupied = {}  # {port: cam}
		

		self.get_video_format_generic()

		self.portOccupied = {} # {port, videoNo}

		GObject.threads_init()
		
		Gst.init(None)
		self.loop = GLib.MainLoop()
		self.loop_thread = threading.Thread(target=self.loop.run)
		self.loop_thread.daemon = True  # 設為守護進程，主程式關閉時它會自動結束
		self.loop_thread.start()
		
		print("[o] VideoManager: started")

	def getFormatInfoByIndex(self, formatIndex):
		formatMap = {
			0: (1920, 1080, 30),
			1: (1280, 720, 30),
			2: (640, 480, 30),
			3: (320, 240, 30),
		}
		return formatMap.get(formatIndex, None)  # 找不到回 None
	
	def getFormatInfoByIndexMap(self):
		return {
			0: (1920, 1080, 30),
			1: (1280, 720, 30),
			2: (640, 480, 30),
			3: (320, 240, 30),
		}

	#get video format from existing camera devices
	def get_video_format_generic(self):
		"""掃描 /dev/video*，只保留 MJPEG>YUYV，h264 不加入"""
		devices = sorted(glob.glob("/dev/video*"))
		if not devices:
			print("[x] no video devices found")
			return

		for dev in devices:
			try:
				cam = int(dev.replace("/dev/video", ""))
			except ValueError:
				continue

			self.pipelines[cam] = {"pipeline": None, "state": False, "formats": []}

			try:
				output = subprocess.check_output(
					["v4l2-ctl", "-d", dev, "--list-formats-ext"],
					stderr=subprocess.DEVNULL,
					text=True
				)
			except subprocess.CalledProcessError:
				print(f"[!] failed to read formats from {dev}")
				continue

			fmt = None
			all_formats = []
			for line in output.splitlines():
				line = line.strip()
				if line.startswith("[") and "]" in line:
					parts = line.split("'")
					fmt = parts[1] if len(parts) > 1 else None
				elif fmt and "Size:" in line:
					toks = line.replace("Size: Discrete", "").strip().split("x")
					if len(toks) == 2:
						w, h = map(int, toks)
						fps = 30
						key = (w, h, fps)
						all_formats.append((fmt, w, h, fps	))
			print(f"video{cam} formats: {all_formats}")
			self.pipelines[cam]["formats"] = all_formats
	def get_videoFormatList_legacy(self):
		"""
		產生舊版 videoFormatList 結構：
		{
			formatIndex: [ [videoNo, fmt], ...]
		}
		"""
		legacyList = {}
		for cam, info in self.pipelines.items():
			for fmt, w, h, fps in info["formats"]:
				if fmt == "H264":
					continue
				# 找到對應 formatIndex
				formatIndex = None
				for idx, res in self.getFormatInfoByIndexMap().items():
					if (w, h, fps) == res:
						formatIndex = idx
						break
				if formatIndex is None:
					continue
				if formatIndex not in legacyList:
					legacyList[formatIndex] = []
				# 若已存在同相機且 MJPEG 優先
				add = True
				for i, (vno, enc) in enumerate(legacyList[formatIndex]):
					if vno == cam:
						# MJPEG 替換 YUYV
						if fmt == "MJPEG" and enc == "YUYV":
							legacyList[formatIndex][i][1] = "MJPEG"
						add = False
						break
						
				if add:
					legacyList[formatIndex].append([cam, fmt])
		return legacyList
	def getYUYVFrameRate(self, cam, width=None, height=None):
		"""
		從系統抓出 cam 支援的 YUYV 格式最高 fps。
		如果 width/height 提供，就限定解析度。
		"""
		import re, subprocess
		dev = f"/dev/video{cam}"
		try:
			output = subprocess.check_output(
				["v4l2-ctl", "-d", dev, "--list-formats-ext"],
				stderr=subprocess.DEVNULL,
				text=True
			)
		except subprocess.CalledProcessError:
			return ""

		fmt = None
		match_res = False
		fps_list = []

		for line in output.splitlines():
			line = line.strip()

			# --- 找格式 ---
			if line.startswith("[") and "]" in line:
				# e.g. [0]: 'YUYV' (YUYV 4:2:2)
				parts = line.split("'")
				fmt = parts[1] if len(parts) > 1 else None
				continue

			# --- 找尺寸 ---
			if fmt == "YUYV" and line.startswith("Size: Discrete"):
				# e.g. Size: Discrete 1280x720
				toks = line.replace("Size: Discrete", "").strip().split("x")
				if len(toks) == 2:
					w, h = map(int, toks)
					if (width is None or w == width) and (height is None or h == height):
						match_res = True
					else:
						match_res = False
				continue

			# --- 找 fps ---
			if match_res and line.startswith("Interval: Discrete"):
				# e.g. Interval: Discrete 0.033s (30.000 fps)
				m = re.search(r"\(([\d\.]+)\s*fps\)", line)
				if m:
					fps_list.append(float(m.group(1)))

		if fps_list:
			return int(max(fps_list))  # 回傳最高 fps
		return ""

	def _on_message(self, bus, message, cam_name):
		t = message.type
		if t == Gst.MessageType.ERROR:
			err, debug = message.parse_error()
			print(f"攝影機 {cam_name} 發生錯誤: {err.message}")
			
			# 重要：當電壓不穩影像斷掉時，這裡會被觸發
			self._stop_pipeline(cam_name)  # 先停止並釋放資源
			self._handle_reconnect(cam_name) 
			
		elif t == Gst.MessageType.EOS:
			print(f"攝影機 {cam_name} 串流結束")
			self._stop_pipeline(cam_name)

		return True # 保持監聽
	def _handle_reconnect(self, cam):
		"""處理斷線重連邏輯"""
		print(f"嘗試重新啟動 {cam}...")
		# 延遲 3 秒後重試，避免在硬體未就緒時狂刷
		GLib.timeout_add(3000, self._start_pipeline, cam)
	
	# ---------------- Pipeline 管理 ---------------- #
	def _create_pipeline(self, cam, gstring, port, encoder):
		"""建立新的 pipeline，若已存在則先釋放"""
		if cam in self.pipelines:
			self._stop_pipeline(cam)

		pipeline = Gst.parse_launch(gstring)
		self.pipelines[cam]['pipeline'] = pipeline
		self.pipelines[cam]['state'] = False
		self.pipelines[cam]['port'] = port
		self.pipelines[cam]['encoder'] = encoder
		
		return pipeline

	def _start_pipeline(self, cam):
		"""啟動 pipeline"""
		if cam not in self.pipelines:
			return False
		pipeline = self.pipelines[cam]["pipeline"]
		bus = pipeline.get_bus()
		bus.add_signal_watch()
		loop = GLib.MainLoop()
		bus.connect("message", self._on_message, cam)
		pipeline.set_state(Gst.State.PLAYING)
		ret = pipeline.set_state(Gst.State.PLAYING)
		if ret == Gst.StateChangeReturn.FAILURE:
			print(f"無法啟動 {cam}")
			return False
			
		self.pipelines[cam]["state"] = True
		return True

	def _stop_pipeline(self, cam):
		"""停止並釋放 pipeline，但保留 camera 格式資訊"""
		if cam not in self.pipelines or not self.pipelines[cam].get("pipeline"):
			print(f"Camera {cam} 已經是停止狀態或不存在")
			return

		pipeline = self.pipelines[cam].get("pipeline")

		bus = pipeline.get_bus()
		bus.remove_signal_watch()
		pipeline.set_state(Gst.State.NULL)
		pipeline.get_state(Gst.CLOCK_TIME_NONE)  # 等待切換完成
		
		res, state, pending = pipeline.get_state(1 * Gst.SECOND) 
		if res == Gst.StateChangeReturn.FAILURE:
			print(f"警告: {cam} 無法完全切換至 NULL 狀態")

		self.pipelines[cam]["pipeline"] = None  # 清掉 pipeline，但保留格式
		self.pipelines[cam]["state"] = False

		# 釋放 portOccupied
		ports_to_remove = [p for p, c in self.portOccupied.items() if c == cam]
		for p in ports_to_remove:
			self.portOccupied.pop(p, None)

		print(f"_stop_pipeline: video{cam} stopped and cleaned")



	# ---------------- 播放控制 ---------------- #
	def play(self, cam, width, height, fps, encoder, IP, port, YOLO_detection_enabled):
		"""
		新版 play()，完全用 self.pipelines 管理 pipeline
		"""
		if cam not in self.pipelines:
			print(f"video{cam} not found")
			return

		# 找對應格式
		fmtFound = None
		for fmt, w, h, f in self.pipelines[cam]["formats"]:
			if w == width and h == height and f == fps:
				fmtFound = fmt
				break
		if fmtFound is None:
			print(f"video{cam} has no format {width}x{height}@{fps}")
			return

		
		# 生成 GStreamer 指令
		gstring = VideoFormat.getFormatCMD(getOS(), cam, fmtFound, width, height, fps, encoder, IP, port)
		print(f"[Pipeline] cam={cam}, port={port}, encoder={encoder}")
		self.node.get_logger().info(f"Pipeline string: {gstring}")

		# 釋放被占用的 port
		if port in self.portOccupied:
			videoToStop = self.portOccupied[port]
			self._stop_pipeline(videoToStop)
			print(f"  -quit occupied: video{videoToStop}")

		# 一般播放
		pipeline = self._create_pipeline(cam, gstring, port, encoder)
		self._start_pipeline(cam)
		self.portOccupied[port] = cam


	def stop(self, cam):
		# 停止 pipeline
		self._stop_pipeline(cam)
		print(f"stop pipeline on cam:{cam}")

	



	def handleMsg(self, data, addr):
		operation = int(data[0])
		self.node.get_logger().info(f"handleMsg: operation={operation}, from {addr}, length={len(data)}")
		if operation == 0 and len(data) >= 9:
			self.handleSetFormat(data, addr)
		elif operation == 1:
			self.handleSetFormat(data, addr)
		elif operation == 2:
			self.handleQuit(data, addr)
		elif operation == 3:
			self.handleSetSeagrassCameraFormat(data, addr)
		elif operation == 4:
			self.handleDetect(data, addr)
		elif operation == 5:
			self.handleSeagrass(data, addr)
		else:
			logging.warning(f"Unknown operation: {operation}")
	def handleSetSeagrassCameraFormat(self, data, addr):
		if len(data) < 8:
			self.node.get_logger().warning("handleSetSeagrassCameraFormat: insufficient data length")
			return
		videoNo = int(data[1])
		formatIndex = int(data[2])
		encoder = 'h264' if int(data[3]) == 0 else 'mjpeg'
		port = int.from_bytes(data[4:8], 'little')

		fmtMap = self.getFormatInfoByIndexMap()
		if formatIndex not in fmtMap:
			self.node.get_logger().warning(f"handleSetSeagrassCameraFormat: invalid formatIndex {formatIndex}")
			return
		width, height, fps = fmtMap[formatIndex]
		if videoNo not in self.pipelines:
			self.node.get_logger().warning(f"handleSetSeagrassCameraFormat: video{videoNo} not found")
			return
		cam_formats = self.pipelines[videoNo]["formats"]
		fmtFound = next((f for f, w2, h2, f2 in cam_formats if w2 == width and h2 == height and f2 == fps), None)
		if not fmtFound:
			self.node.get_logger().warning(f"handleSetSeagrassCameraFormat: video{videoNo} does not support {width}x{height}@{fps}")
			return
		self.setSeagrassCamera(videoNo, fmtFound, width, height, fps, encoder, addr, port)
	def setSeagrassCamera(self, cam, format, width, height, framerate, encoder, IP, port):
		self.node.get_logger().info(f"set seagrass camera: {cam} {format} {width} {height} {framerate} {encoder} {IP} {port}")
		MJPGfps = self.getMJPGFrameRate(cam, width, height)
		if MJPGfps != "":
			self.node.get_logger().info(f"MJPG fps for video{cam}: {MJPGfps}")
			self.seagrass_cam_format = [cam, "MJPG", width, height, MJPGfps, IP, port]
			self.node.get_logger().info(f"start ai on cam:{cam}")
			self.seagrass_cam = cam
			#self._toolBox.seagrassDetect.setFormat(self.seagrass_cam_format)
		else:
			self.node.get_logger().warning(f"video{cam} had no MJPG format")
	def getMJPGFrameRate(self, cam, width=None, height=None):
		"""
		從系統抓出 cam 支援的 MJPG 格式最高 fps。
		如果 width/height 提供，就限定解析度。
		"""
		
		dev = f"/dev/video{cam}"
		try:
			output = subprocess.check_output(
				["v4l2-ctl", "-d", dev, "--list-formats-ext"],
				stderr=subprocess.DEVNULL,
				text=True
			)
		except subprocess.CalledProcessError:
			return ""

		fmt = None
		match_res = False
		fps_list = []

		for line in output.splitlines():
			line = line.strip()

			# --- 找格式 ---
			if line.startswith("[") and "]" in line:
				# e.g. [0]: 'YUYV' (YUYV 4:2:2)
				parts = line.split("'")
				fmt = parts[1] if len(parts) > 1 else None
				continue

			# --- 找尺寸 ---
			if fmt == "MJPG" and line.startswith("Size: Discrete"):
				# e.g. Size: Discrete 1280x720
				toks = line.replace("Size: Discrete", "").strip().split("x")
				if len(toks) == 2:
					w, h = map(int, toks)
					if (width is None or w == width) and (height is None or h == height):
						match_res = True
					else:
						match_res = False
				continue

			# --- 找 fps ---
			if match_res and line.startswith("Interval: Discrete"):
				# e.g. Interval: Discrete 0.033s (30.000 fps)
				m = re.search(r"\(([\d\.]+)\s*fps\)", line)
				if m:
					fps_list.append(float(m.group(1)))

		if fps_list:
			return int(max(fps_list))  # 回傳最高 fps
		return ""

	def handleQuit(self, data, addr):
		videoNo = int(data[1])
		self.stop(videoNo)
		self.node.get_logger().info(f"handleQuit: Stopped video {videoNo}")

	def handleSetFormat(self, data, addr):
		"""
		兼容舊版 formatIndex，呼叫新版 play()
		"""
		videoNo = int(data[1])
		formatIndex = int(data[2])
		encoder = 'h264' if int(data[3]) == 0 else 'mjpeg'
		port = int.from_bytes(data[4:8], 'little')
		ai_enabled = int(data[8])
		ip = addr

		# 1. 取得舊版解析度
		formatInfo = self.getFormatInfoByIndex(formatIndex)
		if not formatInfo:
			self.node.get_logger().warning(f"Invalid formatIndex {formatIndex}")
			return
		width, height, fps = formatInfo

		# 2. 檢查相機是否存在，並確認有該解析度
		if videoNo not in self.pipelines:
			self.node.get_logger().warning(f"video{videoNo} not found")
			return

		cam_formats = self.pipelines[videoNo]["formats"]
		fmtFound = None
		for fmt, w, h, f in cam_formats:
			if w == width and h == height and f == fps:
				fmtFound = fmt
				break
		if not fmtFound:
			self.node.get_logger().warning(f"video{videoNo} does not support {width}x{height}@{fps}")
			return

		# 3. 呼叫新版 play()
		self.play(videoNo, width, height, fps, encoder, ip, port, ai_enabled)
		self.node.get_logger().info(f"handleSetFormat: video{videoNo} {fmtFound} {width}x{height}@{fps} encoder={encoder}, ai={ai_enabled}")

