cmake_minimum_required(VERSION 3.8)
project(obstacle_processor)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_definitions(PTHREAD_STACK_MIN=16384)

option(NAV3D_ENABLE_AVX2 "Compile obstacle_processor with AVX2" OFF)
if(NAV3D_ENABLE_AVX2)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
  if(NOT COMPILER_SUPPORTS_AVX2)
    message(FATAL_ERROR "NAV3D_ENABLE_AVX2=ON, but the compiler does not support -mavx2")
  endif()
  add_compile_options(-mavx2)
endif()

find_package(ament_cmake REQUIRED)
find_package(nav3d_ros2_compat REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(PCL REQUIRED COMPONENTS common filters kdtree)
find_package(Eigen3 REQUIRED)
find_package(OpenMP)

add_executable(obstacle_processor_node
  src/obstacle_processor_node.cpp
  src/execution_classes.cpp)
target_include_directories(obstacle_processor_node PRIVATE
  include ${PCL_INCLUDE_DIRS})
target_link_libraries(obstacle_processor_node
  ${PCL_LIBRARIES} Eigen3::Eigen)
if(OpenMP_CXX_FOUND)
  target_link_libraries(obstacle_processor_node OpenMP::OpenMP_CXX)
endif()
ament_target_dependencies(obstacle_processor_node
  nav3d_ros2_compat rclcpp std_msgs sensor_msgs geometry_msgs pcl_conversions
  tf2 tf2_ros tf2_geometry_msgs)

install(TARGETS obstacle_processor_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})
ament_package()
