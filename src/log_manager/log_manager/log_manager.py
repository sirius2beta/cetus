import rclpy
from rclpy.node import Node

class LogManager(Node):
    def __init__(self):
        super().__init__('log_manager')
        
        self.subscriber_ = self.create_subscription(MarinelinkPacket, '/sensor', self.marinelink_callback, 10)
        self.publisher_ = self.create_publisher(MarinelinkPacket, '/marinelink_tosend', 10)
        self.videoManager = VideoManager(self)
        

def main():
    print('Hi from log_manager.')


if __name__ == '__main__':
    main()
