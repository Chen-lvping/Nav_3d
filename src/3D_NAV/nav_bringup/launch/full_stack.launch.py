from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    launch_dir = os.path.join(get_package_share_directory("nav_bringup"), "launch")
    return LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(os.path.join(launch_dir, "mapping.launch.py")),
            launch_arguments={"save_map": "false"}.items()),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(os.path.join(launch_dir, "localization.launch.py"))),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(os.path.join(launch_dir, "navigation.launch.py"))),
    ])
