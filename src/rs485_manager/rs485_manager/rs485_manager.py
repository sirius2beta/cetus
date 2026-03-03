import serial.tools.list_ports
import rclpy
from rclpy.node import Node
from .config import Config
from .RS485Device import RS485Device

def find_device_by_ids(vid, pid):
    # 取得所有序列埠清單
    ports = serial.tools.list_ports.comports()
    
    for port in ports:
        # 注意：vid 和 pid 在 port 物件中通常是整數
        if port.vid == vid and port.pid == pid:
            return port.device  # 這會回傳像 '/dev/ttyUSB0' 的字串
            
    return None

# ID of RS485 adapter
target_vid = 0x067b
target_pid = 0x2303




class RS485ManagerNode(Node):
    def __init__(self):
        super().__init__('rs485_manager')
        device_path = find_device_by_ids(target_vid, target_pid)

        if device_path:
            print(f"找到裝置！路徑為: {device_path}")
        else:
            print("找不到匹配的裝置。")

        RS485_device = RS485Device(4, device_path)
        RS485_device.start_loop()

def main(args=None):
    rclpy.init(args=args)
    node = RS485ManagerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()