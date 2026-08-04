cmake_minimum_required(VERSION 3.8)
project(planning_3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters)
find_package(TBB REQUIRED)
find_package(glog REQUIRED)
find_package(nanoflann REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(nav3d_ros2_compat REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(visualization_msgs REQUIRED)

add_definitions(${PCL_DEFINITIONS})
add_library(bfnn src/bfnn.cc)
target_include_directories(bfnn PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PCL_INCLUDE_DIRS})
target_link_libraries(bfnn ${PCL_LIBRARIES} Eigen3::Eigen)

add_executable(planning_3d_PRM_node src/planning_3d_PRM.cpp)
target_include_directories(planning_3d_PRM_node PRIVATE include ${PCL_INCLUDE_DIRS})
ament_target_dependencies(planning_3d_PRM_node
  geometry_msgs nav_msgs nav3d_ros2_compat pcl_conversions rclcpp sensor_msgs
  std_msgs std_srvs tf2 tf2_geometry_msgs tf2_ros visualization_msgs)
target_link_libraries(planning_3d_PRM_node
  bfnn ${PCL_LIBRARIES} Eigen3::Eigen TBB::tbb glog::glog nanoflann::nanoflann)

install(TARGETS bfnn planning_3d_PRM_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch rviz DESTINATION share/${PROJECT_NAME})
ament_package()
