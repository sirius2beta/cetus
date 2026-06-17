import os
import time
import threading
import sqlite3
import re
from datetime import datetime
from .log_format import LogFormat

class DataLogger():
    def __init__(self):
        self.logging_interval = 0.25 
        self.log_data = LogFormat() # 存放資料的地方
        
        # 緩衝機制：先存快取，減少硬碟 I/O 次數
        self.buffer = []
        self.buffer_size = 4  # 每 4 筆資料(約1秒)集體寫入硬碟一次
        self.lock = threading.Lock() # 確保執行緒安全
        
        self.log_folder_path = "/home/sirius2beta/GPlayerLogNew"

        # 設定 log 存放路徑
        self.log_directory = os.path.expanduser(self.log_folder_path)
        if not os.path.exists(self.log_directory):
            os.makedirs(self.log_directory)

        # 找出所有 log_xxxxxxxx.db 檔案 (改為 .db 擴充檔名)
        existing_files = [f for f in os.listdir(self.log_directory) if f.startswith("log_") and f.endswith(".db")]

        # 從檔名抓出數字部分
        indices = []
        for f in existing_files:
            match = re.search(r"log_(\d+)\.db", f)
            if match:
                indices.append(int(match.group(1)))

        # 取最大值 + 1，如果沒有檔案就從 1 開始
        file_index = max(indices) + 1 if indices else 1

        # 檔名格式：log_00000001.db
        file_name = f"log_{file_index:08d}.db"
        self.log_file = os.path.join(self.log_directory, file_name)

        # 初始化 SQLite 資料庫與資料表
        self._init_db()
        
        # 開始背景 log 執行緒
        threading.Thread(target=self.looper, daemon=True).start()

    def _init_db(self):
        """建立資料庫、資料表，並設定效能優化參數"""
        # 取得所有欄位名稱
        self.fields = list(self.log_data.__dict__.keys())
        
        # 動態生成 SQL 建立資料表語法 (假設大部分為 TEXT 或 REAL，SQLite 具體會動態分類，這裡預設用通用宣告)
        fields_sql = ", ".join([f"[{field}]" for field in self.fields])
        
        conn = sqlite3.connect(self.log_file)
        cursor = conn.cursor()
        
        # 1. 啟用 WAL 模式 (重要：大大提升高頻讀寫效能)
        cursor.execute("PRAGMA journal_mode=WAL;")
        # 2. 建立資料表
        cursor.execute(f"CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, {fields_sql})")
        
        conn.commit()
        conn.close()

    def save_data(self):   
        try:
            self.log_data.time_usec = time.monotonic()
            
            # 直接呼叫新方法，取得當前所有資料的 Tuple，同時它會自動幫你重置 seagrass_image_name
            row_values = self.log_data.get_values_tuple()
            
            # 將資料放入緩衝區
            with self.lock:
                self.buffer.append(row_values)
                
                # 當緩衝區滿了（例如4筆），執行批次寫入資料庫
                if len(self.buffer) >= self.buffer_size:
                    self._flush_buffer()

        except Exception as e:
            print(f'DataLogger exception: log_entry: msg:{e}')
            
    def _flush_buffer(self):
        """將緩衝區的資料一次性寫入 SQLite 資料庫 (使用 Transaction)"""
        if not self.buffer:
            return
            
        try:
            conn = sqlite3.connect(self.log_file)
            cursor = conn.cursor()
            
            # 高速寫入指令
            placeholders = ", ".join(["?"] * len(self.fields))
            fields_sql = ", ".join([f"[{field}]" for field in self.fields])
            sql = f"INSERT INTO logs ({fields_sql}) VALUES ({placeholders})"
            
            # 使用 executemany 會自動開啟單一 Transaction，速度極快
            cursor.executemany(sql, self.buffer)
            conn.commit()
            self.buffer.clear() # 清空緩衝區
        except Exception as e:
            print(f'Database flush exception: {e}')
        finally:
            conn.close()

    def looper(self):
        while True:
            self.save_data()
            time.sleep(self.logging_interval)