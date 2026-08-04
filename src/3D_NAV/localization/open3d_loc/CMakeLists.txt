cmake_minimum_required(VERSION 3.8)
project(open3d_loc)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
add_compile_options(-O3 -Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(nav3d_ros2_compat REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(Open3D REQUIRED)
find_package(Eigen3 REQUIRED)

add_library(open3d_registration src/open3d_registration/open3d_registration.cpp)
target_include_directories(open3d_registration PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(open3d_registration PUBLIC Open3D::Open3D Eigen3::Eigen)

add_library(open3d_conversions src/open3d_conversions/open3d_conversions.cpp)
target_include_directories(open3d_conversions PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(open3d_conversions PUBLIC Open3D::Open3D Eigen3::Eigen)
ament_target_dependencies(open3d_conversions nav3d_ros2_compat sensor_msgs)

add_executable(global_localization_node src/global_localization.cpp)
target_link_libraries(global_localization_node
  open3d_registration open3d_conversions Open3D::Open3D Eigen3::Eigen)
ament_target_dependencies(global_localization_node
  rclcpp geometry_msgs nav_msgs sensor_msgs std_msgs tf2_ros)

install(TARGETS open3d_registration open3d_conversions global_localization_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch config rviz_cfg DESTINATION share/${PROJECT_NAME})
ament_package()
