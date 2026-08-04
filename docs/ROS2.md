# ROS 2 compatibility

## Design

The existing planner and controller are mature ROS 1 nodes. Rewriting every
node at once would change algorithm behavior and couple the release to one ROS
2 distribution. This branch instead adds a small ROS 2 `ament_python` package
that translates configured standard messages through a private rosbridge
websocket connection.

This design supports Humble and Jazzy from the same source, allows ROS 1 and
ROS 2 to run on their officially supported Ubuntu releases, and works across a
Compose network on Linux, Windows, and macOS hosts.

## Default topic mapping

| Topic | Direction | Type |
| --- | --- | --- |
| `/planning_3d_PRM_node/goal_pose` | ROS 2 -> ROS 1 | `PoseStamped` |
| `/planning_3d_PRM_node/planned_path` | ROS 1 -> ROS 2 | `Path` |
| `/path_smooth`, `/local_path` | ROS 1 -> ROS 2 | `Path` |
| `/navigation_state` | ROS 1 -> ROS 2 | `Int32` |
| `/navigation_state_debug` | ROS 1 -> ROS 2 | `String` |
| `/cmd_vel` | ROS 1 -> ROS 2 | `Twist` |
| `/tf`, `/tf_static` | ROS 1 -> ROS 2 | `TFMessage` |

`/odom` and the LiDAR point cloud are provided as disabled examples. Dense
point clouds encoded as rosbridge JSON consume substantial CPU and bandwidth;
for production sensors, run the driver/localization beside the ROS 1 core or
use a dedicated binary bridge and bridge only the resulting TF/odometry.

## Configuration

Each entry in `config/bridge.yaml` has:

- `topic`, `ros1_type`, and `ros2_type`
- `direction`: `ros1_to_ros2` or `ros2_to_ros1`
- optional `depth`, `reliability`, `durability`, `throttle_ms`, and `enabled`

Only standard message packages installed in both containers work by default.
For custom messages, install the ROS 1 definition in the core image and the
equivalent ROS 2 definition in the adapter image before adding the mapping.
Services, actions, and ROS parameters are not translated by this adapter.

## Native launch

Start ROS 1 and rosbridge:

```bash
roscore &
roslaunch rosbridge_server rosbridge_websocket.launch port:=9090
```

In a ROS 2 shell:

```bash
bash scripts/build_ros2.sh
source ros2_ws/install/setup.bash
ros2 launch nav3d_ros2_adapter bridge.launch.py \
  rosbridge_host:=127.0.0.1 rosbridge_port:=9090
```

Smoke test:

```bash
ros2 topic list
ros2 topic echo /navigation_state_debug
ros2 topic pub --once /planning_3d_PRM_node/goal_pose \
  geometry_msgs/msg/PoseStamped '{header: {frame_id: map}, pose: {orientation: {w: 1.0}}}'
```

## Production notes

- The Compose websocket port is not published to the host. If exposed outside
  a trusted network, add TLS, authentication, and a firewall.
- Keep `ROS_DOMAIN_ID` consistent with the ROS 2 robot or visualization host.
- Use a command watchdog and emergency stop in the ROS 2 base adapter; a
  bridged `/cmd_vel` topic is not a safety controller.
- On Docker Desktop, use explicit TCP/UDP port mappings or an external DDS
  router when ROS 2 discovery must cross the VM boundary.
