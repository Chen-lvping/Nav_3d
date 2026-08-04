cmake_minimum_required(VERSION 3.8)
project(fast_lio)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
add_compile_options(-O3 -Wall -Wextra)
add_compile_definitions(ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

include(ProcessorCount)
ProcessorCount(NAV3D_CPU_COUNT)
if(NAV3D_CPU_COUNT GREATER 4)
  add_compile_definitions(MP_EN MP_PROC_NUM=3)
elseif(NAV3D_CPU_COUNT GREATER 3)
  add_compile_definitions(MP_EN MP_PROC_NUM=2)
else()
  add_compile_definitions(MP_PROC_NUM=1)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(nav3d_ros2_compat REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(livox_ros_driver2 REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED)
find_package(OpenMP)
find_package(Python3 COMPONENTS Development REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Pose6D.msg"
  DEPENDENCIES geometry_msgs)

add_executable(fastlio_mapping
  src/laserMapping.cpp
  include/ikd-Tree/ikd_Tree.cpp
  src/preprocess.cpp)
target_include_directories(fastlio_mapping PRIVATE
  include src ${PCL_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})
target_link_libraries(fastlio_mapping
  ${PCL_LIBRARIES} Eigen3::Eigen Python3::Python)
if(OpenMP_CXX_FOUND)
  target_link_libraries(fastlio_mapping OpenMP::OpenMP_CXX)
endif()
ament_target_dependencies(fastlio_mapping
  nav3d_ros2_compat rclcpp geometry_msgs nav_msgs sensor_msgs std_msgs
  pcl_conversions tf2 tf2_ros livox_ros_driver2)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(fastlio_mapping "${cpp_typesupport_target}")

add_executable(m20_lidar_fusion_node src/m20_lidar_fusion.cpp)
ament_target_dependencies(m20_lidar_fusion_node
  nav3d_ros2_compat rclcpp sensor_msgs)

install(TARGETS fastlio_mapping m20_lidar_fusion_node
  DESTINATION lib/${PROJECT_NAME})
install(PROGRAMS
  scripts/mapping_trajectory_recorder.py
  scripts/level_fastlio_map.py
  scripts/m20_sensor_audit.py
  scripts/m20_static_stability.py
  scripts/record_m20_static.sh
  DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch config rviz_cfg DESTINATION share/${PROJECT_NAME})

ament_export_dependencies(rosidl_default_runtime)
ament_package()
