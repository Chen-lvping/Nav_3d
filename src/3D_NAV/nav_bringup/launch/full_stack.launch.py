import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    launch_dir = os.path.join(get_package_share_directory("nav_bringup"), "launch")

    mapping = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(launch_dir, "mapping.launch.py")),
        launch_arguments={
            "sensor": LaunchConfiguration("sensor"),
            "lidar_topic": LaunchConfiguration("raw_lidar_topic"),
            "imu_topic": LaunchConfiguration("imu_topic"),
            "save_map": LaunchConfiguration("save_map"),
            "record_trajectory": LaunchConfiguration("record_trajectory"),
            "trajectory_output": LaunchConfiguration("trajectory_output"),
            "odom_topic": LaunchConfiguration("odom_topic"),
            "start_sensor_driver": LaunchConfiguration("start_sensor_driver"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_mapping")),
    )
    localization = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "localization.launch.py")
        ),
        launch_arguments={
            "map_path": LaunchConfiguration("static_map"),
            "lidar_topic": LaunchConfiguration("registered_lidar_topic"),
            "odom_topic": LaunchConfiguration("odom_topic"),
            "map_frame": LaunchConfiguration("map_frame"),
            "odom_frame": LaunchConfiguration("odom_frame"),
            "base_frame": LaunchConfiguration("base_frame"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_localization")),
    )
    navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "navigation.launch.py")
        ),
        launch_arguments={
            "traversable_map": LaunchConfiguration("traversable_map"),
            "static_map": LaunchConfiguration("static_map"),
            "lidar_topic": LaunchConfiguration("body_lidar_topic"),
            "map_topic": LaunchConfiguration("map_topic"),
            "map_frame": LaunchConfiguration("map_frame"),
            "lidar_frame": LaunchConfiguration("base_frame"),
            "base_frame": LaunchConfiguration("base_frame"),
            "enable_global_planning": LaunchConfiguration("enable_global_planning"),
            "enable_obstacles": LaunchConfiguration("enable_obstacles"),
            "enable_local_planning": LaunchConfiguration("enable_local_planning"),
            "publish_static_map": LaunchConfiguration("publish_static_map"),
            "use_go2": LaunchConfiguration("use_go2"),
            "start_in_auto": LaunchConfiguration("start_in_auto"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_navigation")),
    )

    return LaunchDescription([
        DeclareLaunchArgument("enable_mapping", default_value="true"),
        DeclareLaunchArgument("enable_localization", default_value="true"),
        DeclareLaunchArgument("enable_navigation", default_value="true"),
        DeclareLaunchArgument("enable_global_planning", default_value="true"),
        DeclareLaunchArgument("enable_obstacles", default_value="true"),
        DeclareLaunchArgument("enable_local_planning", default_value="true"),
        DeclareLaunchArgument("sensor", default_value="livox"),
        DeclareLaunchArgument("start_sensor_driver", default_value="true"),
        DeclareLaunchArgument("save_map", default_value="false"),
        DeclareLaunchArgument("record_trajectory", default_value="true"),
        DeclareLaunchArgument("raw_lidar_topic", default_value="/livox/lidar"),
        DeclareLaunchArgument("imu_topic", default_value="/livox/imu"),
        DeclareLaunchArgument(
            "registered_lidar_topic", default_value="/cloud_registered"
        ),
        DeclareLaunchArgument(
            "body_lidar_topic", default_value="/cloud_registered_body"
        ),
        DeclareLaunchArgument("odom_topic", default_value="/Odometry_loc"),
        DeclareLaunchArgument(
            "trajectory_output",
            default_value=os.path.join(
                data_root, "trace_data", "mapping_trajectory.txt"
            ),
        ),
        DeclareLaunchArgument("map_topic", default_value="/map"),
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("odom_frame", default_value="camera_init"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        DeclareLaunchArgument(
            "static_map",
            default_value=os.path.join(data_root, "point_cloud", "scans.pcd"),
        ),
        DeclareLaunchArgument(
            "traversable_map",
            default_value=os.path.join(
                data_root, "traversable", "traversable_areas.pcd"
            ),
        ),
        # Localization publishes the latched static map in the default full stack.
        DeclareLaunchArgument("publish_static_map", default_value="false"),
        DeclareLaunchArgument("use_go2", default_value="false"),
        DeclareLaunchArgument("start_in_auto", default_value="false"),
        mapping,
        localization,
        navigation,
    ])
