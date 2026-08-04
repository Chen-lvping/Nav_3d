# 原生 ROS 2 使用说明

## 结论

`ros2-native` 提供已验证的原生 ROS 2 基线；`ros2-modular-cleanup` 在此基础上拆分启动模块并清理遗留源码。建图、定位、导航和底盘控制均直接使用 ROS 2 消息、服务、参数、DDS QoS 与 TF2，不存在 ROS 1 master 或桥接进程。

旧的 `galileo_lio`、ROS 1 RViz 插件和测试包已从模块化分支删除，需要对照时可从 Git 历史恢复。

## 模块化入口

模块既可以独立启动，也可以通过 `navigation.launch.py` 或 `full_stack.launch.py` 组合：

| 启动文件 | 责任 |
|---|---|
| `sensors.launch.py` | Livox/RoboSense 传感器驱动 |
| `mapping.launch.py` | 传感器与 FAST-LIO 建图/里程计 |
| `map_processing.launch.py` | 离线提取可通行区域点云 |
| `localization.launch.py` | Open3D 全局定位 |
| `global_planning.launch.py` | 地图发布、PRM 和 Bezier 优化 |
| `perception.launch.py` | 在线障碍物提取 |
| `local_planning.launch.py` | NMPC 局部规划、控制及可选 Go2 适配 |
| `navigation.launch.py` | 导航模块组合入口 |
| `full_stack.launch.py` | 建图、定位、导航全栈组合入口 |

组合入口支持 `enable_mapping`、`enable_localization`、`enable_navigation`、`enable_global_planning`、`enable_obstacles` 和 `enable_local_planning` 等开关。定位启用时，`full_stack.launch.py` 默认关闭静态地图重复发布，避免两个节点同时占用 `/map`。

## 三种入口

### 1. 建图

```bash
ros2 launch nav_bringup mapping.launch.py \
  sensor:=livox \
  lidar_topic:=/livox/lidar \
  imu_topic:=/livox/imu \
  save_map:=true
```

RoboSense：

```bash
ros2 launch nav_bringup mapping.launch.py \
  sensor:=robosense \
  lidar_topic:=/rslidar_points \
  imu_topic:=/IMU
```

FAST-LIO 发布 `/Odometry_loc`、`/cloud_registered` 和 TF。地图写入 `${NAV3D_DATA_ROOT}/point_cloud`。

停止建图后，可独立生成导航使用的可通行区域地图：

```bash
ros2 launch nav_bringup map_processing.launch.py \
  point_cloud_file:=/data/point_cloud/scans.pcd \
  trajectory_file:=/data/trace_data/mapping_trajectory.txt \
  output_file:=/data/traversable/traversable_areas.pcd
```

`mapping.launch.py` 默认同步记录 `/Odometry_loc` 轨迹；不需要时设置 `record_trajectory:=false`。

### 2. 已有地图定位

先运行 FAST-LIO 获取连续里程计，再启动 ICP 全局定位：

```bash
ros2 launch nav_bringup localization.launch.py \
  map_path:=/data/point_cloud/scans.pcd \
  lidar_topic:=/cloud_registered \
  odom_topic:=/Odometry_loc
```

在 RViz 2 发布 `/initialpose` 后，定位节点估计并持续发布 `map -> camera_init`，同时输出 `/localization_3d` 和 `/localization_3d_confidence`。

### 3. 导航

```bash
ros2 launch nav_bringup navigation.launch.py \
  traversable_map:=/data/traversable/traversable_areas.pcd \
  static_map:=/data/point_cloud/scans.pcd \
  lidar_topic:=/cloud_registered_body \
  enable_obstacles:=true \
  use_go2:=false
```

设置目标后数据链路为：

```text
/goal_pose -> planning_3d -> /path -> Bezier -> /path_smooth
           -> NMPC local planner -> local plan -> NMPC controller -> /cmd_vel
PointCloud2 -> obstacle_processor -> /obs_raw ------------^
```

Go2 实机执行时设置 `use_go2:=true`，并确保容器能访问机器人网卡和 Unitree SDK2 Python 包。

## 主要接口

| 接口 | 类型 | 说明 |
|---|---|---|
| `/livox/lidar`、`/rslidar_points` | PointCloud2/CustomMsg | 原始 LiDAR |
| `/livox/imu`、`/IMU` | sensor_msgs/Imu | IMU |
| `/Odometry_loc` | nav_msgs/Odometry | FAST-LIO 里程计 |
| `/cloud_registered` | sensor_msgs/PointCloud2 | 配准点云 |
| `/initialpose` | PoseWithCovarianceStamped | 全局定位初值 |
| `/localization_3d` | PoseStamped | 地图坐标定位结果 |
| `/goal_pose` | PoseStamped | ROS 2 导航目标 |
| `/path_smooth` | nav_msgs/Path | 平滑全局路径 |
| `/obs_raw` | Float32MultiArray | 动态障碍 XYZ 数组 |
| `/cmd_vel` | geometry_msgs/Twist | 机器人速度指令 |

## ROS 发行版与架构

- ROS 2 Humble / Ubuntu 22.04
- ROS 2 Jazzy / Ubuntu 24.04
- `linux/amd64`、`linux/arm64`
- DDS 实现可通过 `RMW_IMPLEMENTATION` 切换；所有节点使用标准 ROS 2 接口

硬件驱动对内核、网卡、广播和设备权限仍有客观要求。Docker Compose 默认使用 host network、host IPC 和 `/dev` 映射。
