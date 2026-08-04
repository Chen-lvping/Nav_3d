# Deployment

## Recommended: Docker Compose

Compose isolates ROS 1 Noetic and ROS 2 on their supported Ubuntu bases while
sharing only declared topics:

```bash
export NAV3D_DATA_ROOT=/absolute/path/to/nav3d_data
export ROS2_DISTRO=humble       # humble or jazzy
docker compose build
docker compose up
```

To start the generic navigation launch in the ROS 1 service:

```bash
export NAV3D_LAUNCH='roslaunch nav_bringup bringup_navigation.launch pcd_path:=/data/traversable/traversable_areas.pcd'
docker compose up
```

The same Compose file runs as Linux containers on Docker Engine, Docker Desktop
for Windows, and Docker Desktop for macOS. `linux/amd64` and `linux/arm64` are
supported by the generic images. Hardware drivers may require Linux-specific
device mappings and should be added in a local Compose override.

## Build individual images

```bash
docker build -f docker/Dockerfile -t nav3d:noetic .
docker build --build-arg ROS_DISTRO=humble \
  -f docker/Dockerfile.ros2 -t nav3d:humble-adapter .
```

Multi-architecture publication:

```bash
docker buildx build --platform linux/amd64,linux/arm64 \
  -f docker/Dockerfile -t your-registry/nav3d:noetic --push .
docker buildx build --platform linux/amd64,linux/arm64 \
  --build-arg ROS_DISTRO=humble -f docker/Dockerfile.ros2 \
  -t your-registry/nav3d:humble-adapter --push .
```

## Native ROS 1 Noetic

Ubuntu 20.04 is the supported native baseline:

```bash
sudo apt install python3-rosdep python3-catkin-tools build-essential cmake git
rosdep install --from-paths src --ignore-src -r -y
source scripts/nav3d_env.sh
scripts/build_workspace.sh core
```

CasADi must provide C++ headers and libraries. Use `scripts/install_casadi.sh`
or install CasADi 3.5.5 under `/opt/casadi`.

## Native ROS 2 adapter

On a supported Humble or Jazzy host:

```bash
bash scripts/build_ros2.sh
source ros2_ws/install/setup.bash
ros2 launch nav3d_ros2_adapter bridge.launch.py rosbridge_host:=127.0.0.1
```

## Runtime data

Keep data outside the image and mount it at `/data`:

```text
$NAV3D_DATA_ROOT/
  point_cloud/scans.pcd
  trace_data/mapping_trajectory.txt
  traversable/traversable_areas.pcd
  bags/
```

GUI forwarding, GPU acceleration, USB/CAN devices, real-time scheduling, and
host DDS discovery are deliberately opt-in because their configuration differs
by host OS and robot.
