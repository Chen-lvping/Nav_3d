import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    return LaunchDescription([
        DeclareLaunchArgument("map_path", default_value=os.path.join(data_root, "point_cloud", "scans.pcd")),
        DeclareLaunchArgument("lidar_topic", default_value="/cloud_registered"),
        DeclareLaunchArgument("odom_topic", default_value="/Odometry_loc"),
        Node(
            package="open3d_loc", executable="global_localization_node", output="screen",
            parameters=[{
                "map_path": LaunchConfiguration("map_path"),
                "scan_topic": LaunchConfiguration("lidar_topic"),
                "odom_topic": LaunchConfiguration("odom_topic"),
                "map_frame": "map", "odom_frame": "camera_init", "base_frame": "body",
                "frequency": 2.0, "voxel_size": 0.2,
            }]),
    ])
