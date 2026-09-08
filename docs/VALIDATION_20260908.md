# 2026-09-08 发布验证记录

## 版本与制品边界

- 源码来源：`Nav_3d-ros2-native-rebuild-20260817.zip`，归档内文件时间最晚为
  2026-08-29；
- 随附镜像：`ros2_nav_3d.tar.gz`，加载标签为 `ros2_nav_3d:latest`，镜像创建时间为
  2026-08-29，解压后大小约 3.26 GB；
- 发布分支：`ros2-native-rebuild-20260908`；
- 镜像包、源码 ZIP、编译目录、日志、地图与 PCD 均不提交到 GitHub。

随附镜像提供 Livox SDK2、Unitree SDK2 Python、CasADi 和 ROS 2 实机/三维运行依赖；
ZIP 提供当前工作区源码和数据。二维基线另从 ZIP 当前源码执行 Docker 重建和验收。

## 已通过项目

### Compose 二维基线验收

加载交付制品前，本机已有与 2026-08-17 基线一致的 `nav3d:ros2-native` 标签。首次运行
Compose 验收结果：

```json
{
  "status": "PASS",
  "elapsed_sec": 23.51,
  "occupied_cells": 541,
  "free_cells": 7514,
  "path_poses": 29,
  "localization_error_m": 0.05,
  "goal_error_m": 0.384
}
```

Flake8 通过，`nav3d_native` 单元测试为 6 passed。

### ZIP 当前源码重建验收

执行 `docker compose build nav3d` 后重新运行 Compose 验收，结果：

```json
{
  "status": "PASS",
  "elapsed_sec": 24.0,
  "occupied_cells": 542,
  "free_cells": 7514,
  "path_poses": 29,
  "localization_error_m": 0.05,
  "goal_error_m": 0.321
}
```

Flake8 通过，`nav3d_native` 单元测试为 6 passed。

### 随附镜像 + ZIP 当前源码三维闭环

使用 `ros2_nav_3d:latest` 挂载当前工程，关闭 RViz2 后运行
`offline_navigation_sim.launch.py`，向 `/goal_pose` 发布手册中的三维目标点。结果：

- 五个节点成功启动：三维 PRM、Bezier、NMPC 局部规划、NMPC 控制器、虚拟底盘；
- `/path_smooth` 输出 21 个姿态；
- `/tracking_error` 最终为 `[0.0, 20.0, 1.0]`；
- `/navigation_state` 最终为 `4`，调试状态为 `COMPLETED`；
- 进程返回码为 0，未启动 RViz2、MID360 或 Go2 运动接口。

运行期间 CasADi QRSQP 在部分周期报告数值不收敛，系统按操作手册说明切换到已有 fallback
tracker，三维路径执行仍正常完成。这是当前实现的已知行为，不记作 NMPC 求解器稳定性通过。

### 源码附加检查

- `src/nav3d_hardware_bringup/test`：22 passed；
- `src/nav3d_native/test`：6 passed；
- `nav3d_hardware_bringup` 独立 `colcon build` 通过，五个测试套件全部通过；
- 源树只读挂载、构建和安装目录全新的条件下，`nav3d_native`、`livox_ros_driver2`、
  `fast_lio`、`planning_3d_ros2`、`nmpc_planner_ros2`、`rviz_3d_nav_goal_tool_ros2` 和
  `nav3d_hardware_bringup` 共 7 个 ROS 2 包全部构建完成；
- `nav3d_hardware_bringup` 与 `scripts` 下 Python 文件：`py_compile` 通过；
- 顶层 Shell 脚本：`bash -n` 通过。

## 实机入口检查结果

已调用操作手册第 674 行对应脚本：

```bash
/home/nuc/Nav_3d-ros2-native-rebuild-20260817/scripts/run_hardware_navigation_rviz.sh
```

本次验证主机没有现场常驻容器 `Nav_3d_ros2`，也没有文档要求的 Go2 网卡 `enp86s0`，脚本
按设计在预检阶段停止并报告：

```text
Container Nav_3d_ros2 does not exist.
```

因此，本记录证明实机入口的第一道 fail-closed 检查有效，但不声称在这台机器上完成了
MID360/Go2 实机运动验证。完整实机复验必须在文档所述 NUC、机器人、雷达、旧地图起点、
遥控器急停和清空场地全部就绪时由现场人员执行。

## 通过判据

离线闭环只有在命令返回 0、JSON 为 `PASS`、地图文件非空且全部测试通过时才记为通过。
实机部分必须通过常驻容器、网络、Go2 连通、实时 LIO、RViz 权限与人工确认全部门禁；任何
一项缺失都不得绕过。
