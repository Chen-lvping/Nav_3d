import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    return LaunchDescription([
        DeclareLaunchArgument("map_path", default_value=os.path.join(data_root, "point_cloud", "scans.pcd")),
        DeclareLaunchArgument("lidar_topic", default_value="/cloud_registered"),
        DeclareLaunchArgument("odom_topic", default_value="/Odometry_loc"),
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("odom_frame", default_value="camera_init"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        DeclareLaunchArgument("frequency", default_value="2.0"),
        DeclareLaunchArgument("voxel_size", default_value="0.2"),
        Node(
            package="open3d_loc", executable="global_localization_node", output="screen",
            parameters=[{
                "map_path": LaunchConfiguration("map_path"),
                "scan_topic": LaunchConfiguration("lidar_topic"),
                "odom_topic": LaunchConfiguration("odom_topic"),
                "map_frame": LaunchConfiguration("map_frame"),
                "odom_frame": LaunchConfiguration("odom_frame"),
                "base_frame": LaunchConfiguration("base_frame"),
                "frequency": ParameterValue(
                    LaunchConfiguration("frequency"), value_type=float
                ),
                "voxel_size": ParameterValue(
                    LaunchConfiguration("voxel_size"), value_type=float
                ),
            }]),
    ])
