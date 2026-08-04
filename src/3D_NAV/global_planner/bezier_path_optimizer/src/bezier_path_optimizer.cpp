#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <nav_msgs/msg/path.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace
{
constexpr double kEpsilon = 1e-6;

double distance_3d(const geometry_msgs::msg::Point & a,
                   const geometry_msgs::msg::Point & b)
{
  const double dx = a.x - b.x;
  const double dy = a.y - b.y;
  const double dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

geometry_msgs::msg::Point bezier_n(
  const std::vector<geometry_msgs::msg::Point> & control_points, double t)
{
  auto points = control_points;
  const auto order = static_cast<int>(points.size()) - 1;
  for (int k = 1; k <= order; ++k) {
    for (int i = 0; i <= order - k; ++i) {
      points[i].x = (1.0 - t) * points[i].x + t * points[i + 1].x;
      points[i].y = (1.0 - t) * points[i].y + t * points[i + 1].y;
      points[i].z = (1.0 - t) * points[i].z + t * points[i + 1].z;
    }
  }
  return points.front();
}
}  // namespace

class BezierOptimizer final : public rclcpp::Node
{
public:
  BezierOptimizer()
  : Node("bezier_path_optimizer")
  {
    sample_rate_ = declare_parameter<int>("sample_rate", 20);
    q1_scale_ = declare_parameter<double>("q1_scale", 0.3);
    q1_max_ratio_ = declare_parameter<double>("q1_max_ratio", 1.0);
    enable_orientation_ = declare_parameter<bool>("enable_orientation", true);
    end_strategy_ = declare_parameter<std::string>("end_strategy", "same_as_prev");
    const auto input_topic = declare_parameter<std::string>("input_topic", "/planned_path");
    const auto output_topic = declare_parameter<std::string>("output_topic", "/path_smooth");

    auto output_qos = rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local();
    path_publisher_ = create_publisher<nav_msgs::msg::Path>(output_topic, output_qos);
    path_subscription_ = create_subscription<nav_msgs::msg::Path>(
      input_topic, rclcpp::QoS(1).reliable(),
      std::bind(&BezierOptimizer::path_callback, this, std::placeholders::_1));
    RCLCPP_INFO(get_logger(), "ready: %s -> %s", input_topic.c_str(), output_topic.c_str());
  }

private:
  geometry_msgs::msg::Point compute_q1(
    const geometry_msgs::msg::Point & p4,
    const geometry_msgs::msg::Point & p5,
    const geometry_msgs::msg::Point & q2) const
  {
    geometry_msgs::msg::Point direction;
    direction.x = p5.x - p4.x;
    direction.y = p5.y - p4.y;
    direction.z = p5.z - p4.z;
    double length = std::sqrt(
      direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length < kEpsilon) {
      length = kEpsilon;
    }
    double distance = std::min(q1_scale_ * length, q1_max_ratio_ * length);

    geometry_msgs::msg::Point q1;
    q1.x = p5.x + distance * direction.x / length;
    q1.y = p5.y + distance * direction.y / length;
    q1.z = p5.z + distance * direction.z / length;
    if (distance_3d(q1, p5) >= distance_3d(q1, q2)) {
      const double extra = distance_3d(q1, p5) - distance_3d(q1, q2) + 0.01;
      q1.x += extra * direction.x / length;
      q1.y += extra * direction.y / length;
      q1.z += extra * direction.z / length;
    }
    return q1;
  }

  void path_callback(const nav_msgs::msg::Path::ConstSharedPtr message)
  {
    if (message->poses.empty()) {
      return;
    }

    std::vector<geometry_msgs::msg::Point> raw;
    raw.reserve(message->poses.size());
    for (const auto & pose : message->poses) {
      raw.push_back(pose.pose.position);
    }

    std::vector<geometry_msgs::msg::Point> samples;
    std::size_t segment_begin = 0;
    std::size_t point_count = raw.size();
    while (segment_begin + 5 < point_count) {
      std::vector<geometry_msgs::msg::Point> control_points(
        raw.begin() + segment_begin, raw.begin() + segment_begin + 6);
      for (int k = 0; k <= sample_rate_; ++k) {
        samples.push_back(bezier_n(control_points, static_cast<double>(k) / sample_rate_));
      }
      auto q1 = compute_q1(
        control_points[4], control_points[5],
        segment_begin + 6 < point_count ? raw[segment_begin + 6] : raw.back());
      raw.insert(raw.begin() + segment_begin + 6, q1);
      point_count = raw.size();
      segment_begin += 6;
    }

    if (segment_begin < point_count) {
      std::vector<geometry_msgs::msg::Point> control_points(
        raw.begin() + segment_begin, raw.end());
      if (control_points.size() == 1) {
        for (int k = 0; k <= sample_rate_; ++k) {
          samples.push_back(control_points.front());
        }
      } else {
        for (int k = 0; k <= sample_rate_; ++k) {
          samples.push_back(bezier_n(control_points, static_cast<double>(k) / sample_rate_));
        }
      }
    }

    nav_msgs::msg::Path output;
    output.header = message->header;
    output.poses.reserve(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
      geometry_msgs::msg::PoseStamped pose;
      pose.header = output.header;
      pose.pose.position = samples[i];

      if (!enable_orientation_) {
        pose.pose.orientation.w = 1.0;
        output.poses.push_back(pose);
        continue;
      }

      tf2::Vector3 direction;
      if (i + 1 < samples.size()) {
        direction = tf2::Vector3(
          samples[i + 1].x - samples[i].x,
          samples[i + 1].y - samples[i].y,
          samples[i + 1].z - samples[i].z);
      } else if (
        end_strategy_ == "keep_original" && i < message->poses.size() &&
        std::abs(message->poses[i].pose.orientation.w) > kEpsilon)
      {
        pose.pose.orientation = message->poses[i].pose.orientation;
        output.poses.push_back(pose);
        continue;
      } else if (i > 0) {
        direction = tf2::Vector3(
          samples[i].x - samples[i - 1].x,
          samples[i].y - samples[i - 1].y,
          samples[i].z - samples[i - 1].z);
      }

      if (direction.length() < kEpsilon) {
        direction = tf2::Vector3(1.0, 0.0, 0.0);
      } else {
        direction.normalize();
      }
      tf2::Vector3 z_axis(0.0, 0.0, 1.0);
      auto y_axis = z_axis.cross(direction);
      if (y_axis.length() < kEpsilon) {
        y_axis = direction.z() > 0.9 ? tf2::Vector3(0.0, -1.0, 0.0) :
          tf2::Vector3(0.0, 1.0, 0.0);
      }
      y_axis.normalize();
      const auto x_axis = y_axis.cross(z_axis);
      tf2::Matrix3x3 rotation(
        x_axis.x(), y_axis.x(), z_axis.x(),
        x_axis.y(), y_axis.y(), z_axis.y(),
        x_axis.z(), y_axis.z(), z_axis.z());
      tf2::Quaternion quaternion;
      rotation.getRotation(quaternion);
      quaternion.normalize();
      pose.pose.orientation.x = quaternion.x();
      pose.pose.orientation.y = quaternion.y();
      pose.pose.orientation.z = quaternion.z();
      pose.pose.orientation.w = quaternion.w();
      output.poses.push_back(pose);
    }

    path_publisher_->publish(output);
    RCLCPP_INFO(get_logger(), "published smooth path with %zu poses", output.poses.size());
  }

  int sample_rate_{20};
  double q1_scale_{0.3};
  double q1_max_ratio_{1.0};
  bool enable_orientation_{true};
  std::string end_strategy_;
  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr path_subscription_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_publisher_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BezierOptimizer>());
  rclcpp::shutdown();
  return 0;
}
