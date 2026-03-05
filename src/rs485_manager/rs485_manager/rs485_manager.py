import serial.tools.list_ports
import rclpy
import sys

from rclpy.node import Node
from .config import Config
from .RS485Device import RS485Device
from more_interfaces.msg import AquaValue, WinchStatus

def find_device_by_ids(vid, pid):
    # 取得所有序列埠清單
    ports = serial.tools.list_ports.comports()
    
    for port in ports:
        # 注意：vid 和 pid 在 port 物件中通常是整數
        if port.vid == vid and port.pid == pid:
            return port.device  # 這會回傳像 '/dev/ttyUSB0' 的字串
            
    return None

# ID of RS485 adapter
target_vid = 0x1a86
target_pid = 0x7523

class RS485ManagerNode(Node):
    def __init__(self):
        super().__init__('rs485_manager')
        device_path = find_device_by_ids(target_vid, target_pid)
        self.aqua_value_publisher_ = self.create_publisher(AquaValue, '/sensor/aqua_value', 10)
        self.winch_status_publisher_ = self.create_publisher(WinchStatus, '/sensor/winch_status', 10)
        if device_path:
            self.get_logger().info(f"找到裝置！路徑為: {device_path}")
            RS485_device = RS485Device(4, device_path, self)
            RS485_device.start_loop()
        else:
            self.get_logger().error("找不到匹配的裝置。")
            sys.exit(1) # 退出碼非 0 會讓 Launch 知道這是不正常退出

def main(args=None):
    rclpy.init(args=args)
    node = RS485ManagerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()