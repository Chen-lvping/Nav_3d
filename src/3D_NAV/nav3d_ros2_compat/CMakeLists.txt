cmake_minimum_required(VERSION 3.8)
project(nav3d_ros2_compat)

find_package(ament_cmake REQUIRED)

set(NAV3D_ROS2_DEPENDENCIES
  builtin_interfaces
  geometry_msgs
  nav_msgs
  rclcpp
  sensor_msgs
  std_msgs
  std_srvs
  visualization_msgs)
foreach(dependency IN LISTS NAV3D_ROS2_DEPENDENCIES)
  find_package(${dependency} REQUIRED)
endforeach()

install(DIRECTORY include/ DESTINATION include)
ament_export_include_directories(include)
ament_export_dependencies(${NAV3D_ROS2_DEPENDENCIES})
ament_package()
