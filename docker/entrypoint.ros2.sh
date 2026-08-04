#!/usr/bin/env bash
set -e

# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO}/setup.bash"
# shellcheck disable=SC1091
source "/opt/nav3d_ros2_ws/install/setup.bash"
exec "$@"
