from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("lidar_topic", default_value="/cloud_registered_body"),
        DeclareLaunchArgument("map_topic", default_value="/map"),
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("lidar_frame", default_value="body"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        Node(
            package="obstacle_processor",
            executable="obstacle_processor_node",
            output="screen",
            parameters=[{
                "lidar_topic": LaunchConfiguration("lidar_topic"),
                "world_map_topic": LaunchConfiguration("map_topic"),
                "map/map_frame_id": LaunchConfiguration("map_frame"),
                "map/lidar_frame_id": LaunchConfiguration("lidar_frame"),
                "map/base_frame_id": LaunchConfiguration("base_frame"),
            }],
        ),
    ])
