from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='link_manager',
            namespace='link_manager',
            executable='link_manager',
            name='link_manager'
        )
    ])