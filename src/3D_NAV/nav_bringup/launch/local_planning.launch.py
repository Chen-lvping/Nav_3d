from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("base_frame", default_value="body"),
        DeclareLaunchArgument("start_in_auto", default_value="false"),
        DeclareLaunchArgument("use_go2", default_value="false"),
        Node(
            package="nmpc_planner",
            executable="nmpc_local_planner_node",
            output="screen",
            parameters=[{
                "map_frame": LaunchConfiguration("map_frame"),
                "base_frame": LaunchConfiguration("base_frame"),
            }],
        ),
        Node(
            package="nmpc_planner",
            executable="nmpc_controller_node",
            output="screen",
            parameters=[{
                "map_frame": LaunchConfiguration("map_frame"),
                "base_frame": LaunchConfiguration("base_frame"),
                "start_in_auto": ParameterValue(
                    LaunchConfiguration("start_in_auto"), value_type=bool
                ),
            }],
        ),
        Node(
            package="go2_base_controller",
            executable="go2_base_controller",
            output="screen",
            condition=IfCondition(LaunchConfiguration("use_go2")),
        ),
    ])
