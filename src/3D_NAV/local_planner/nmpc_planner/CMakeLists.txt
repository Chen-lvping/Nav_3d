cmake_minimum_required(VERSION 3.8)
project(nmpc_planner)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_definitions(PTHREAD_STACK_MIN=16384)

find_package(ament_cmake REQUIRED)
find_package(nav3d_ros2_compat REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(Eigen3 REQUIRED)

find_path(CASADI_INCLUDE_DIR casadi/casadi.hpp
  HINTS "$ENV{CASADI_ROOT}/include"
  PATHS /usr/local/include /opt/casadi/include)
find_library(CASADI_LIBRARY casadi
  HINTS "$ENV{CASADI_LIB_PATH}" "$ENV{CASADI_ROOT}/lib"
  PATHS /usr/local/lib /opt/casadi/lib)
if(NOT CASADI_INCLUDE_DIR OR NOT CASADI_LIBRARY)
  message(FATAL_ERROR "CasADi C++ library not found. Set CASADI_ROOT or CASADI_LIB_PATH.")
endif()

add_library(${PROJECT_NAME} SHARED
  src/mpc_solver.cpp
  src/local_planner.cpp
  src/controller_node.cpp
  src/keyboard_input.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${CASADI_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen ${CASADI_LIBRARY})
ament_target_dependencies(${PROJECT_NAME}
  nav3d_ros2_compat rclcpp std_msgs geometry_msgs nav_msgs tf2 tf2_ros tf2_geometry_msgs)

add_executable(nmpc_controller_node src/controller_main.cpp)
add_executable(nmpc_local_planner_node src/local_planner_main.cpp)
foreach(target nmpc_controller_node nmpc_local_planner_node)
  target_link_libraries(${target} ${PROJECT_NAME})
  ament_target_dependencies(${target} rclcpp)
endforeach()

install(TARGETS ${PROJECT_NAME} nmpc_controller_node nmpc_local_planner_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(nav3d_ros2_compat rclcpp std_msgs geometry_msgs nav_msgs tf2 tf2_ros tf2_geometry_msgs)
ament_package()
