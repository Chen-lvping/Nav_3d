import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    default_config = os.path.join(
        get_package_share_directory("map_process"),
        "config",
        "default_params.ros2.yaml",
    )

    return LaunchDescription([
        DeclareLaunchArgument("config_file", default_value=default_config),
        DeclareLaunchArgument(
            "point_cloud_file",
            default_value=os.path.join(data_root, "point_cloud", "scans.pcd"),
        ),
        DeclareLaunchArgument(
            "trajectory_file",
            default_value=os.path.join(
                data_root, "trace_data", "mapping_trajectory.txt"
            ),
        ),
        DeclareLaunchArgument(
            "output_file",
            default_value=os.path.join(
                data_root, "traversable", "traversable_areas.pcd"
            ),
        ),
        Node(
            package="map_process",
            executable="map_process_node",
            output="screen",
            parameters=[
                LaunchConfiguration("config_file"),
                {
                    "point_cloud_file": LaunchConfiguration("point_cloud_file"),
                    "trajectory_file": LaunchConfiguration("trajectory_file"),
                    "output_file": LaunchConfiguration("output_file"),
                },
            ],
        ),
    ])
