# Nav_3d

Nav_3d is a ROS 1 3D navigation workspace for legged and mobile robots. The
reusable navigation chain is separated from sensor drivers and robot SDKs so a
new platform only needs to satisfy a small set of ROS topic and TF contracts.

```text
map + trajectory -> traversable-area extraction
localization + traversable PCD -> PRM/A* -> Bezier -> NMPC -> /cmd_vel
point cloud + static map -> dynamic obstacles --------------------^
```

## Supported baseline

- ROS Noetic on Ubuntu 20.04, or the provided Docker image
- `amd64` and `arm64` for the generic algorithm stack
- PCL 1.10, Eigen 3, and CasADi 3.5.5 with IPOPT
- Included profiles: generic ROS interface, Livox MID360 + Go2, and M20

"Any platform" means any host and robot that can provide the documented ROS 1
interfaces. Native ROS Noetic is tied to Ubuntu 20.04; use Docker on newer
Ubuntu releases or other Linux distributions. Robot SDKs and kernel drivers
may still impose their own CPU, OS, and network restrictions.

## Quick start

Clone and build the generic stack:

```bash
git clone https://github.com/Chen-lvping/Nav_3d.git nav_3d_ws
cd nav_3d_ws
source scripts/nav3d_env.sh
scripts/build_workspace.sh core
```

Place maps outside Git, or under `src/data`, then set the data root:

```bash
export NAV3D_DATA_ROOT=/absolute/path/to/nav3d_data
source devel/setup.bash
roslaunch nav_bringup bringup_navigation.launch \
  pcd_path:="$NAV3D_DATA_ROOT/traversable/traversable_areas.pcd"
```

The generic launch does not start a sensor driver, localization node, or robot
bridge. It expects `map -> base_link` and publishes `/cmd_vel`; enable optional
static-map and obstacle inputs with launch arguments after their contracts are
available.

Docker build and run:

```bash
docker build -f docker/Dockerfile -t nav3d:noetic .
docker run --rm -it --network host \
  -v /absolute/path/to/nav3d_data:/data \
  -e NAV3D_DATA_ROOT=/data nav3d:noetic
```

## Documentation

- [Architecture and package ownership](docs/ARCHITECTURE.md)
- [Native and Docker deployment](docs/DEPLOYMENT.md)
- [Sensor and robot porting guide](docs/PORTING_GUIDE.md)
- [ROS topic, TF, and file contracts](docs/INTERFACES.md)
- [Runtime data policy](src/data/README.md)
- [Existing MID360/M20 runbooks](src/3D_NAV/README.md)

## Repository layout

```text
src/3D_NAV/
  localization/       sensor-specific localization and map publication
  map_process/        offline traversable-area extraction
  global_planner/     PRM/A* planning and Bezier smoothing
  local_planner/      NMPC local planning and velocity control
  obstacle_processor/ dynamic obstacle extraction
  go2_base_controller example /cmd_vel-to-platform bridge
  nav_bringup/        generic and platform integration launches
scripts/              environment, build, and portability checks
docker/               reproducible ROS Noetic build environment
```

Runtime bags, point clouds, generated maps, build trees, machine credentials,
and network-specific values are intentionally excluded from Git. See
[third-party notes](docs/THIRD_PARTY.md) before redistributing a derived image.
