# Interface contracts

## Required online inputs

| Interface | Type | Producer requirement |
| --- | --- | --- |
| `map -> base_link` | TF | Continuous robot pose in the planner map frame |
| Traversable map | binary/ascii PCD (`PointXYZI`) | Free/traversable surface samples |
| `/planning_3d_PRM_node/goal_pose` | `geometry_msgs/PoseStamped` | Goal in `map` frame |

The frame names are launch arguments. A platform may use different native
frames, but it must publish a connected TF tree or remap them at bringup.

## Outputs and internal topics

| Topic | Type | Meaning |
| --- | --- | --- |
| `/planning_3d_PRM_node/planned_path` | `nav_msgs/Path` | Global PRM/A* path |
| `/path_smooth` | `nav_msgs/Path` | Bezier-smoothed reference |
| `/local_plan` | controller-local plan message flow | NMPC tracking input |
| `/local_path` | `nav_msgs/Path` | Local-plan visualization |
| `/cmd_vel` | `geometry_msgs/Twist` | Platform-neutral velocity command |
| `/navigation_state` | `std_msgs/Int32` | Navigation state machine value |
| `/navigation_state_debug` | `std_msgs/String` | Human-readable state |

## Optional obstacle input

`obstacle_processor` consumes a `sensor_msgs/PointCloud2` live cloud and a
static point-cloud map, with valid TF between `map`, the LiDAR frame, and the
robot base. Its output `/obs_raw` is consumed by the global and local planners.

Launch arguments in `bringup_navigation.launch`:

- `lidar_topic`
- `world_map_topic`
- `lidar_frame`
- `map_frame`
- `robot_frame`

Leave `start_obstacle_processor:=false` until all five are verified.

## Platform bridge contract

A new base adapter must subscribe to `/cmd_vel`, enforce a short command
timeout, publish zero motion on timeout/shutdown, clamp velocity and yaw rate,
and expose an emergency-stop or enable gate. The upper navigation stack must
not import the platform SDK directly.

Before a motion test, verify:

```bash
rostopic info /cmd_vel
rostopic echo -n 1 /navigation_state_debug
rosrun tf tf_echo map base_link
```
