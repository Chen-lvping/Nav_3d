cmake_minimum_required(VERSION 3.8)
project(map_process)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PCL 1.10 REQUIRED)
find_package(rclcpp REQUIRED)

add_library(${PROJECT_NAME}
  src/trajectory_processor.cpp
  src/region_growing.cpp
  src/traversable_extractor.cpp
  src/utils.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PCL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PCL_LIBRARIES} Eigen3::Eigen)
add_definitions(${PCL_DEFINITIONS})

add_executable(map_process_node src/main.cpp)
target_link_libraries(map_process_node ${PROJECT_NAME})
ament_target_dependencies(map_process_node rclcpp)

install(TARGETS ${PROJECT_NAME} map_process_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(Eigen3 PCL)
ament_package()
