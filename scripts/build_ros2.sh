#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
workspace="$(cd "${script_dir}/../ros2_ws" && pwd)"
ros_distro="${ROS_DISTRO:-humble}"

# shellcheck disable=SC1090
source "/opt/ros/${ros_distro}/setup.bash"
cd "${workspace}"
rosdep install --from-paths src --ignore-src -r -y --rosdistro "${ros_distro}"
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release
echo "ROS 2 adapter built. Run: source ${workspace}/install/setup.bash"
