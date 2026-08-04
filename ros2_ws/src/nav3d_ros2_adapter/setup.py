from glob import glob
from setuptools import find_packages, setup

package_name = "nav3d_ros2_adapter"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        ("share/" + package_name + "/config", glob("config/*.yaml")),
        ("share/" + package_name + "/launch", glob("launch/*.launch.py")),
    ],
    install_requires=["setuptools", "PyYAML", "roslibpy>=1.5.0"],
    zip_safe=True,
    maintainer="Nav_3d maintainers",
    maintainer_email="user@example.com",
    description="ROS 1/ROS 2 topic adapter for Nav_3d",
    license="MIT",
    entry_points={"console_scripts": ["bridge = nav3d_ros2_adapter.bridge:main"]},
)
