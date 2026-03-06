from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='link_manager',
            namespace='link_manager',
            executable='link_manager',
            name='link_manager',
            output='both',
            respawn=True,
            respawn_delay=2.0,
            #prefix=['gdb -ex run --args']
        ),
        Node(
            package='video_manager',
            namespace='video_manager',
            executable='video_manager',
            name='video_manager'
        ),
        Node(
            package='log_manager',
            namespace='log_manager',
            executable='log_manager',
            name='log_manager'
        ),
        Node(
            package='rs485_manager',
            namespace='rs485_manager',
            executable='rs485_manager',
            name='rs485_manager',\
            respawn=True,
            respawn_delay=3.0,
        ),
        Node(
            package='septentrio_gnss_driver',
            executable='septentrio_gnss_driver_node',
            name='septentrio_gnss',
            output='screen',
            # 這裡開啟熱插拔自動重連
            respawn=True,
            respawn_delay=2.0,
            parameters=[{
                'device': 'serial:/dev/sensors/gps_data',
                'baudrate': 115200,
                'frame_id': 'gps_link',
            }]
        )
    ])