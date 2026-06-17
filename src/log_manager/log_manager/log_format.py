import csv
from io import StringIO

class LogFormat:
    def __init__(self):
        self.time_usec = None             # 時間戳記 (微秒)
        self.gps_date = None
        self.gps_timestamp = None         # 時間戳記 (YYYYMMDD_HHMM)
        # Pixhawk 資料
        self.fix_type = None              # 定位類型
        self.depth = None                 # 深度
        self.speed = None                 # 速度
        self.roll = None                  # 姿態-翻滚
        self.pitch = None                 # 姿態-俯仰
        self.yaw = None                   # 姿態-航偏

        # ArduSimple 資料
        self.lat = None                   # 緯度
        self.lon = None                   # 經度
        self.alt = None                   # 海拔高度
        self.HDOP = None                  # 水平精度因子 (HDOP)
        self.VDOP = None                  # 垂直精度因子 (VDOP)
        self.lat_acc = None               # 緯度精度
        self.lon_acc = None               # 經度精度
        self.alt_acc = None               # 高度精度
        self.gps_speed = None             # GPS-速度
        self.gps_tilt = None              # GPS-俯仰
        self.gps_yaw = None               # GPS-航偏
        self.gps_orthometric_height = None  # GPS-正高
        self.geoid_separation = None        # GPS-大地起伏

        # Aqua 資料
        self.temperature = None                     # 1. 水溫
        self.pressure = None                        # 2. 壓力
        self.aqua_depth = None                      # 3. 深度
        self.level_depth_to_water = None            # 4. 水位深度
        self.level_surface_elevation = None         # 5. 表面高程
        self.actual_conductivity = None             # 6. 實際導電率
        self.specific_conductivity = None           # 7. 特定導電率
        self.resistivity = None                     # 8. 電阻率
        self.salinity = None                        # 9. 鹽度
        self.total_dissolved_solids = None          # 10. 總溶解固體
        self.density_of_water = None                # 11. 水密度
        self.barometric_pressure = None             # 12. 大氣壓力
        self.ph = None                              # 13. pH 值
        self.ph_mv = None                           # 14. pH 毫伏
        self.orp = None                             # 15. 氧化還原電位 (ORP)
        self.dissolved_oxygen_concentration = None  # 16. 溶解氧濃度
        self.dissolved_oxygen_saturation = None     # 17. 溶解氧飽和度百分比
        self.turbidity = None                       # 18. 濁度
        self.oxygen_partial_pressure = None         # 19. 氧分壓
        self.external_voltage = None                # 20. 外部電壓
        self.battery_capacity_remaining = None      # 21. 電池剩餘容量

        # Kbest
        self.kbest_boat_rssi = None            # Kbest-船載接收訊號強度指標
        self.kbest_ground_rssi = None          # Kbest-基站接收訊
        self.super_taira_strength = None       # SuperTaiRa-訊號強度指標
        self.super_taira_error_byte = None     # SuperTaiRa-錯誤碼

        # seagrass image name
        self.seagrass_coverage_ratio = None
        self.seagrass_image_name = None

    def get_fields(self):
        """傳回固定的欄位名稱清單，供資料庫建表使用"""
        return list(self.__dict__.keys())

    def get_values_tuple(self):
        """傳回當前所有屬性值組成的元組 (Tuple)，並在傳回後重置圖片名稱"""
        values = tuple(self.__dict__.values())
        self.seagrass_image_name = None  # 保持你原本寫入後重置圖片的邏輯
        return values

    def get_all(self):
        """(保留備用) 取得所有屬性並返回 CSV 格式"""
        data = self.__dict__
        values = list(data.values())
        output = StringIO()
        writer = csv.writer(output) 
        writer.writerow(values)
        self.seagrass_image_name = None 
        return output.getvalue()