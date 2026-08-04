cmake_minimum_required(VERSION 3.8)
project(map_publisher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(ament_cmake REQUIRED)
find_package(Open3D REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(map_publisher_node src/map_publisher_node.cpp)
ament_target_dependencies(map_publisher_node rclcpp sensor_msgs)
target_link_libraries(map_publisher_node Open3D::Open3D)

install(TARGETS map_publisher_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME} OPTIONAL)
ament_package()
