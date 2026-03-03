# cetus
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run link_manager link_manager

colcon build --packages-select link_manager

ros2 launch launch/cetus_launch.py

ros2 topic list
ros2 node list


from ament_index_python.packages import get_package_share_directory
import os

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