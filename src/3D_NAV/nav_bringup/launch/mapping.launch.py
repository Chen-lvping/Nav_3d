from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

import os


def generate_launch_description():
    sensor = LaunchConfiguration("sensor")
    lidar_topic = LaunchConfiguration("lidar_topic")
    imu_topic = LaunchConfiguration("imu_topic")
    save_map = LaunchConfiguration("save_map")
    record_trajectory = LaunchConfiguration("record_trajectory")
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    start_sensor_driver = LaunchConfiguration("start_sensor_driver")
    sensors_launch = os.path.join(
        get_package_share_directory("nav_bringup"), "launch", "sensors.launch.py"
    )
    return LaunchDescription([
        DeclareLaunchArgument(
            "sensor", default_value="livox", choices=["livox", "robosense"]
        ),
        DeclareLaunchArgument("lidar_topic", default_value="/livox/lidar"),
        DeclareLaunchArgument("imu_topic", default_value="/livox/imu"),
        DeclareLaunchArgument("save_map", default_value="true"),
        DeclareLaunchArgument("record_trajectory", default_value="true"),
        DeclareLaunchArgument("odom_topic", default_value="/Odometry_loc"),
        DeclareLaunchArgument(
            "trajectory_output",
            default_value=os.path.join(
                data_root, "trace_data", "mapping_trajectory.txt"
            ),
        ),
        DeclareLaunchArgument("start_sensor_driver", default_value="true"),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(sensors_launch),
            launch_arguments={"sensor": sensor}.items(),
            condition=IfCondition(start_sensor_driver),
        ),
        Node(
            package="fast_lio", executable="fastlio_mapping", name="fast_lio_node", output="screen",
            parameters=[{
                "common/lid_topic": lidar_topic,
                "common/imu_topic": imu_topic,
                "common/time_sync_en": False,
                "preprocess/lidar_type": ParameterValue(
                    PythonExpression(["1 if '", sensor, "' == 'livox' else 5"]), value_type=int),
                "preprocess/scan_line": ParameterValue(
                    PythonExpression(["4 if '", sensor, "' == 'livox' else 96"]), value_type=int),
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
                "pcd_save/pcd_save_en": ParameterValue(save_map, value_type=bool),
                "pcd_save/output_dir": os.path.join(data_root, "point_cloud"),
            }]),
        Node(
            package="fast_lio",
            executable="mapping_trajectory_recorder.py",
            name="mapping_trajectory_recorder",
            output="screen",
            condition=IfCondition(record_trajectory),
            parameters=[{
                "odom_topic": LaunchConfiguration("odom_topic"),
                "output_file": LaunchConfiguration("trajectory_output"),
            }],
        ),
    ])
