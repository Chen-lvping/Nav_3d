# Deployment

## Native ROS Noetic

The supported native baseline is Ubuntu 20.04 with ROS Noetic.

```bash
sudo apt install python3-rosdep python3-catkin-tools build-essential cmake git
sudo rosdep init 2>/dev/null || true
rosdep update

cd /path/to/Nav_3d
rosdep install --from-paths src --ignore-src -r -y
source scripts/nav3d_env.sh
scripts/build_workspace.sh core
```

CasADi must provide C++ headers and libraries, not only the Python wheel. Build
the pinned version locally (requires `coinor-libipopt-dev`, `gfortran`, and
`liblapack-dev`):

```bash
scripts/install_casadi.sh
source scripts/nav3d_env.sh
```

Alternatively install CasADi 3.5.5 under `/opt/casadi` or export:

```bash
export CASADI_ROOT=/absolute/path/to/casadi
export CASADI_LIB_PATH="$CASADI_ROOT/lib"
```

Build profiles:

- `core`: mapping conversion, planner, smoother, NMPC, obstacles, bringup
- `mid360`: core plus FAST-LIO and Livox driver
- `all`: every catkin package in the workspace

## Docker

The provided image builds CasADi from source for the target CPU, then compiles
the generic navigation packages. This avoids copying an `amd64` CasADi binary
onto an `arm64` computer.

```bash
docker buildx build --platform linux/amd64,linux/arm64 \
  -f docker/Dockerfile -t your-registry/nav3d:noetic --push .
```

For local hardware access, use host networking and explicitly pass only the
devices required by the sensor or robot SDK. Mount runtime data instead of
baking maps into the image:

```bash
docker run --rm -it --network host \
  -v /path/to/nav3d_data:/data \
  -e NAV3D_DATA_ROOT=/data \
  your-registry/nav3d:noetic
```

GUI/RViz forwarding and raw USB/CAN access are host-specific and intentionally
not enabled by default.

## Runtime data layout

```text
$NAV3D_DATA_ROOT/
  point_cloud/scans.pcd
  trace_data/mapping_trajectory.txt
  traversable/traversable_areas.pcd
  bags/                         # optional, never committed
```

Run the portability check before publishing a new profile:

```bash
scripts/check_portability.sh
```
