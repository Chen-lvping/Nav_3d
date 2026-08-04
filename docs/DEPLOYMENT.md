# ROS 2 原生部署

## 配置

```bash
cp .env.example .env
```

常用环境变量：

```dotenv
ROS_DISTRO=humble
ROS_DOMAIN_ID=0
RMW_IMPLEMENTATION=rmw_fastrtps_cpp
NAV3D_DATA_ROOT=./data
BUILD_JOBS=4
```

Jazzy 部署只需把 `ROS_DISTRO` 改为 `jazzy` 后重建。

## 构建与检查

```bash
docker compose build nav3d
docker compose run --rm nav3d ros2 pkg list | grep -E 'fast_lio|open3d_loc|planning_3d|nmpc_planner'
docker compose run --rm nav3d ros2 launch nav_bringup mapping.launch.py --show-args
docker compose run --rm nav3d ros2 launch nav_bringup navigation.launch.py --show-args
```

## 运行方式

```bash
# 默认完整链路
docker compose up

# 单独建图
docker compose run --rm nav3d ros2 launch nav_bringup mapping.launch.py sensor:=livox

# 单独定位
docker compose run --rm nav3d ros2 launch nav_bringup localization.launch.py map_path:=/data/point_cloud/scans.pcd

# 单独导航
docker compose run --rm nav3d ros2 launch nav_bringup navigation.launch.py
```

## 跨平台边界

算法和 ROS 2 节点通过容器在 x86_64/ARM64 Linux 上保持一致。Windows/macOS 可用于仿真、bag 回放与开发，但真实 LiDAR UDP、Go2 DDS 和低延迟控制建议部署到 Linux 主机，因为 Docker Desktop 的虚拟网络无法等价替代 Linux host network。

## 数据目录

```text
data/
├── bags/
├── point_cloud/scans.pcd
└── traversable/traversable_areas.pcd
```

生产环境应把 `NAV3D_DATA_ROOT` 指向持久化磁盘，并确保点云地图路径与 launch 参数一致。
