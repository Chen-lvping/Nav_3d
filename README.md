# Nav_3d — 原生 ROS 2 全栈 3D 导航

`ros2-native` 分支把建图、定位、全局规划、轨迹平滑、动态障碍处理、NMPC 控制和机器人执行统一迁移到 ROS 2。运行时只使用 DDS，不需要 ROS 1、`roscore`、`rosbridge` 或消息桥。

## 能力链路

| 能力 | 原生 ROS 2 实现 |
|---|---|
| LiDAR 驱动 | Livox ROS Driver 2、RoboSense rslidar SDK |
| 建图/里程计 | FAST-LIO (`rclcpp`) |
| 已知地图定位 | Open3D ICP + `tf2_ros` |
| 地图处理/发布 | `map_process`、`map_publisher` |
| 全局规划 | 3D PRM |
| 路径平滑 | Bezier optimizer |
| 动态避障 | PointCloud2 obstacle processor |
| 局部控制 | CasADi NMPC |
| 底盘执行 | Unitree Go2 ROS 2 controller |

## Docker 快速启动

支持 ROS 2 Humble（Ubuntu 22.04）和 Jazzy（Ubuntu 24.04），镜像可在 `linux/amd64` 与 `linux/arm64` 上源码构建：

```bash
cp .env.example .env
docker compose build
docker compose run --rm nav3d ros2 launch nav_bringup mapping.launch.py
docker compose run --rm nav3d ros2 launch nav_bringup localization.launch.py
docker compose run --rm nav3d ros2 launch nav_bringup navigation.launch.py
```

完整链路：

```bash
docker compose up --build
```

地图与 bag 数据默认挂载到 `./data`，可通过 `NAV3D_DATA_ROOT` 修改。

## 本机构建

```bash
source /opt/ros/humble/setup.bash   # Jazzy 改为 /opt/ros/jazzy
export CASADI_ROOT=/opt/casadi
export CASADI_LIB_PATH=/opt/casadi/lib
rosdep install --from-paths src/3D_NAV --ignore-src -r -y
colcon build --base-paths src/3D_NAV --merge-install --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

详细参数、话题契约和部署说明见 [docs/ROS2.md](docs/ROS2.md) 与 [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md)。
