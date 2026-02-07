import rclpy
from rclpy.node import Node
from more_interfaces.msg import MavlinkPacket, MarinelinkPacket

class VideoControl(Node):
    def __init__(self):
        super().__init__('video_control')
        self.publisher_ = self.create_publisher(MavlinkPacket, 'mavlink_packet', 10)
        timer_period = 1.0  # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)

    def timer_callback(self):
        msg = MavlinkPacket()
        msg.payload = b'abc'
        # Fill in the MavlinkPacket message fields as needed
        self.publisher_.publish(msg)
        self.get_logger().info('Publishing MavlinkPacket message')

def main(args=None):
    rclpy.init(args=args)
    video_control = VideoControl()
    rclpy.spin(video_control)
    video_control.destroy_node()
    rclpy.shutdown()