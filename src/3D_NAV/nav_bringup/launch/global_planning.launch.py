import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")

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
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        DeclareLaunchArgument("publish_static_map", default_value="true"),
        Node(
            package="map_publisher",
            executable="map_publisher_node",
            output="screen",
            condition=IfCondition(LaunchConfiguration("publish_static_map")),
            parameters=[{
                "map_path": LaunchConfiguration("static_map"),
                "map_frame_id": LaunchConfiguration("map_frame"),
            }],
        ),
        Node(
            package="planning_3d",
            executable="planning_3d_PRM_node",
            output="screen",
            parameters=[{
                "pcd_path": LaunchConfiguration("traversable_map"),
                "map_frame": LaunchConfiguration("map_frame"),
                "robot_frame": LaunchConfiguration("base_frame"),
                "safe_margin": 0.2,
                "voxel_leaf": 0.2,
                "robot_height": 0.6,
                "enable_headroom_check": False,
                "step_size": 0.5,
                "max_nodes": 8000,
                "k_neigh": 30,
                "max_slope_deg": 35.0,
                "auto_replan": True,
            }],
        ),
        Node(
            package="bezier_path_optimizer",
            executable="bezier_path_optimizer_node",
            output="screen",
        ),
    ])
