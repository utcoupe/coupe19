cmake_minimum_required(VERSION 2.8.3)
project(driver_teraranger)

find_package(catkin REQUIRED COMPONENTS
    port_finder
    ard_others
    game_manager
    sensor_msgs
)

catkin_package(
    CATKIN_DEPENDS
    port_finder
    ard_others
    sensor_msgs
)

include_directories(
    ${catkin_INCLUDE_DIRS}
)

#############
## Install ##
#############

catkin_install_python(
    PROGRAMS src/teraranger_node.py
    DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
