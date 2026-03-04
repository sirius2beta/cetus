# cetus
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run link_manager link_manager

colcon build --packages-select link_manager

ros2 launch launch/cetus_launch.py

ros2 topic list
ros2 node list
## 查看某topic訊息
ros2 topic echo /sensor/mavlink_values


from ament_index_python.packages import get_package_share_directory
import os

# create packaage with c++
ros2 pkg create --build-type ament_cmake
ros2 pkg create --build-type ament_python --node-name my_node my_package

def generate_launch_description():
    # 獲取來源 Package 的 share 路徑
    shared_dir = get_package_share_directory('my_shared_configs')
    xml_file_path = os.path.join(shared_dir, 'config', 'params.xml')
    
    # 現在你可以把這個路徑餵給 Node 或是進行解析
    print(f"Loading XML from: {xml_file_path}")


#include <ament_index_cpp/get_package_share_directory.hpp>
#include <string>

std::string shared_dir = ament_index_cpp::get_package_share_directory("my_shared_configs");
std::string xml_path = shared_dir + "/config/params.xml";

# Python
In package.xml
<exec_depend>rclpy</exec_depend>
<exec_depend>more_interfaces</exec_depend>

from more_interfaces.msg import MavlinkPacket, MarinelinkPacket
import rclpy
from rclpy.node import Node

#寫到jetson detect的時候記得要 update IMU
