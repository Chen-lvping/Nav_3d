#include <open3d/Open3D.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>

#include <chrono>
#include <memory>
#include <string>

class MapPublisher final : public rclcpp::Node
{
public:
  MapPublisher()
  : Node("map_publisher_node")
  {
    map_path_ = declare_parameter<std::string>("map_path", "");
    voxel_size_ = declare_parameter<double>("voxel_size", 0.1);
    frame_id_ = declare_parameter<std::string>("map_frame_id", "map");
    publish_rate_ = declare_parameter<double>("publish_rate", 1.0);
    topic_ = declare_parameter<std::string>("map_topic", "/map");

    auto qos = rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local();
    publisher_ = create_publisher<sensor_msgs::msg::PointCloud2>(topic_, qos);
    if (!load_map()) {
      throw std::runtime_error("failed to load point-cloud map");
    }
    publish_map();
    if (publish_rate_ > 0.0) {
      timer_ = create_wall_timer(
        std::chrono::duration<double>(1.0 / publish_rate_),
        std::bind(&MapPublisher::publish_map, this));
    }
  }

private:
  bool load_map()
  {
    if (map_path_.empty()) {
      RCLCPP_ERROR(get_logger(), "map_path is empty");
      return false;
    }
    auto cloud = std::make_shared<open3d::geometry::PointCloud>();
    if (!open3d::io::ReadPointCloud(map_path_, *cloud) || cloud->IsEmpty()) {
      RCLCPP_ERROR(get_logger(), "failed to read point cloud: %s", map_path_.c_str());
      return false;
    }
    map_ = voxel_size_ > 0.0 ? cloud->VoxelDownSample(voxel_size_) : cloud;
    RCLCPP_INFO(get_logger(), "loaded %zu map points", map_->points_.size());
    return true;
  }

  void publish_map()
  {
    sensor_msgs::msg::PointCloud2 message;
    sensor_msgs::PointCloud2Modifier modifier(message);
    const bool has_colors = map_->HasColors();
    if (has_colors) {
      modifier.setPointCloud2FieldsByString(2, "xyz", "rgb");
    } else {
      modifier.setPointCloud2FieldsByString(1, "xyz");
    }
    modifier.resize(map_->points_.size());
    sensor_msgs::PointCloud2Iterator<float> x(message, "x");
    sensor_msgs::PointCloud2Iterator<float> y(message, "y");
    sensor_msgs::PointCloud2Iterator<float> z(message, "z");
    if (has_colors) {
      sensor_msgs::PointCloud2Iterator<uint8_t> red(message, "r");
      sensor_msgs::PointCloud2Iterator<uint8_t> green(message, "g");
      sensor_msgs::PointCloud2Iterator<uint8_t> blue(message, "b");
      for (std::size_t i = 0; i < map_->points_.size(); ++i, ++x, ++y, ++z, ++red, ++green, ++blue) {
        *x = static_cast<float>(map_->points_[i].x());
        *y = static_cast<float>(map_->points_[i].y());
        *z = static_cast<float>(map_->points_[i].z());
        *red = static_cast<uint8_t>(255.0 * map_->colors_[i].x());
        *green = static_cast<uint8_t>(255.0 * map_->colors_[i].y());
        *blue = static_cast<uint8_t>(255.0 * map_->colors_[i].z());
      }
    } else {
      for (const auto & point : map_->points_) {
        *x = static_cast<float>(point.x());
        *y = static_cast<float>(point.y());
        *z = static_cast<float>(point.z());
        ++x;
        ++y;
        ++z;
      }
    }
    message.header.frame_id = frame_id_;
    message.header.stamp = now();
    publisher_->publish(message);
  }

  std::string map_path_;
  std::string frame_id_;
  std::string topic_;
  double voxel_size_{0.1};
  double publish_rate_{1.0};
  std::shared_ptr<open3d::geometry::PointCloud> map_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<MapPublisher>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("map_publisher"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
