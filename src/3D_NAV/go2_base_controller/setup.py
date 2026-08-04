from setuptools import setup

package_name = "go2_base_controller"
setup(
    name=package_name,
    version="1.0.0",
    packages=["loco"],
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml", "README.md"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="Chen-lvping",
    maintainer_email="2721705912@qq.com",
    description="Native ROS 2 Unitree Go2 base controller",
    license="MIT",
    entry_points={"console_scripts": [
        "go2_base_controller = loco.go2_base_controller:main",
        "emergency_stop_monitor = loco.emergency_stop_monitor:main",
    ]},
)
