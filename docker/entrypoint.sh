#!/usr/bin/env bash
set -e

# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO:-noetic}/setup.bash"
# shellcheck disable=SC1091
source "${NAV3D_WS:-/opt/nav3d_ws}/devel/setup.bash"
exec "$@"
