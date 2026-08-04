import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    sensor = LaunchConfiguration("sensor")
    lidar_topic = LaunchConfiguration("lidar_topic")
    imu_topic = LaunchConfiguration("imu_topic")
    save_map = LaunchConfiguration("save_map")
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    livox_launch = os.path.join(
        get_package_share_directory("livox_ros_driver2"), "launch_ROS2", "msg_MID360_launch.py")
    return LaunchDescription([
        DeclareLaunchArgument("sensor", default_value="livox"),
        DeclareLaunchArgument("lidar_topic", default_value="/livox/lidar"),
        DeclareLaunchArgument("imu_topic", default_value="/livox/imu"),
        DeclareLaunchArgument("save_map", default_value="true"),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(livox_launch),
            condition=IfCondition(PythonExpression(["'", sensor, "' == 'livox'"]))),
        Node(
            package="rslidar_sdk", executable="rslidar_sdk_node", output="screen",
            condition=IfCondition(PythonExpression(["'", sensor, "' == 'robosense'"]))),
        Node(
            package="fast_lio", executable="fastlio_mapping", name="fast_lio_node", output="screen",
            parameters=[{
                "common/lid_topic": lidar_topic,
                "common/imu_topic": imu_topic,
                "common/time_sync_en": False,
                "preprocess/lidar_type": PythonExpression(["1 if '", sensor, "' == 'livox' else 5"]),
                "preprocess/scan_line": PythonExpression(["4 if '", sensor, "' == 'livox' else 96"]),
                "preprocess/scan_rate": 10,
                "preprocess/timestamp_unit": 2,
                "preprocess/blind": 0.2,
                "mapping/extrinsic_est_en": False,
                "mapping/extrinsic_T": [0.0, 0.0, 0.0],
                "mapping/extrinsic_R": [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0],
                "publish/path_en": True,
                "publish/scan_publish_en": True,
                "publish/dense_publish_en": True,
                "publish/scan_bodyframe_pub_en": True,
                "pcd_save/pcd_save_en": save_map,
                "pcd_save/output_dir": os.path.join(data_root, "point_cloud"),
            }]),
    ])
