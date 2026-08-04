#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/float32.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <open3d/Open3D.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include "open3d_conversions/open3d_conversions.h"

namespace
{
Eigen::Matrix4d pose_matrix(const geometry_msgs::msg::Pose & pose)
{
  Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
  Eigen::Quaterniond quaternion(
    pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);
  matrix.block<3, 3>(0, 0) = quaternion.normalized().toRotationMatrix();
  matrix.block<3, 1>(0, 3) = Eigen::Vector3d(
    pose.position.x, pose.position.y, pose.position.z);
  return matrix;
}

geometry_msgs::msg::Pose matrix_pose(const Eigen::Matrix4d & matrix)
{
  geometry_msgs::msg::Pose pose;
  pose.position.x = matrix(0, 3);
  pose.position.y = matrix(1, 3);
  pose.position.z = matrix(2, 3);
  const Eigen::Quaterniond quaternion(matrix.block<3, 3>(0, 0));
  pose.orientation.x = quaternion.x();
  pose.orientation.y = quaternion.y();
  pose.orientation.z = quaternion.z();
  pose.orientation.w = quaternion.w();
  return pose;
}
}  // namespace

class GlobalLocalization : public rclcpp::Node
{
public:
  GlobalLocalization()
  : Node("global_localization"), tf_broadcaster_(*this)
  {
    const auto map_path = declare_parameter<std::string>("map_path", "");
    map_frame_ = declare_parameter<std::string>("map_frame", "map");
    odom_frame_ = declare_parameter<std::string>("odom_frame", "odom");
    base_frame_ = declare_parameter<std::string>("base_frame", "base_link");
    odom_topic_ = declare_parameter<std::string>("odom_topic", "/Odometry_loc");
    scan_topic_ = declare_parameter<std::string>("scan_topic", "/cloud_registered");
    voxel_size_ = declare_parameter<double>("voxel_size", 0.2);
    max_correspondence_ = declare_parameter<double>("max_correspondence_distance", 1.0);
    min_fitness_ = declare_parameter<double>("min_fitness", 0.55);
    const auto frequency = declare_parameter<double>("frequency", 2.0);

    map_ = std::make_shared<open3d::geometry::PointCloud>();
    if (map_path.empty() || !open3d::io::ReadPointCloud(map_path, *map_) || map_->IsEmpty()) {
      throw std::runtime_error("map_path is empty or the point-cloud map cannot be loaded: " + map_path);
    }
    map_ = map_->VoxelDownSample(voxel_size_);
    map_->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(voxel_size_ * 2.0, 30));

    auto map_message = sensor_msgs::msg::PointCloud2();
    open3d_conversions::open3dToRos(*map_, map_message, map_frame_);
    map_message.header.stamp = now();
    map_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
      "/map", rclcpp::QoS(1).reliable().transient_local());
    map_pub_->publish(map_message);
    pose_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("/localization_3d", 10);
    confidence_pub_ = create_publisher<std_msgs::msg::Float32>("/localization_3d_confidence", 10);
    aligned_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>("/scan2map", 1);

    odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
      odom_topic_, rclcpp::SensorDataQoS(),
      [this](nav_msgs::msg::Odometry::ConstSharedPtr message) {
        std::lock_guard<std::mutex> lock(mutex_);
        odom_ = message;
      });
    scan_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
      scan_topic_, rclcpp::SensorDataQoS(),
      [this](sensor_msgs::msg::PointCloud2::ConstSharedPtr message) {
        auto scan = std::make_shared<open3d::geometry::PointCloud>();
        open3d_conversions::rosToOpen3d(message, *scan, true);
        std::lock_guard<std::mutex> lock(mutex_);
        scan_ = std::move(scan);
      });
    initial_pose_sub_ = create_subscription<geometry_msgs::msg::PoseWithCovarianceStamped>(
      "/initialpose", 10,
      [this](geometry_msgs::msg::PoseWithCovarianceStamped::ConstSharedPtr message) {
        std::lock_guard<std::mutex> lock(mutex_);
        initial_map_base_ = pose_matrix(message->pose.pose);
        initial_pose_pending_ = true;
      });
    timer_ = create_wall_timer(
      std::chrono::duration<double>(1.0 / std::max(0.1, frequency)),
      std::bind(&GlobalLocalization::localize, this));
    RCLCPP_INFO(get_logger(), "Loaded %zu map points from %s", map_->points_.size(), map_path.c_str());
  }

private:
  void localize()
  {
    nav_msgs::msg::Odometry::ConstSharedPtr odom;
    std::shared_ptr<open3d::geometry::PointCloud> scan;
    Eigen::Matrix4d initial = Eigen::Matrix4d::Identity();
    bool reset = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      odom = odom_;
      scan = scan_;
      reset = initial_pose_pending_;
      initial = initial_map_base_;
      initial_pose_pending_ = false;
    }
    if (!odom || !scan || scan->IsEmpty()) {return;}

    const Eigen::Matrix4d odom_base = pose_matrix(odom->pose.pose);
    if (reset) {map_odom_ = initial * odom_base.inverse();}
    auto source = scan->VoxelDownSample(voxel_size_);
    const auto result = open3d::pipelines::registration::RegistrationICP(
      *source, *map_, max_correspondence_, map_odom_,
      open3d::pipelines::registration::TransformationEstimationPointToPoint());
    if (result.fitness_ < min_fitness_) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000, "ICP rejected: fitness %.3f < %.3f",
        result.fitness_, min_fitness_);
      return;
    }
    map_odom_ = result.transformation_;
    const Eigen::Matrix4d map_base = map_odom_ * odom_base;
    publish(map_base, result.fitness_, odom->header.stamp, *source);
  }

  void publish(
    const Eigen::Matrix4d & map_base, double fitness,
    const builtin_interfaces::msg::Time & stamp, open3d::geometry::PointCloud scan)
  {
    geometry_msgs::msg::TransformStamped transform;
    transform.header.stamp = stamp;
    transform.header.frame_id = map_frame_;
    transform.child_frame_id = odom_frame_;
    const auto map_odom_pose = matrix_pose(map_odom_);
    transform.transform.translation.x = map_odom_pose.position.x;
    transform.transform.translation.y = map_odom_pose.position.y;
    transform.transform.translation.z = map_odom_pose.position.z;
    transform.transform.rotation = map_odom_pose.orientation;
    tf_broadcaster_.sendTransform(transform);

    geometry_msgs::msg::PoseStamped pose;
    pose.header.stamp = stamp;
    pose.header.frame_id = map_frame_;
    pose.pose = matrix_pose(map_base);
    pose_pub_->publish(pose);
    std_msgs::msg::Float32 confidence;
    confidence.data = static_cast<float>(fitness);
    confidence_pub_->publish(confidence);
    scan.Transform(map_odom_);
    sensor_msgs::msg::PointCloud2 aligned;
    open3d_conversions::open3dToRos(scan, aligned, map_frame_);
    aligned.header.stamp = stamp;
    aligned_pub_->publish(aligned);
  }

  std::mutex mutex_;
  std::shared_ptr<open3d::geometry::PointCloud> map_;
  std::shared_ptr<open3d::geometry::PointCloud> scan_;
  nav_msgs::msg::Odometry::ConstSharedPtr odom_;
  Eigen::Matrix4d map_odom_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d initial_map_base_ = Eigen::Matrix4d::Identity();
  bool initial_pose_pending_{false};
  double voxel_size_, max_correspondence_, min_fitness_;
  std::string map_frame_, odom_frame_, base_frame_, odom_topic_, scan_topic_;
  tf2_ros::TransformBroadcaster tf_broadcaster_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr map_pub_, aligned_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr confidence_pub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr scan_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr initial_pose_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<GlobalLocalization>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("global_localization"), "%s", error.what());
  }
  rclcpp::shutdown();
  return 0;
}
