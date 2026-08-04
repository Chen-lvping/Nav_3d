#!/usr/bin/env python3
"""Bridge configured ROS 1 topics to ROS 2 without coupling ROS distributions."""

from __future__ import annotations

import os
from pathlib import Path
from typing import Any, Dict, Iterable

import rclpy
import roslibpy
import yaml
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from rosidl_runtime_py.convert import message_to_ordereddict
from rosidl_runtime_py.set_message import set_message_fields
from rosidl_runtime_py.utilities import get_message


def _rename_time_fields(value: Any, to_ros2: bool) -> Any:
    """Recursively translate ROS 1 and ROS 2 builtin time field names."""
    if isinstance(value, dict):
        converted: Dict[str, Any] = {}
        for key, item in value.items():
            if to_ros2 and key == "secs":
                key = "sec"
            elif to_ros2 and key == "nsecs":
                key = "nanosec"
            elif not to_ros2 and key == "sec":
                key = "secs"
            elif not to_ros2 and key == "nanosec":
                key = "nsecs"
            converted[key] = _rename_time_fields(item, to_ros2)
        return converted
    if isinstance(value, (list, tuple)):
        return [_rename_time_fields(item, to_ros2) for item in value]
    return value


def _qos(entry: Dict[str, Any]) -> QoSProfile:
    reliability = (
        ReliabilityPolicy.BEST_EFFORT
        if entry.get("reliability", "reliable").lower() == "best_effort"
        else ReliabilityPolicy.RELIABLE
    )
    durability = (
        DurabilityPolicy.TRANSIENT_LOCAL
        if entry.get("durability", "volatile").lower() == "transient_local"
        else DurabilityPolicy.VOLATILE
    )
    return QoSProfile(
        history=HistoryPolicy.KEEP_LAST,
        depth=max(1, int(entry.get("depth", 10))),
        reliability=reliability,
        durability=durability,
    )


class Nav3dBridge(Node):
    def __init__(self) -> None:
        super().__init__("nav3d_ros2_adapter")
        default_config = os.environ.get(
            "NAV3D_BRIDGE_CONFIG", "/config/bridge.yaml"
        )
        self.declare_parameter("config", default_config)
        self.declare_parameter("rosbridge_host", os.environ.get("ROSBRIDGE_HOST", "ros1"))
        self.declare_parameter("rosbridge_port", int(os.environ.get("ROSBRIDGE_PORT", "9090")))

        config_path = Path(str(self.get_parameter("config").value)).expanduser()
        with config_path.open("r", encoding="utf-8") as stream:
            config = yaml.safe_load(stream) or {}

        self._ros1 = roslibpy.Ros(
            host=str(self.get_parameter("rosbridge_host").value),
            port=int(self.get_parameter("rosbridge_port").value),
        )
        self._ros1.run(timeout=15)
        if not self._ros1.is_connected:
            raise RuntimeError("ROS 1 rosbridge websocket connection failed")

        self._ros1_topics: list[roslibpy.Topic] = []
        self._ros2_entities: list[Any] = []
        self._configure(config.get("bridges", []))
        self.get_logger().info(
            f"Connected to ROS 1 at {self._ros1.host}:{self._ros1.port}; "
            f"configured {len(self._ros1_topics)} topic bridges"
        )

    def _configure(self, entries: Iterable[Dict[str, Any]]) -> None:
        for entry in entries:
            if not entry.get("enabled", True):
                continue
            direction = entry["direction"]
            topic = entry["topic"]
            ros1_type = entry["ros1_type"]
            ros2_type = entry["ros2_type"]
            ros2_class = get_message(ros2_type)
            ros1_topic = roslibpy.Topic(
                self._ros1,
                topic,
                ros1_type,
                queue_size=max(1, int(entry.get("depth", 10))),
                throttle_rate=max(0, int(entry.get("throttle_ms", 0))),
            )
            self._ros1_topics.append(ros1_topic)

            if direction == "ros1_to_ros2":
                publisher = self.create_publisher(ros2_class, topic, _qos(entry))
                self._ros2_entities.append(publisher)

                def forward_to_ros2(payload: Dict[str, Any], *, pub=publisher, cls=ros2_class, name=topic) -> None:
                    try:
                        message = cls()
                        set_message_fields(message, _rename_time_fields(dict(payload), True))
                        pub.publish(message)
                    except Exception as exc:  # keep other topic bridges alive
                        self.get_logger().error(f"ROS 1 -> ROS 2 conversion failed on {name}: {exc}")

                ros1_topic.subscribe(forward_to_ros2)
            elif direction == "ros2_to_ros1":
                ros1_topic.advertise()

                def forward_to_ros1(message: Any, *, destination=ros1_topic, name=topic) -> None:
                    try:
                        payload = _rename_time_fields(message_to_ordereddict(message), False)
                        destination.publish(roslibpy.Message(payload))
                    except Exception as exc:
                        self.get_logger().error(f"ROS 2 -> ROS 1 conversion failed on {name}: {exc}")

                subscription = self.create_subscription(
                    ros2_class, topic, forward_to_ros1, _qos(entry)
                )
                self._ros2_entities.append(subscription)
            else:
                raise ValueError(f"Unsupported direction {direction!r} for {topic}")

    def destroy_node(self) -> bool:
        for topic in self._ros1_topics:
            try:
                topic.unsubscribe()
                topic.unadvertise()
            except Exception:
                pass
        self._ros1.terminate()
        return super().destroy_node()


def main(args: list[str] | None = None) -> None:
    rclpy.init(args=args)
    node = Nav3dBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
