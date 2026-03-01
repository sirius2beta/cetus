# cetus
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run link_manager link_manager

colcon build --packages-select link_manager