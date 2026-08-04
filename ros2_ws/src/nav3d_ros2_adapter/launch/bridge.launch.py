from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
import os


def generate_launch_description():
    default_config = os.path.join(
        get_package_share_directory("nav3d_ros2_adapter"), "config", "bridge.yaml"
    )
    return LaunchDescription([
        DeclareLaunchArgument("config", default_value=default_config),
        DeclareLaunchArgument("rosbridge_host", default_value="ros1"),
        DeclareLaunchArgument("rosbridge_port", default_value="9090"),
        Node(
            package="nav3d_ros2_adapter",
            executable="bridge",
            output="screen",
            parameters=[{
                "config": LaunchConfiguration("config"),
                "rosbridge_host": LaunchConfiguration("rosbridge_host"),
                "rosbridge_port": ParameterValue(
                    LaunchConfiguration("rosbridge_port"), value_type=int
                ),
            }],
        ),
    ])
