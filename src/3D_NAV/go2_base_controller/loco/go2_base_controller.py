#!/usr/bin/env python3
import argparse
import time
from threading import Lock

import rclpy
from geometry_msgs.msg import Twist
from rclpy.node import Node
from std_msgs.msg import Bool
from unitree_sdk2py.go2.sport.sport_client import SportClient

from .dds_config import add_dds_arguments, initialize_channel, resolved_interface


class Go2BaseController(Node):
    def __init__(self):
        super().__init__("go2_base_controller")
        self.client = SportClient()
        self.client.SetTimeout(10.0)
        self.client.Init()
        self.lock = Lock()
        self.last_cmd = Twist()
        self.last_cmd_time = self.get_clock().now()
        self.stopped = False
        self.timeout = float(self.declare_parameter("command_timeout", 1.0).value)
        self.create_subscription(Twist, "/cmd_vel", self.cmd_callback, 10)
        self.create_subscription(Bool, "/emergency_stop", self.stop_callback, 10)
        self.create_timer(0.02, self.control_loop)
        self.get_logger().info("Unitree Go2 controller ready (native ROS 2, 50 Hz)")

    def cmd_callback(self, message):
        with self.lock:
            self.last_cmd = message
            self.last_cmd_time = self.get_clock().now()

    def stop_callback(self, message):
        if message.data:
            self.stopped = True
            self.client.StopMove()
            self.get_logger().error("Emergency stop activated")

    def control_loop(self):
        with self.lock:
            age = (self.get_clock().now() - self.last_cmd_time).nanoseconds * 1e-9
            command = self.last_cmd
        if self.stopped or age > self.timeout:
            vx = vy = yaw = 0.0
        else:
            vx, vy, yaw = command.linear.x, command.linear.y, command.angular.z
        try:
            self.client.Move(vx, vy, yaw)
        except Exception as error:
            self.get_logger().error(f"Go2 Move failed: {error}")

    def halt(self):
        for _ in range(10):
            self.client.Move(0.0, 0.0, 0.0)
            time.sleep(0.02)
        self.client.StopMove()


def main():
    parser = argparse.ArgumentParser(description="ROS 2 cmd_vel controller for Unitree Go2")
    add_dds_arguments(parser)
    args, ros_args = parser.parse_known_args()
    print(f"Initializing Go2 DDS: iface={resolved_interface(args)}, peer={args.peer or 'none'}")
    initialize_channel(args)
    rclpy.init(args=ros_args)
    node = Go2BaseController()
    try:
        rclpy.spin(node)
    finally:
        node.halt()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
