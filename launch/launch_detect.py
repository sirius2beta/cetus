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
            package='jetsondetect',
            namespace='jetsondetect',
            executable='jetsondetect',
            name='jetsondetect',
            output='screen',
            emulate_tty=True,
        ),
        
    ])