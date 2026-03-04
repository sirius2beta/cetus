import rclpy
from rclpy.node import Node
from more_interfaces.msg import MavlinkValues, MarinelinkPacket
from .DataLogger import DataLogger
from .config import Config
class LogManager(Node):
    def __init__(self):
        super().__init__('log_manager')
        
        # 修正 1: Topic 名稱必須跟 C++ 端完全對齊
        self.subscriber_ = self.create_subscription(
            MavlinkValues, 
            '/sensor/mavlink_values', 
            self.mavlinkValues_callback, 
            10
        )
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        #self.get_logger().info('LogManager has started and is listening to /sensor/mavlink_values')
        
        self.config = Config()
        self.sensor_group_list = self.config.sensor_group_list
        self.data_logger = DataLogger()

    def mavlinkValues_callback(self, msg):
        # 這裡會每秒印出一次你在 C++ 定時器發出的資料
        #self.get_logger().info(f'Received MavlinkValues - Yaw: {msg.yaw}, Pitch: {msg.pitch}, Roll: {msg.roll}')
        # 將資料傳遞給 DataLogger
        self.data_logger.log_data.fix_type = msg.fix_type
        self.data_logger.log_data.yaw = msg.yaw
        self.data_logger.log_data.pitch = msg.pitch
        self.data_logger.log_data.roll = msg.roll
        self.data_logger.log_data.speed = msg.groundspeed
        self.data_logger.log_data.depth = msg.depth

        self.sensor_group_list[4].get_sensor(0).data = msg.fix_type
        self.sensor_group_list[4].get_sensor(1).data = msg.lon
        self.sensor_group_list[4].get_sensor(2).data = msg.lat
        self.sensor_group_list[4].get_sensor(3).data = msg.alt
        self.sensor_group_list[4].get_sensor(4).data = msg.yaw
        self.sensor_group_list[4].get_sensor(5).data = msg.pitch
        self.sensor_group_list[4].get_sensor(6).data = msg.roll
        self.sensor_group_list[4].get_sensor(7).data = msg.groundspeed
        
        self.sensor_group_list[3].get_sensor(0).data = msg.depth
        self.sensor_group_list[3].get_sensor(1).data = msg.voltage_battery
        self.sensor_group_list[3].get_sensor(2).data = msg.current_battery
        self.sensor_group_list[3].get_sensor(3).data = msg.battery_remaining    
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[4].pack()))
        self.publisher_.publish(MarinelinkPacket(topic=4, payload=self.sensor_group_list[3].pack()))
def main(args=None):
    # 修正 2: 加入必要的 ROS 2 啟動流程
    rclpy.init(args=args)
    log_manager = LogManager()
    
    try:
        # 保持節點運行，直到被手動停止 (Ctrl+C)
        rclpy.spin(log_manager)
    except KeyboardInterrupt:
        pass
    finally:
        log_manager.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()