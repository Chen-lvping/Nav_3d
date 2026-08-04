# 原生 ROS 2 使用说明

## 结论

当前 `ros2-native` 分支的生产链路是全栈原生 ROS 2：建图、定位、导航和底盘控制均直接使用 ROS 2 消息、服务、参数、DDS QoS 与 TF2。不存在 ROS 1 master 或桥接进程。

旧的 `galileo_lio`、ROS 1 RViz 插件和测试包保留为参考源码，并通过 `COLCON_IGNORE` 排除，不参与构建或运行。

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
  use_obstacles:=true \
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
