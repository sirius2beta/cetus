from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import SetEnvironmentVariable, DeclareLaunchArgument
import os
def generate_launch_description():
    custom_log_dir = '/home/sirius2beta/GPlayerLogNew/debug'

    # 如果目錄不存在，可以考慮在啟動前手動建立，或者讓系統自動生成
    os.makedirs(custom_log_dir, exist_ok=True)
    return LaunchDescription([
        SetEnvironmentVariable('ROS_LOG_DIR', custom_log_dir),
        Node(
            package='link_manager',
            namespace='link_manager',
            executable='link_manager',
            name='link_manager',
            output='both',
            respawn=True,
            respawn_delay=5.0,
            #prefix=['gdb -ex run --args']
        ),
        Node(
            package='video_manager',
            namespace='video_manager',
            executable='video_manager',
            output='both',
            respawn=True,
            respawn_delay=2.0,
            name='video_manager'
        ),
        Node(
            package='log_manager',
            namespace='log_manager',
            executable='log_manager',
            respawn=True,
            respawn_delay=2.0,
            name='log_manager'
        ),
        Node(
            package='rs485_manager',
            namespace='rs485_manager',
            executable='rs485_manager',
            name='rs485_manager',
            respawn=True,
            respawn_delay=3.0,
        ),
        Node(
            package='gps_manager',
            namespace='gps_manager',
            executable='gps_manager',
            name='gps_manager',
            respawn=True,
            respawn_delay=3.0,
            output='both',
        ),
        Node(
            package='kbest_manager',
            namespace='kbest_manager',
            executable='kbest_manager',
            name='kbest_manager',
            respawn=True,
            respawn_delay=3.0,
            output='both',
        ),
        Node(
            package='seagrassdetect',
            namespace='seagrassdetect',
            executable='seagrassdetect',
            name='seagrassdetect',
            respawn=True,
            respawn_delay=3.0,
            output='screen',
            emulate_tty=True,
        ),
        
    ])