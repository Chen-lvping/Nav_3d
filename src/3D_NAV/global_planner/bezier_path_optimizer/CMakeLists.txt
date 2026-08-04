cmake_minimum_required(VERSION 3.8)
project(bezier_path_optimizer)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2 REQUIRED)

add_executable(bezier_path_optimizer_node src/bezier_path_optimizer.cpp)
ament_target_dependencies(bezier_path_optimizer_node geometry_msgs nav_msgs rclcpp tf2)

install(TARGETS bezier_path_optimizer_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
