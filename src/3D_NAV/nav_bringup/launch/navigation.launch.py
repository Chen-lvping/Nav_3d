import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    data_root = os.environ.get("NAV3D_DATA_ROOT", "/data")
    use_obstacles = LaunchConfiguration("use_obstacles")
    use_go2 = LaunchConfiguration("use_go2")
    return LaunchDescription([
        DeclareLaunchArgument("traversable_map", default_value=os.path.join(data_root, "traversable", "traversable_areas.pcd")),
        DeclareLaunchArgument("static_map", default_value=os.path.join(data_root, "point_cloud", "scans.pcd")),
        DeclareLaunchArgument("lidar_topic", default_value="/cloud_registered_body"),
        DeclareLaunchArgument("use_obstacles", default_value="true"),
        DeclareLaunchArgument("use_go2", default_value="false"),
        DeclareLaunchArgument("start_in_auto", default_value="false"),
        Node(package="map_publisher", executable="map_publisher_node", output="screen",
             parameters=[{"map_path": LaunchConfiguration("static_map"), "map_frame_id": "map"}]),
        Node(package="planning_3d", executable="planning_3d_PRM_node", output="screen",
             parameters=[{
                 "pcd_path": LaunchConfiguration("traversable_map"),
                 "map_frame": "map", "robot_frame": "body", "safe_margin": 0.2,
                 "voxel_leaf": 0.2, "robot_height": 0.6, "enable_headroom_check": False,
                 "step_size": 0.5, "max_nodes": 8000, "k_neigh": 30,
                 "max_slope_deg": 35.0, "auto_replan": True,
             }]),
        Node(package="bezier_path_optimizer", executable="bezier_path_optimizer_node", output="screen"),
        Node(package="obstacle_processor", executable="obstacle_processor_node", output="screen",
             condition=IfCondition(use_obstacles), parameters=[{
                 "lidar_topic": LaunchConfiguration("lidar_topic"), "world_map_topic": "/map",
                 "map/map_frame_id": "map", "map/lidar_frame_id": "body", "map/base_frame_id": "body",
             }]),
        Node(package="nmpc_planner", executable="nmpc_local_planner_node", output="screen",
             parameters=[{"map_frame": "map", "base_frame": "body"}]),
        Node(package="nmpc_planner", executable="nmpc_controller_node", output="screen",
             parameters=[{"map_frame": "map", "base_frame": "body",
                          "start_in_auto": LaunchConfiguration("start_in_auto")}]),
        Node(package="go2_base_controller", executable="go2_base_controller", output="screen",
             condition=IfCondition(use_go2)),
    ])
