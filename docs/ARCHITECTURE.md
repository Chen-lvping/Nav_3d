# Architecture

## Stage boundaries

Nav_3d is split into an offline map pipeline and an online navigation pipeline.
The split is deliberate: changing a LiDAR, localization system, or robot base
does not require changing the PRM, Bezier, or NMPC algorithms.

```text
OFFLINE
sensor/SLAM -> raw scene PCD + trajectory -> map_process -> traversable PCD

ONLINE
localization: map -> base_link
traversable PCD -> planning_3d -> planned_path
planned_path -> bezier_path_optimizer -> /path_smooth
/path_smooth + TF + /obs_raw -> nmpc_planner -> /cmd_vel
/cmd_vel -> platform bridge -> robot SDK or motor controller

OPTIONAL OBSTACLES
live PointCloud2 + static map -> obstacle_processor -> /obs_raw
```

The traversable PCD contains valid free/traversable samples. It is not an
obstacle map. `planning_3d` builds its PRM when it starts, so replace the PCD and
restart the planner when the offline map changes.

## Package ownership

| Package | Responsibility | Portability |
| --- | --- | --- |
| `map_process` | PCD + trajectory to traversable PCD | Generic |
| `planning_3d` | Traversable voxel model, PRM, A* | Generic |
| `bezier_path_optimizer` | Global path smoothing | Generic |
| `nmpc_planner` | Local path and `/cmd_vel` | Generic, CasADi required |
| `obstacle_processor` | Live obstacle extraction | Generic after topic/TF remap |
| `map_publisher` | Static point-cloud map publication | Generic |
| `FAST_LIO`, `galileo_lio` | LiDAR/IMU localization | Sensor-specific |
| `livox_ros_driver2`, `rslidar_sdk` | Sensor transport | Hardware-specific |
| `go2_base_controller` | `/cmd_vel` to Unitree SDK | Robot-specific example |
| `nav_bringup` | Integration and launch arguments | Integration boundary |

## Configuration layers

Use these layers in order. Do not hard-code a developer home directory in
source code.

1. Environment: `NAV3D_DATA_ROOT`, `CASADI_ROOT`, `CASADI_LIB_PATH`.
2. Launch arguments: file paths, topic names, frames, feature switches.
3. Package YAML: algorithm parameters for one sensor or robot profile.
4. Platform bridge configuration: network interface, peer address, SDK mode.

`nav_bringup/bringup_navigation.launch` is the sensor- and base-neutral entry
point. MID360, Go2, and M20 launches are concrete examples built around it.
