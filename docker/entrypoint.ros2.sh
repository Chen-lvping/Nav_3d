#!/usr/bin/env bash
set -e
source "/opt/ros/${ROS_DISTRO}/setup.bash"
source "/opt/nav3d_ws/install/setup.bash"
export LD_LIBRARY_PATH="/opt/casadi/lib:/usr/local/lib:${LD_LIBRARY_PATH:-}"
exec "$@"
