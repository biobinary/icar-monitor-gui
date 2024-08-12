import os

from launch import LaunchDescription
from launch.substitutions import Command

from launch_ros.actions import Node
from launch_ros.descriptions import ParameterValue

from ament_index_python import get_package_share_path


def generate_launch_description():

    ld = LaunchDescription()

    urdf_location = os.path.join(get_package_share_path("my_rviz_description"), "urdf/mobil.urdf.xacro")
    urdf_content = ParameterValue(Command(["xacro ", urdf_location]))

    robot_state_pub = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[
            {'robot_description' : urdf_content}
        ]
    )

    main_monitor = Node(
        package='my_rviz_panel',
        executable='myviz'
    )

    ld.add_action(robot_state_pub)
    ld.add_action(main_monitor)

    return ld
