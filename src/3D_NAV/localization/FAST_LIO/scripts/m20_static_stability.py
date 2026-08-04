#!/usr/bin/env python3
import argparse
import math
import time

import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node


def euler(q):
    roll = math.atan2(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y))
    pitch = math.asin(max(-1.0, min(1.0, 2 * (q.w * q.y - q.z * q.x))))
    yaw = math.atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z))
    return roll, pitch, yaw


class Collector(Node):
    def __init__(self):
        super().__init__("m20_static_stability")
        self.poses = []
        self.create_subscription(Odometry, "/Odometry_loc", self.callback, 1000)

    def callback(self, msg):
        p = msg.pose.pose.position
        self.poses.append((p.x, p.y, p.z, *euler(msg.pose.pose.orientation)))


def collect(node, duration):
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline and rclpy.ok():
        rclpy.spin_once(node, timeout_sec=0.02)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--duration", type=float, default=120.0)
    parser.add_argument("--warmup", type=float, default=15.0)
    parser.add_argument("--max-position-drift", type=float, default=0.05)
    parser.add_argument("--max-attitude-drift-deg", type=float, default=0.5)
    args, ros_args = parser.parse_known_args()
    rclpy.init(args=ros_args)
    node = Collector()
    print(f"Warmup for {args.warmup:.1f}s; keep M20 stationary.")
    collect(node, args.warmup)
    node.poses.clear()
    collect(node, args.duration)
    poses = node.poses
    node.destroy_node()
    rclpy.shutdown()
    if len(poses) < 2:
        raise RuntimeError("Insufficient /Odometry_loc samples")
    ref = poses[0]
    pos = [math.dist(p[:3], ref[:3]) for p in poses]
    axes = [[abs(math.degrees(math.atan2(math.sin(p[i] - ref[i]), math.cos(p[i] - ref[i]))))
             for p in poses] for i in range(3, 6)]
    att = [max(values) for values in zip(*axes)]
    print(f"samples={len(poses)} final_position_drift={pos[-1]:.4f}m max_position_drift={max(pos):.4f}m")
    print(f"final_attitude_drift={att[-1]:.3f}deg max_attitude_drift={max(att):.3f}deg")
    passed = max(pos) <= args.max_position_drift and max(att) <= args.max_attitude_drift_deg
    print("PASS" if passed else "FAIL")
    raise SystemExit(0 if passed else 1)


if __name__ == "__main__":
    main()
