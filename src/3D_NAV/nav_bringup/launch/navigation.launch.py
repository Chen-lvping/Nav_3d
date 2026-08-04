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

    global_planning = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "global_planning.launch.py")
        ),
        launch_arguments={
            "traversable_map": LaunchConfiguration("traversable_map"),
            "static_map": LaunchConfiguration("static_map"),
            "map_frame": LaunchConfiguration("map_frame"),
            "base_frame": LaunchConfiguration("base_frame"),
            "publish_static_map": LaunchConfiguration("publish_static_map"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_global_planning")),
    )
    perception = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "perception.launch.py")
        ),
        launch_arguments={
            "lidar_topic": LaunchConfiguration("lidar_topic"),
            "map_topic": LaunchConfiguration("map_topic"),
            "map_frame": LaunchConfiguration("map_frame"),
            "lidar_frame": LaunchConfiguration("lidar_frame"),
            "base_frame": LaunchConfiguration("base_frame"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_obstacles")),
    )
    local_planning = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "local_planning.launch.py")
        ),
        launch_arguments={
            "map_frame": LaunchConfiguration("map_frame"),
            "base_frame": LaunchConfiguration("base_frame"),
            "start_in_auto": LaunchConfiguration("start_in_auto"),
            "use_go2": LaunchConfiguration("use_go2"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("enable_local_planning")),
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            "traversable_map",
            default_value=os.path.join(
                data_root, "traversable", "traversable_areas.pcd"
            ),
        ),
        DeclareLaunchArgument(
            "static_map",
            default_value=os.path.join(data_root, "point_cloud", "scans.pcd"),
        ),
        DeclareLaunchArgument("lidar_topic", default_value="/cloud_registered_body"),
        DeclareLaunchArgument("map_topic", default_value="/map"),
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("lidar_frame", default_value="body"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        DeclareLaunchArgument("enable_global_planning", default_value="true"),
        DeclareLaunchArgument("enable_obstacles", default_value="true"),
        DeclareLaunchArgument("enable_local_planning", default_value="true"),
        DeclareLaunchArgument("publish_static_map", default_value="true"),
        DeclareLaunchArgument("use_go2", default_value="false"),
        DeclareLaunchArgument("start_in_auto", default_value="false"),
        global_planning,
        perception,
        local_planning,
    ])
