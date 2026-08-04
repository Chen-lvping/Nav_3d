#!/usr/bin/env python3
import math
import os

import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node


class MappingTrajectoryRecorder(Node):
    def __init__(self):
        super().__init__("mapping_trajectory_recorder")
        self.odom_topic = self.declare_parameter("odom_topic", "/Odometry_loc").value
        self.output_file = self.declare_parameter("output_file", "").value
        self.min_interval = float(self.declare_parameter("min_interval", 0.1).value)
        self.min_distance = float(self.declare_parameter("min_distance", 0.02).value)
        append = bool(self.declare_parameter("append", False).value)
        if not self.output_file:
            raise RuntimeError("output_file parameter must be set")
        os.makedirs(os.path.dirname(os.path.abspath(self.output_file)), exist_ok=True)
        self.file_handle = open(self.output_file, "a" if append else "w", buffering=1)
        self.last_stamp = None
        self.last_position = None
        self.count = 0
        self.create_subscription(Odometry, self.odom_topic, self.odom_callback, 100)
        self.get_logger().info(f"Recording {self.odom_topic} to {self.output_file}")

    @staticmethod
    def stamp_seconds(stamp):
        return stamp.sec + stamp.nanosec * 1e-9

    def odom_callback(self, msg):
        stamp_sec = self.stamp_seconds(msg.header.stamp) or self.get_clock().now().nanoseconds * 1e-9
        p, q = msg.pose.pose.position, msg.pose.pose.orientation
        if self.last_stamp is not None:
            distance = math.dist((p.x, p.y, p.z), self.last_position)
            if stamp_sec - self.last_stamp < self.min_interval or distance < self.min_distance:
                return
        self.file_handle.write(
            f"{stamp_sec:.9f} {p.x:.9f} {p.y:.9f} {p.z:.9f} "
            f"{q.x:.9f} {q.y:.9f} {q.z:.9f} {q.w:.9f}\n")
        self.last_stamp = stamp_sec
        self.last_position = (p.x, p.y, p.z)
        self.count += 1

    def destroy_node(self):
        if not self.file_handle.closed:
            self.file_handle.close()
        self.get_logger().info(f"Saved {self.count} poses to {self.output_file}")
        return super().destroy_node()


def main():
    rclpy.init()
    node = MappingTrajectoryRecorder()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
