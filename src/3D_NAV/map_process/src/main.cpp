#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include "map_process/traversable_extractor.h"
#include "map_process/utils.h"

template<typename T>
void parameter(const rclcpp::Node::SharedPtr& node, const std::string& name,
               T& value, const T& default_value) {
    value = node->declare_parameter<T>(name, default_value);
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("map_process_node");
    
    std::cout << "=== Map Process Node Started ===" << std::endl;
    
    // Parameters
    std::string point_cloud_file;
    std::string trajectory_file;
    std::string output_file;
    
    // Get parameters
    parameter<std::string>(node, "point_cloud_file", point_cloud_file, 
                                  "data/scans.pcd");
    parameter<std::string>(node, "trajectory_file", trajectory_file, 
                                  "data/fe_lio_pose.txt");
    parameter<std::string>(node, "output_file", output_file, 
                                  "output/traversable_areas.pcd");

    size_t output_sep = output_file.find_last_of("/");
    std::string output_dir = output_sep == std::string::npos ? "" : output_file.substr(0, output_sep);
    if (!output_dir.empty()) {
        std::filesystem::create_directories(output_dir);
    }
    
    // Create extraction parameters
    map_process::ExtractionParams params;
    
    // Load parameters from ROS parameter server
    parameter(node, "trajectory_sample_interval", params.trajectory_sample_interval, 1.0);
    parameter(node, "seed_search_radius", params.seed_search_radius, 0.5);
    parameter(node, "seed_height_offset", params.seed_height_offset, 1.0);
    parameter(node, "merge_distance_threshold", params.merge_distance_threshold, 0.2);
    parameter(node, "min_region_size", params.min_region_size, 100);
    parameter(node, "remove_outliers", params.remove_outliers, true);
    
    // Region growing parameters
    parameter(node, "normal_angle_threshold", params.region_params.normal_angle_threshold, 30.0);
    parameter(node, "curvature_threshold", params.region_params.curvature_threshold, 1.0);
    parameter(node, "height_threshold", params.region_params.height_threshold, 0.1);
    parameter(node, "search_radius", params.region_params.search_radius, 0.3);
    parameter(node, "min_cluster_size", params.region_params.min_cluster_size, 50);
    parameter(node, "max_cluster_size", params.region_params.max_cluster_size, 1000000);
    parameter(node, "normal_search_radius", params.region_params.normal_search_radius, 0.2);
    
    // Ground surface constraint parameters
    parameter(node, "max_ground_normal_angle", params.max_ground_normal_angle, 25.0);
    parameter(node, "enable_height_filter", params.enable_height_filter, true);
    parameter(node, "seed_height_diff_threshold", params.seed_height_diff_threshold, 0.1);
    parameter(node, "seed_xy_search_radius", params.seed_xy_search_radius, 5.0);
    
    // Point cloud downsampling parameters
    parameter(node, "enable_voxel_downsampling", params.enable_voxel_downsampling, true);
    parameter(node, "voxel_size", params.voxel_size, 0.05);
    parameter(node, "downsampling_threshold", params.downsampling_threshold, 1000000);
    
    // Trajectory format parameter
    parameter(node, "trajectory_format", params.trajectory_format, 1);
    parameter(node, "require_seed_containment", params.require_seed_containment, true);

    parameter<std::string>(node, "debug_seed_points_file", params.debug_seed_points_file,
                                  output_dir.empty() ? "seed_points.pcd" : output_dir + "/seed_points.pcd");
    parameter<std::string>(node, "debug_ground_candidates_file", params.debug_ground_candidates_file,
                                  output_dir.empty() ? "ground_candidates.pcd" : output_dir + "/ground_candidates.pcd");
    
    // Print parameters
    std::cout << "\\nParameters:" << std::endl;
    std::cout << "  Point cloud file: " << point_cloud_file << std::endl;
    std::cout << "  Trajectory file: " << trajectory_file << std::endl;
    std::cout << "  Output file: " << output_file << std::endl;
    std::cout << "  Trajectory sample interval: " << params.trajectory_sample_interval << " m" << std::endl;
    std::cout << "  Seed search radius: " << params.seed_search_radius << " m" << std::endl;
    std::cout << "  Seed height offset: " << params.seed_height_offset << " m" << std::endl;
    std::cout << "  Normal angle threshold: " << params.region_params.normal_angle_threshold << " deg" << std::endl;
    std::cout << "  Height threshold: " << params.region_params.height_threshold << " m" << std::endl;
    std::cout << "  Search radius: " << params.region_params.search_radius << " m" << std::endl;
    std::cout << "  Min cluster size: " << params.region_params.min_cluster_size << std::endl;
    std::cout << "  Max ground normal angle: " << params.max_ground_normal_angle << " deg" << std::endl;
    std::cout << "  Height filter: " << (params.enable_height_filter ? "enabled" : "disabled") << std::endl;
    std::cout << "  Seed height diff threshold: " << params.seed_height_diff_threshold << " m" << std::endl;
    std::cout << "  Seed XY search radius: " << params.seed_xy_search_radius << " m" << std::endl;
    std::cout << "  Voxel downsampling: " << (params.enable_voxel_downsampling ? "enabled" : "disabled") << std::endl;
    std::cout << "  Voxel size: " << params.voxel_size << " m" << std::endl;
    std::cout << "  Downsampling threshold: " << params.downsampling_threshold << " points" << std::endl;
    std::cout << "  Trajectory format: " << params.trajectory_format << std::endl;
    std::cout << "  Require seed containment: " << (params.require_seed_containment ? "true" : "false") << std::endl;
    std::cout << "  Debug seed points file: " << params.debug_seed_points_file << std::endl;
    std::cout << "  Debug ground candidates file: " << params.debug_ground_candidates_file << std::endl;
    
    // Create traversable extractor
    map_process::TraversableExtractor extractor;
    extractor.setParameters(params);
    
    // Load data
    std::cout << "\\n=== Loading Data ===" << std::endl;
    
    if (!extractor.loadPointCloud(point_cloud_file)) {
        RCLCPP_ERROR(node->get_logger(), "Failed to load point cloud from: %s", point_cloud_file.c_str());
        return -1;
    }
    
    if (!extractor.loadTrajectory(trajectory_file)) {
        RCLCPP_ERROR(node->get_logger(), "Failed to load trajectory from: %s", trajectory_file.c_str());
        return -1;
    }
    
    // Extract traversable areas
    std::cout << "\\n=== Processing ===" << std::endl;
    
    const auto start_time = std::chrono::steady_clock::now();
    
    if (!extractor.extractTraversableAreas()) {
        RCLCPP_ERROR(node->get_logger(), "Failed to extract traversable areas");
        return -1;
    }
    
    const auto processing_time = std::chrono::steady_clock::now() - start_time;
    
    // Get statistics
    int total_points, traversable_points, num_regions;
    extractor.getStatistics(total_points, traversable_points, num_regions);
    
    std::cout << "\\n=== Results ===" << std::endl;
    std::cout << "  Processing time: " << std::chrono::duration<double>(processing_time).count() << " seconds" << std::endl;
    std::cout << "  Total input points: " << total_points << std::endl;
    std::cout << "  Traversable points: " << traversable_points << std::endl;
    const double coverage = total_points > 0
        ? 100.0 * static_cast<double>(traversable_points) / total_points
        : 0.0;
    std::cout << "  Coverage: " << coverage << "%" << std::endl;
    std::cout << "  Number of regions: " << num_regions << std::endl;
    
    // Save results
    std::cout << "\\n=== Saving Results ===" << std::endl;
    
    // Create output directory if it doesn't exist
    if (!output_dir.empty()) {
        std::filesystem::create_directories(output_dir);
    }
    
    if (!extractor.saveTraversableAreas(output_file)) {
        RCLCPP_ERROR(node->get_logger(), "Failed to save traversable areas to: %s", output_file.c_str());
        return -1;
    }
    
    // Save individual regions (optional)
    bool save_individual_regions = false;
    parameter(node, "save_individual_regions", save_individual_regions, false);
    
    if (save_individual_regions) {
        std::cout << "Saving individual regions..." << std::endl;
        auto regions = extractor.getRegions();
        
        for (size_t i = 0; i < regions.size(); ++i) {
            const std::string region_prefix = output_dir.empty()
                ? "region_"
                : output_dir + "/region_";
            std::string region_file = region_prefix + std::to_string(i) + ".pcd";
            map_process::Utils::savePointCloud(regions[i], region_file);
        }
    }
    
    std::cout << "\\n=== Processing Complete ===" << std::endl;
    std::cout << "Results saved to: " << output_file << std::endl;
    
    rclcpp::shutdown();
    return 0;
}
