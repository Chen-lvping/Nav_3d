# Nav_3d

Nav_3d is a portable ROS 1 3D navigation workspace with a ROS 2 compatibility
adapter for legged and mobile robots. The proven Noetic algorithm stack remains
unchanged; standard topics are exposed to ROS 2 Humble or Jazzy through a
distribution-independent gateway.

```text
ROS 2 robot / visualization
          <-> nav3d_ros2_adapter <-> rosbridge <-> ROS 1 Nav_3d core
                                              map -> PRM/A* -> NMPC -> /cmd_vel
```

## Supported deployment matrix

| Layer | Supported baseline |
| --- | --- |
| Navigation core | ROS 1 Noetic / Ubuntu 20.04 |
| ROS 2 adapter | ROS 2 Humble / Ubuntu 22.04; Jazzy / Ubuntu 24.04 |
| Host OS | Linux, Windows 10/11 + Docker Desktop, macOS + Docker Desktop |
| CPU | `amd64` and `arm64` generic stack |
| Container runtime | Docker Engine/Desktop with Compose v2 |

Native ROS installations remain tied to the operating systems supported by
their ROS distribution. Docker is the compatibility boundary for other hosts.
Sensor drivers, robot SDKs, GPU access, real-time kernels, USB/CAN devices, and
host networking can still impose platform-specific restrictions.

## Fastest start: ROS 1 + ROS 2

```bash
git clone -b ros2-docker-portable https://github.com/Chen-lvping/Nav_3d.git
cd Nav_3d
export NAV3D_DATA_ROOT=/absolute/path/to/nav3d_data
export ROS2_DISTRO=humble                 # or jazzy
docker compose build
docker compose up
```

By default Compose starts the ROS 1 master, rosbridge, and the ROS 2 adapter.
Start the navigation launch after mounting valid map data:

```bash
NAV3D_LAUNCH='roslaunch nav_bringup bringup_navigation.launch pcd_path:=/data/traversable/traversable_areas.pcd' \
docker compose up
```

ROS 2 can then send a goal and consume the resulting paths, TF, state, and
`/cmd_vel`. Edit `ros2_ws/src/nav3d_ros2_adapter/config/bridge.yaml` to change
topic directions, QoS, or optional interfaces.

## Native builds

ROS 1 Noetic:

```bash
source scripts/nav3d_env.sh
scripts/build_workspace.sh core
```

ROS 2 adapter (Humble/Jazzy):

```bash
bash scripts/build_ros2.sh
source ros2_ws/install/setup.bash
ros2 launch nav3d_ros2_adapter bridge.launch.py rosbridge_host:=127.0.0.1
```

## Documentation

- [ROS 2 design, usage, and limitations](docs/ROS2.md)
- [Native and Docker deployment](docs/DEPLOYMENT.md)
- [Architecture and package ownership](docs/ARCHITECTURE.md)
- [Sensor and robot porting guide](docs/PORTING_GUIDE.md)
- [ROS topic, TF, and file contracts](docs/INTERFACES.md)
- [Third-party notes](docs/THIRD_PARTY.md)

Runtime bags, point clouds, generated maps, build trees, credentials, and
network-specific values are intentionally excluded from Git.
