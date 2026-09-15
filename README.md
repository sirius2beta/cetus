# cetus
source /opt/ros/humble/setup.bash
source install/setup.bash

# 建置某個
colcon build --packages-select link_manager


# 執行某個package
ros2 run link_manager link_manager

ros2 launch launch/cetus_launch.py

# 模擬回放模式（不建立新的任務 SQLite log，也不啟動 RS485/GPS/KBest 實體資料來源）
# 會依 logs.time_usec 的相鄰時間差，將 log 中的 sensor 資料傳送至陸地端
ros2 launch launch/cetus_launch.py simulation_mode:=true replay_log:=/absolute/path/to/log_00000001.db

# 模擬影像：資料夾中的 MP4 依檔名排序，分別提供 cetusvideo1 與 cetusvideo3，並循環播放
ros2 launch launch/cetus_launch.py \
    simulation_mode:=true \
    replay_log:=/absolute/path/to/log_00000008.db \
    simulation_video_dir:=/absolute/path/to/mp4_directory


# 查看某topic訊息
ros2 topic list
ros2 node list
ros2 topic echo /sensor/mavlink_values


from ament_index_python.packages import get_package_share_directory
import os

# create packaage with c++
ros2 pkg create --build-type ament_cmake

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
ros2 pkg create --build-type ament_python my_package

In package.xml
<exec_depend>rclpy</exec_depend>
<exec_depend>more_interfaces</exec_depend>

from more_interfaces.msg import MavlinkPacket, MarinelinkPacket
import rclpy
from rclpy.node import Node

#寫到jetson detect的時候記得要 update IMU


# 如果AI無法載入，記憶體缺
# 檢查快取
free -h 
如果 Swap 那一列是 0B，請立刻執行：

# 快速建立 4G Swap
sudo fallocate -l 4G /var/swapfile
sudo chmod 600 /var/swapfile
sudo mkswap /var/swapfile
sudo swapon /var/swapfile

# 載入 ROS 2 Humble 核心環境
source /opt/ros/humble/setup.bash

# 載入你的 Workspace 環境 (請替換成你實際的絕對路徑)
# 例如: source /home/user/ros2_ws/install/setup.bash
source /home/sirius2beta/cetus/install/setup.bash


ros2 launch launch/cetus_launch.py \
    simulation_mode:=true \
    replay_log:=/absolute/path/to/log_00000001.db
    
ros2 launch launch/cetus_launch.py \
    simulation_mode:=true \
    replay_log:=/home/sirius2beta/GPlayerLogNew/log_00000008.db \
    simulation_video_dir:=/home/sirius2beta/simulation
    /home/sirius2beta/GPlayerLogNew/log_00000001.db
    /home/sirius2beta/simulation
