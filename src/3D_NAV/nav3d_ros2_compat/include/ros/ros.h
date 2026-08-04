#pragma once

#include <builtin_interfaces/msg/time.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <nav_msgs/msg/path.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <visualization_msgs/msg/marker.hpp>

#include <algorithm>
#include <chrono>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

namespace geometry_msgs
{
using Point = msg::Point;
using PointStamped = msg::PointStamped;
using Pose = msg::Pose;
using PoseStamped = msg::PoseStamped;
using PoseWithCovarianceStamped = msg::PoseWithCovarianceStamped;
using Quaternion = msg::Quaternion;
using TransformStamped = msg::TransformStamped;
using Twist = msg::Twist;
using Vector3 = msg::Vector3;
}
namespace nav_msgs {using Odometry = msg::Odometry; using Path = msg::Path;}
namespace sensor_msgs
{
using Imu = msg::Imu;
using ImuConstPtr = msg::Imu::ConstSharedPtr;
using PointCloud2 = msg::PointCloud2;
using PointCloud2ConstPtr = msg::PointCloud2::ConstSharedPtr;
}
namespace std_msgs
{
using Float32MultiArray = msg::Float32MultiArray;
using Float32 = msg::Float32;
using Int32 = msg::Int32;
using String = msg::String;
using UInt8 = msg::UInt8;
}
namespace visualization_msgs {using Marker = msg::Marker;}
namespace std_srvs {using Trigger = srv::Trigger;}

namespace ros
{
inline rclcpp::Node::SharedPtr & global_node()
{
  static rclcpp::Node::SharedPtr node;
  return node;
}

inline rclcpp::Clock & logging_clock()
{
  static rclcpp::Clock clock(RCL_STEADY_TIME);
  return clock;
}

class Duration
{
public:
  explicit Duration(double seconds = 0.0) : seconds_(seconds) {}
  double toSec() const {return seconds_;}
  void sleep() const {std::this_thread::sleep_for(std::chrono::duration<double>(seconds_));}
  rclcpp::Duration rcl_duration() const {return rclcpp::Duration::from_seconds(seconds_);}
private:
  double seconds_;
};

class Time
{
public:
  explicit Time(double seconds = 0.0) : nanoseconds_(static_cast<int64_t>(seconds * 1e9)) {}
  explicit Time(const builtin_interfaces::msg::Time & stamp)
  : nanoseconds_(rclcpp::Time(stamp).nanoseconds()) {}
  static Time now()
  {
    return global_node() ? Time(global_node()->now().seconds()) : Time(0.0);
  }
  Time & fromSec(double seconds)
  {
    nanoseconds_ = static_cast<int64_t>(seconds * 1e9);
    return *this;
  }
  double toSec() const {return static_cast<double>(nanoseconds_) / 1e9;}
  int64_t toNSec() const {return nanoseconds_;}
  operator builtin_interfaces::msg::Time() const
  {
    return rclcpp::Time(nanoseconds_);
  }
  rclcpp::Time rcl_time() const {return rclcpp::Time(nanoseconds_);}
private:
  int64_t nanoseconds_;
};

inline Duration operator-(const Time & lhs, const Time & rhs)
{
  return Duration(lhs.toSec() - rhs.toSec());
}

struct TimerEvent {};

class Publisher
{
  struct Base {virtual ~Base() = default;};
  template<typename MessageT>
  struct Holder final : Base
  {
    explicit Holder(typename rclcpp::Publisher<MessageT>::SharedPtr value) : publisher(std::move(value)) {}
    typename rclcpp::Publisher<MessageT>::SharedPtr publisher;
  };
public:
  Publisher() = default;
  template<typename MessageT>
  explicit Publisher(typename rclcpp::Publisher<MessageT>::SharedPtr publisher)
  : holder_(std::make_shared<Holder<MessageT>>(std::move(publisher))) {}

  template<typename MessageT>
  void publish(const MessageT & message) const
  {
    auto typed = std::dynamic_pointer_cast<Holder<MessageT>>(holder_);
    if (!typed) {throw std::runtime_error("publisher message type mismatch");}
    typed->publisher->publish(message);
  }
  explicit operator bool() const {return static_cast<bool>(holder_);}
private:
  std::shared_ptr<Base> holder_;
};

class Subscriber
{
public:
  Subscriber() = default;
  explicit Subscriber(rclcpp::SubscriptionBase::SharedPtr subscription)
  : subscription_(std::move(subscription)) {}
private:
  rclcpp::SubscriptionBase::SharedPtr subscription_;
};

class ServiceServer
{
public:
  ServiceServer() = default;
  explicit ServiceServer(rclcpp::ServiceBase::SharedPtr service) : service_(std::move(service)) {}
private:
  rclcpp::ServiceBase::SharedPtr service_;
};

class Timer
{
public:
  Timer() = default;
  explicit Timer(rclcpp::TimerBase::SharedPtr timer) : timer_(std::move(timer)) {}
  bool isValid() const {return static_cast<bool>(timer_);}
  void stop() {if (timer_) {timer_->cancel();}}
private:
  rclcpp::TimerBase::SharedPtr timer_;
};

class TransportHints
{
public:
  TransportHints & tcpNoDelay(bool = true) {return *this;}
};

class NodeHandle
{
public:
  explicit NodeHandle(const std::string & = "") : node_(global_node())
  {
    if (!node_) {throw std::runtime_error("ros::init must be called before NodeHandle construction");}
  }

  template<typename T>
  void param(const std::string & name, T & output, const T & default_value)
  {
    if constexpr (std::is_same_v<T, float>) {
      output = static_cast<float>(node_->declare_parameter<double>(name, default_value));
    } else if constexpr (std::is_same_v<T, int>) {
      output = static_cast<int>(node_->declare_parameter<int64_t>(name, default_value));
    } else {
      output = node_->declare_parameter<T>(name, default_value);
    }
  }

  template<typename T>
  bool getParam(const std::string & name, T & output)
  {
    if constexpr (std::is_same_v<T, float>) {
      if (!node_->has_parameter(name)) {node_->declare_parameter<double>(name, output);}
      double value{};
      if (!node_->get_parameter(name, value)) {return false;}
      output = static_cast<float>(value);
      return true;
    } else if constexpr (std::is_same_v<T, int>) {
      if (!node_->has_parameter(name)) {node_->declare_parameter<int64_t>(name, output);}
      int64_t value{};
      if (!node_->get_parameter(name, value)) {return false;}
      output = static_cast<int>(value);
      return true;
    } else {
      if (!node_->has_parameter(name)) {node_->declare_parameter<T>(name, output);}
      return node_->get_parameter(name, output);
    }
  }

  template<typename MessageT>
  Publisher advertise(const std::string & topic, std::size_t depth, bool latch = false)
  {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(std::max<std::size_t>(1, depth))).reliable();
    if (latch) {qos.transient_local();}
    return Publisher(node_->create_publisher<MessageT>(topic, qos));
  }

  template<typename MessageT, typename ObjectT>
  Subscriber subscribe(
    const std::string & topic, std::size_t depth,
    void (ObjectT::* callback)(const typename MessageT::ConstSharedPtr &), ObjectT * object)
  {
    auto subscription = node_->create_subscription<MessageT>(
      topic, rclcpp::QoS(std::max<std::size_t>(1, depth)),
      [object, callback](typename MessageT::ConstSharedPtr message) {(object->*callback)(message);});
    return Subscriber(subscription);
  }

  template<typename MessageT>
  Subscriber subscribe(
    const std::string & topic, std::size_t depth,
    void (* callback)(const typename MessageT::ConstSharedPtr &))
  {
    auto subscription = node_->create_subscription<MessageT>(
      topic, rclcpp::QoS(std::max<std::size_t>(1, depth)), callback);
    return Subscriber(subscription);
  }

  template<typename MessageT>
  Subscriber subscribe(
    const std::string & topic, std::size_t depth,
    void (* callback)(const MessageT &))
  {
    auto subscription = node_->create_subscription<MessageT>(
      topic, rclcpp::QoS(std::max<std::size_t>(1, depth)),
      [callback](typename MessageT::ConstSharedPtr message) {callback(*message);});
    return Subscriber(subscription);
  }

  template<typename MessageT, typename CallbackT>
  Subscriber subscribe(
    const std::string & topic, std::size_t depth, CallbackT callback, const TransportHints &)
  {
    auto subscription = node_->create_subscription<MessageT>(
      topic, rclcpp::SensorDataQoS().keep_last(std::max<std::size_t>(1, depth)), callback);
    return Subscriber(subscription);
  }

  template<typename ObjectT>
  ServiceServer advertiseService(
    const std::string & service_name,
    bool (ObjectT::* callback)(std_srvs::Trigger::Request &, std_srvs::Trigger::Response &),
    ObjectT * object)
  {
    auto service = node_->create_service<std_srvs::srv::Trigger>(
      service_name,
      [object, callback](
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response)
      {
        (object->*callback)(*request, *response);
      });
    return ServiceServer(service);
  }

  template<typename ObjectT>
  Timer createTimer(Duration period, void (ObjectT::* callback)(const TimerEvent &), ObjectT * object)
  {
    auto timer = node_->create_wall_timer(
      std::chrono::duration<double>(period.toSec()),
      [object, callback]() {(object->*callback)(TimerEvent{});});
    return Timer(timer);
  }

  rclcpp::Node::SharedPtr node() const {return node_;}
private:
  rclcpp::Node::SharedPtr node_;
};

class Rate
{
public:
  explicit Rate(double hz) : rate_(hz) {}
  void sleep() {rate_.sleep();}
private:
  rclcpp::Rate rate_;
};

inline void init(int argc, char ** argv, const std::string & name)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(false).automatically_declare_parameters_from_overrides(false);
  global_node() = std::make_shared<rclcpp::Node>(name, options);
}
inline bool ok() {return rclcpp::ok();}
inline void shutdown() {rclcpp::shutdown();}
inline void spin() {rclcpp::spin(global_node());}
inline void spinOnce() {rclcpp::spin_some(global_node());}
}  // namespace ros

#define ROS_DEBUG(...) RCLCPP_DEBUG(ros::global_node()->get_logger(), __VA_ARGS__)
#define ROS_INFO(...) RCLCPP_INFO(ros::global_node()->get_logger(), __VA_ARGS__)
#define ROS_WARN(...) RCLCPP_WARN(ros::global_node()->get_logger(), __VA_ARGS__)
#define ROS_ERROR(...) RCLCPP_ERROR(ros::global_node()->get_logger(), __VA_ARGS__)
#define ROS_FATAL(...) RCLCPP_FATAL(ros::global_node()->get_logger(), __VA_ARGS__)
#define ROS_DEBUG_STREAM(args) RCLCPP_DEBUG_STREAM(ros::global_node()->get_logger(), args)
#define ROS_INFO_STREAM(args) RCLCPP_INFO_STREAM(ros::global_node()->get_logger(), args)
#define ROS_WARN_STREAM(args) RCLCPP_WARN_STREAM(ros::global_node()->get_logger(), args)
#define ROS_ERROR_STREAM(args) RCLCPP_ERROR_STREAM(ros::global_node()->get_logger(), args)
#define ROS_FATAL_STREAM(args) RCLCPP_FATAL_STREAM(ros::global_node()->get_logger(), args)
#define ROS_ASSERT(condition) assert(condition)
#define ROS_INFO_THROTTLE(period, ...) \
  RCLCPP_INFO_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), __VA_ARGS__)
#define ROS_DEBUG_THROTTLE(period, ...) \
  RCLCPP_DEBUG_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), __VA_ARGS__)
#define ROS_WARN_THROTTLE(period, ...) \
  RCLCPP_WARN_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), __VA_ARGS__)
#define ROS_ERROR_THROTTLE(period, ...) \
  RCLCPP_ERROR_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), __VA_ARGS__)
#define ROS_INFO_STREAM_THROTTLE(period, args) \
  RCLCPP_INFO_STREAM_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), args)
#define ROS_WARN_STREAM_THROTTLE(period, args) \
  RCLCPP_WARN_STREAM_THROTTLE(ros::global_node()->get_logger(), ros::logging_clock(), \
    static_cast<int64_t>((period) * 1000.0), args)
