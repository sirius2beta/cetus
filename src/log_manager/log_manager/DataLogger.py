import os
import time
import threading
import csv
import re

from datetime import datetime
from .log_format import LogFormat

class DataLogger():
    def __init__(self):
        self.logging_interval = 0.25 # 設定每秒記錄一次
        self.log_data = LogFormat() # 存放資料的地方
        """
        1. 在初始化 DataLogger 時，會檢查是否存在存放log的資料夾 "../GPlayerLog"，
            若不存在，則自動建立該資料夾。
        2. 使用初始化當下的時間來定義日誌文件的名稱，格式為：log_YYYYMMDD_HHMM.txt，
            其中 YYYY 是西元年，MM 是月份，DD 是日期，HHMM 是時間的時和分。
        """
        # ================================================================================
        self.log_folder_path = "/home/sirius2beta/GPlayerLogNew"
        

        # 設定 log 存放路徑
        self.log_directory = os.path.expanduser(self.log_folder_path)
        if not os.path.exists(self.log_directory):
            os.makedirs(self.log_directory)

        # 找出所有 log_xxxxxxxx.csv 檔案
        existing_files = [f for f in os.listdir(self.log_directory) if f.startswith("log_") and f.endswith(".csv")]

        # 從檔名抓出數字部分
        indices = []
        for f in existing_files:
            match = re.search(r"log_(\d+)\.csv", f)
            if match:
                indices.append(int(match.group(1)))

        # 取最大值 + 1，如果沒有檔案就從 1 開始
        file_index = max(indices) + 1 if indices else 1

        # 檔名格式：log_00000001.csv
        file_name = f"log_{file_index:08d}.csv"
        self.log_file = os.path.join(self.log_directory, file_name)

        # 建立 CSV 檔案，並寫入欄位名稱
        with open(self.log_file, 'w', newline='', encoding="utf-8") as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=self.log_data.__dict__.keys())
            writer.writeheader()
        threading.Thread(target = self.looper, daemon = True).start() # 開始log

    def save_data(self):   
        try:
            self.log_data.time_usec = time.monotonic()
            # 保存到日誌檔案
        
            with open(self.log_file, 'a', newline='', encoding="utf-8") as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=self.log_data.__dict__.keys())
                writer.writerow(self.log_data.__dict__)
            self.log_data.seagrass_image_name = None # 寫入後重置圖片名稱，避免重複寫入同一張圖片
        except Exception as e:
            print(f'DataLogger exception: log_entry: msg:{e}')

    def looper(self):
        while True:
            self.save_data()
            time.sleep(self.logging_interval)
