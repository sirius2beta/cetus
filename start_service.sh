#!/bin/bash
# 1. 這裡還是手動載入一次最保險，因為 Service 不會讀取 .bashrc 的環境
source /opt/ros/humble/setup.bash
source /home/sirius2beta/cetus/install/setup.bash

# 2. 設定環境變數
export PYTHONUNBUFFERED=1
export ROS_LOG_DIR=/home/sirius2beta/GPlayerLogNew/debug

# 3. 執行 Launch 並導出 Log
# 使用 -a 代表 append (追加)，避免重啟後舊 Log 被洗掉
ros2 launch /home/sirius2beta/cetus/launch/cetus_launch.py 2>&1 | tee -a /home/sirius2beta/GPlayerLogNew/debug/all_nodes.log
