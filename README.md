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

sudo nano /etc/udev/rules.d/99-robot.rules

# Septentrio GNSS - 數據主通道 (Interface 02 為 CDC-ACM)
SUBSYSTEM=="tty", ATTRS{idVendor}=="152a", ATTRS{idProduct}=="85c0", ENV{ID_USB_INTERFACE_NUM}=="02", SYMLINK+="sensors/gps_data", MODE="0666"

# Septentrio GNSS - 配置通道 (Interface 04 為 CDC-ACM)
SUBSYSTEM=="tty", ATTRS{idVendor}=="152a", ATTRS{idProduct}=="85c0", ENV{ID_USB_INTERFACE_NUM}=="04", SYMLINK+="sensors/gps_config", MODE="0666"

sudo udevadm control --reload-rules && sudo udevadm trigger

sudo apt install ros-humble-septentrio-gnss-driver

ros2 run septentrio_gnss_driver septentrio_gnss_driver_node --ros-args -p device:=serial:/dev/sensors/gps_data -p baudrate:=115200

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
                'activate_configuration': False,
            }]
        )