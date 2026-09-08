# Nav_3d — ROS 2 Humble 三维导航

当前发布分支：`ros2-native-rebuild-20260908`。本分支基于原生 ROS 2 Humble，保留可重复的
二维建图/定位/规划/控制闭环，并包含 ROS 2 版 MID360、FAST-LIO、三维规划、Bezier
平滑、NMPC、RViz2 三维目标工具和 Go2 实机接入代码。

完整操作手册见 [docs/RUN_GUIDE_ROS2_CONTAINER.md](docs/RUN_GUIDE_ROS2_CONTAINER.md)，
2026-09-08 的发布验证记录见 [docs/VALIDATION_20260908.md](docs/VALIDATION_20260908.md)。

## 1. 获取当前分支

```bash
git clone -b ros2-native-rebuild-20260908 \
  https://github.com/Chen-lvping/Nav_3d.git \
  /home/nuc/Nav_3d-ros2-native-rebuild-20260817
cd /home/nuc/Nav_3d-ros2-native-rebuild-20260817
```

工程目录名继续使用 `20260817`，是为了兼容现场已有容器挂载与第 674 行的一键脚本路径；
实际代码版本以 Git 分支和提交为准。

## 2. 离线闭环一键验收

从当前源码重建并验收：

```bash
docker compose build
docker compose run --rm nav3d
python3 -m json.tool artifacts/demo_result.json
```

返回码为 0 且 `status` 为 `PASS` 才表示通过。验收覆盖：

```text
LaserScan + mapping pose
  -> OccupancyGrid
  -> scan-matching localization
  -> inflated A* + smoothing
  -> pure pursuit cmd_vel
  -> kinematic simulator
```

如果拿到的是随项目交付的 `ros2_nav_3d.tar.gz`，它是 2026-08-29 的完整硬件依赖镜像，
包含固定版本的 Livox SDK2、Unitree SDK2 Python 和 CasADi。先加载并检查镜像：

```bash
docker load -i /path/to/ros2_nav_3d.tar.gz
docker image inspect ros2_nav_3d:latest
```

它是操作手册第 9～18 节使用的实机/三维运行环境，不会被 Compose 的二维验收自动选中。
镜像本身不包含 GitHub 代码更新；运行时仍应把当前工程挂载到 `/workspace/Nav_3d`。要验证
ZIP 中更新后的二维基线源码，执行前面的 `docker compose build`。

## 3. 最新源码的附加测试

```bash
python3 -m pytest -q src/nav3d_hardware_bringup/test
PYTHONPATH=src/nav3d_native python3 -m pytest -q src/nav3d_native/test
```

主要 ROS 2 包：

- `nav3d_native`：确定性二维闭环与基础算法验收；
- `livox_ros_driver2`、`fast_lio`：MID360 与 LIO；
- `planning_3d_ros2`：三维 PRM 路径规划与 Bezier 平滑；
- `nmpc_planner_ros2`：局部轨迹与控制接口；
- `rviz_3d_nav_goal_tool_ros2`：RViz2 的 `3D Nav Goal` 工具；
- `nav3d_hardware_bringup`：外参、里程计适配、安全门禁、离线仿真和实机 bringup。

## 4. 离线三维导航

在文档规定的常驻容器中构建当前源码后执行：

```bash
cd /home/nuc/Nav_3d-ros2-native-rebuild-20260817
./scripts/run_offline_navigation_rviz.sh
```

RViz2 中使用 `3D Nav Goal` 选点，`/navigation_state` 最终应为 `4`（`COMPLETED`）。
默认 `path` 模式由虚拟底盘沿三维参考路径运动；传统平面速度模式可用：

```bash
NAV3D_MOTION_MODE=cmd_vel ./scripts/run_offline_navigation_rviz.sh
```

## 5. 实机一键启动

该入口会控制真实 Go2。只允许在操作手册第 18 节的全部现场前提满足时执行：

```bash
/home/nuc/Nav_3d-ros2-native-rebuild-20260817/scripts/run_hardware_navigation_rviz.sh
```

脚本先检查常驻容器 `Nav_3d_ros2`、Go2 网卡/地址、实时 `/lio/odom` 和 RViz 显示权限；
全部通过后仍需输入 `START_REAL_NAVIGATION` 才会解锁。停止时优先使用遥控器急停，启动终端
按 `Ctrl+C` 会发送连续零速度；终端丢失时执行：

```bash
/home/nuc/Nav_3d-ros2-native-rebuild-20260817/scripts/stop_hardware_navigation.sh
```

重要边界：工程没有全局地图重定位。机器狗必须放在旧地图建图起点并保持当时朝向，否则
RViz2 中的路线会相对实机整体错位。

## 6. 仓库内容边界

GitHub 分支只保存源码、配置、脚本和文档。以下内容不进入 Git：Docker 镜像包、ZIP 交付包、
编译产物、运行日志、地图/PCD 和现场采集数据。大文件应继续通过独立制品存储交付。
