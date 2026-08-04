import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node


def generate_launch_description():
    sensor = LaunchConfiguration("sensor")
    livox_launch = os.path.join(
        get_package_share_directory("livox_ros_driver2"),
        "launch_ROS2",
        "msg_MID360_launch.py",
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            "sensor",
            default_value="livox",
            choices=["livox", "robosense"],
            description="LiDAR driver module to start",
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(livox_launch),
            condition=IfCondition(PythonExpression(["'", sensor, "' == 'livox'"])),
        ),
        Node(
            package="rslidar_sdk",
            executable="rslidar_sdk_node",
            output="screen",
            condition=IfCondition(
                PythonExpression(["'", sensor, "' == 'robosense'"])
            ),
        ),
    ])
