from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='link_manager',
            namespace='link_manager',
            executable='link_manager',
            name='link_manager'
        ),
        Node(
            package='video_manager',
            namespace='video_manager',
            executable='video_manager',
            name='video_manager'
        )
    ])