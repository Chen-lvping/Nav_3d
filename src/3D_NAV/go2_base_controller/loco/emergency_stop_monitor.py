#!/usr/bin/env python3
import argparse

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool
from unitree_sdk2py.core.channel import ChannelSubscriber
from unitree_sdk2py.idl.default import unitree_go_msg_dds__LowState_
from unitree_sdk2py.idl.unitree_go.msg.dds_ import LowState_ as LowStateGo2

from .dds_config import add_dds_arguments, initialize_channel
from .remote_controller import KeyMap, RemoteController


class EmergencyStopMonitor(Node):
    def __init__(self):
        super().__init__("go2_emergency_stop_monitor")
        self.remote = RemoteController()
        self.low_state = unitree_go_msg_dds__LowState_()
        self.last_button = 0
        self.publisher = self.create_publisher(Bool, "/emergency_stop", 1)
        self.subscriber = ChannelSubscriber("rt/lf/lowstate", LowStateGo2)
        self.subscriber.Init(self.low_state_callback, 10)
        self.create_timer(0.1, self.check)

    def low_state_callback(self, message):
        self.low_state = message
        try:
            self.remote.set(message.wireless_remote)
        except Exception as error:
            self.get_logger().error(f"Remote decode failed: {error}")

    def check(self):
        current = self.remote.button[KeyMap.B]
        if current == 1 and self.last_button == 0:
            message = Bool(data=True)
            self.publisher.publish(message)
            self.get_logger().error("Emergency stop requested from Go2 remote")
        self.last_button = current


def main():
    parser = argparse.ArgumentParser(description="Go2 ROS 2 emergency-stop monitor")
    add_dds_arguments(parser)
    args, ros_args = parser.parse_known_args()
    initialize_channel(args)
    rclpy.init(args=ros_args)
    node = EmergencyStopMonitor()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
