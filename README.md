# Nav_3d — Native ROS 2

本分支将项目重构为原生 ROS 2 Humble 算法栈，默认路径不再依赖 ROS 1、catkin、roscore、
rosbridge 或 ROS 1 API 兼容层。容器内提供确定性传感器仿真，可完整验证：

```text
LaserScan + mapping pose -> log-odds occupancy mapping -> map.pgm/yaml
LaserScan + wheel odom + map -> correlative scan matching -> map->odom TF
map + localized pose + goal -> obstacle inflation + A* + smoothing -> Path
Path + localized pose -> pure pursuit -> cmd_vel -> kinematic simulator
```

## 一键 Docker 验收

```bash
git clone -b ros2-native-rebuild-20260817 https://github.com/Chen-lvping/Nav_3d.git
cd Nav_3d
docker compose build
docker compose run --rm nav3d
cat artifacts/demo_result.json
```

成功结果为 `"status": "PASS"`，同时生成 `artifacts/demo_map.pgm`、
`artifacts/demo_map.yaml` 和 `artifacts/demo.log`。

## 本机 ROS 2 Humble

```bash
source /opt/ros/humble/setup.bash
./scripts/build_workspace.sh
source install/setup.bash
./scripts/validate_native.sh
```

## 包结构

- `src/nav3d_native/nav3d_native/algorithms.py`：与 ROS 解耦的建图、定位、A*、平滑、控制算法
- `mapping_node.py`：原生 `rclpy` 占据栅格建图与标准地图文件输出
- `localization_node.py`：相关扫描匹配定位与 TF2 发布
- `planner_node.py`：障碍膨胀 A* 全局规划
- `controller_node.py`：路径跟踪和 `/cmd_vel`
- `simulator_node.py`：仅用于无实机闭环验证
- `demo_supervisor.py`：量化验收与 JSON 证据输出

旧 ROS 1 源码保留在 `src/3D_NAV` 并由 `COLCON_IGNORE` 隔离；旧容器定义保留为
`docker/Dockerfile.ros1-legacy`。实机驱动不在本阶段启动。

## 文档

- [运行环境与实机边界](docs/ENVIRONMENT.md)
- [原生 ROS 2 架构](docs/ARCHITECTURE.md)
- [Docker、本机部署与验收](docs/DEPLOYMENT.md)
- [ROS 2 话题、TF 与参数接口](docs/INTERFACES.md)
- [实机和 3D 前端迁移指南](docs/PORTING_GUIDE.md)
