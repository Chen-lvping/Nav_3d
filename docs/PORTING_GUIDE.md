# Porting guide

## Add a sensor or localization source

The navigation core does not require a particular LiDAR brand. Adapt the new
sensor below the interface boundary:

1. Publish a time-correct `sensor_msgs/PointCloud2` stream and IMU data needed
   by the selected localization system.
2. Configure extrinsics and publish a connected sensor-to-base TF.
3. Make localization publish a stable `map -> base_link` transform.
4. During mapping, export a `PointXYZI` PCD and a trajectory accepted by
   `map_process`.
5. If online obstacle processing is required, remap `lidar_topic` and set
   `lidar_frame` in the generic bringup.

Keep driver IP addresses, UDP ports, scan patterns, timestamps, and extrinsics
in a sensor-specific YAML/launch file. Do not add them to planner code.

## Add a robot base

Create a separate ROS package whose only upper-stack dependency is the standard
ROS message interface. The minimum adapter is:

```text
/cmd_vel (geometry_msgs/Twist)
  -> velocity limits + watchdog + enable/estop gate
  -> vendor SDK, CAN, serial, DDS, or ros_control

platform state
  -> /odom and odom -> base_link TF when localization needs them
```

Validate in this order:

1. SDK/transport read-only probe.
2. Robot state and mode checks.
3. Zero-velocity command.
4. Small manually bounded command with the navigation stack stopped.
5. `/cmd_vel` bridge subscription and watchdog.
6. Full navigation at reduced limits.

`go2_base_controller` is an example, not a base class. M20 uses an external
ROS1/ROS2 bridge and motion adapter. New platforms should preserve the same
boundary instead of adding SDK calls to `nmpc_planner`.

## Tune robot geometry and dynamics

At minimum review:

- planner: `safe_margin`, `robot_height`, `max_slope_deg`, headroom settings
- obstacle processor: crop bounds, height thresholds, LiDAR height compensation
- NMPC: max linear/angular velocity, safe distance, influence distance
- bridge: acceleration limits, command timeout, supported lateral velocity

Start with obstacle processing disabled in a cleared test area, then enable and
verify `/obs_raw` independently. A healthy `/cmd_vel` topic does not prove the
robot transport or SDK is healthy.

## Acceptance checklist

```text
[ ] target CPU can build or run the Docker image
[ ] map, base, and sensor frames form one TF tree
[ ] planner loads the intended traversable PCD
[ ] a goal produces planned_path and path_smooth
[ ] NMPC publishes bounded /cmd_vel
[ ] platform bridge watchdog stops stale commands
[ ] emergency stop is available during every motion test
[ ] sensor/network values live in a profile, not source code
```
