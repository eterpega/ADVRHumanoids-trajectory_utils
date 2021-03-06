# 
#  Copyright (C) 2016 Walkman
#  Author: Enrico Mingo Hoffman
#  email: enrico.mingo@iit.it
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>
#

cmake_minimum_required(VERSION 2.8.12)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
    cmake_policy(SET CMP0005 NEW)
    cmake_policy(SET CMP0017 NEW)
endif(COMMAND cmake_policy)
include(ExternalProject)
PROJECT(trajectory_utils)

FIND_PACKAGE(orocos_kdl REQUIRED)
FIND_PACKAGE(kdl_parser REQUIRED)
FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(roslib QUIET)
FIND_PACKAGE(tf QUIET)
FIND_PACKAGE(nav_msgs QUIET)
FIND_PACKAGE(rviz_visual_tools QUIET)
FIND_PACKAGE(interactive_markers QUIET)
FIND_PACKAGE(catkin QUIET)

find_package(catkin REQUIRED COMPONENTS
  message_generation
  geometry_msgs
  std_msgs
)


find_package(srdfdom_advr QUIET)
find_package(OpenSoT QUIET)
FIND_PACKAGE(XBotInterface QUIET)



# compilation flags
option(TRAJECTORY_UTILS_COMPILE_EXAMPLES "Compile trajectory_utils examples" FALSE)
option(TRAJECTORY_UTILS_COMPILE_TESTS "Compile trajectory_utils tests" FALSE)
option(TRAJECTORY_UTILS_COMPILE_TRJ_DESIG "Compile trajectory_designer" FALSE)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wmaybe-uninitialized -Wuninitialized")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wmaybe-uninitialized -Wuninitialized")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

INCLUDE_DIRECTORIES(include ${orocos_kdl_DIRS} ${kdl_parser_INCLUDE_DIRS}
                            ${Boost_INCLUDE_DIRS}
                            ${roslib_INCLUDE_DIRS} ${tf_INCLUDE_DIRS} ${nav_msgs_INCLUDE_DIRS}
                            ${rviz_visual_tools_INCLUDE_DIRS})

file(GLOB_RECURSE trajectory_utils_INCLUDES
"${CMAKE_CURRENT_SOURCE_DIR}/include/trajectory_utils" *.h*
"${CMAKE_CURRENT_SOURCE_DIR}/include/trajectory_utils/paths" *.h*
"${CMAKE_CURRENT_SOURCE_DIR}/include/trajectory_utils/utils" *.h*
"${CMAKE_CURRENT_SOURCE_DIR}/include/trajectory_utils/ros_nodes" *.h*)

message("trajectory_utils added to compilation...")
set(TRAJECTORY_UTILS_SOURCES src/trajectory_utils.cpp
                             src/paths/path_circle_fix.cpp)

if(${tf_FOUND} AND ${nav_msgs_FOUND} AND ${roslib_FOUND} AND ${rviz_visual_tools_FOUND})
    message("ros_trj_publisher added to compilation...")
    set(TRAJECTORY_UTILS_SOURCES ${TRAJECTORY_UTILS_SOURCES}
        src/utils/ros_trj_publisher.cpp)
endif()

message("ros messages added to compilation...")
add_message_files(DIRECTORY msg FILES
      Cartesian.msg
      CartesianTrj.msg
      segment.msg
      segmentTrj.msg
          )

        ## Generate added messages and services with any dependencies listed here
generate_messages(DEPENDENCIES
      geometry_msgs
      std_msgs
          )

catkin_package(
      INCLUDE_DIRS include
      CATKIN_DEPENDS message_runtime geometry_msgs std_msgs
      DEPENDS system_lib
          )

#This will install the python script in /build/install/lib
catkin_install_python(PROGRAMS python/joint_states_topic_to_param.py
                      DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

#This will install the launch folder in build/install/share/trajectory_utils/launch
install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch)


ADD_LIBRARY(trajectory_utils SHARED ${TRAJECTORY_UTILS_SOURCES}
${trajectory_utils_INCLUDES})
TARGET_LINK_LIBRARIES(trajectory_utils ${kdl_parser_LIBRARIES} ${orocos_kdl_LIBRARIES}
${Boost_LIBRARIES} ${rviz_visual_tools_LIBRARIES})

if(${TRAJECTORY_UTILS_COMPILE_EXAMPLES})
    if(${tf_FOUND} AND ${nav_msgs_FOUND} AND ${roslib_FOUND} AND ${rviz_visual_tools_FOUND})
        message("exampleTrjPublisher added to compilation...")
        ADD_EXECUTABLE(exampleTrjPublisher example/exampleTrjPublisher.cpp)
        TARGET_LINK_LIBRARIES(exampleTrjPublisher trajectory_utils ${kdl_parser_LIBRARIES} ${orocos_kdl_LIBRARIES}
        ${Boost_LIBRARIES} ${rviz_visual_tools_LIBRARIES})
    endif()

    if(${OpenSoT_FOUND} AND ${srdfdom_advr_FOUND})
        message("exampleOpenSoT added to compilation...")
        ADD_EXECUTABLE(exampleOpenSoT example/exampleOpenSoT.cpp)
        TARGET_LINK_LIBRARIES(exampleOpenSoT ${kdl_parser_LIBRARIES} ${orocos_kdl_LIBRARIES}
        ${Boost_LIBRARIES} ${rviz_visual_tools_LIBRARIES} ${OpenSoT_LIBRARIES} ${XBotInterface_LIBRARIES}
        ${srdfdom_advr_LIBRARIES})

       catkin_package(
              INCLUDE_DIRS include
              LIBRARIES trajectory_utils
              CATKIN_DEPENDS roscpp
              DEPENDS system_lib
              )

       install(TARGETS exampleOpenSoT
              ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
              LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
              RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
              )
    endif()
endif()

if(${TRAJECTORY_UTILS_COMPILE_TRJ_DESIG})
if(${tf_FOUND} AND ${nav_msgs_FOUND} AND ${roslib_FOUND} AND ${rviz_visual_tools_FOUND})
    if(${interactive_markers_FOUND} AND ${catkin_FOUND})
        message("trajectory_designer added to compilation...")
        ADD_EXECUTABLE(trajectory_designer src/ros_nodes/trj_designer.cpp)
        TARGET_LINK_LIBRARIES(trajectory_designer trajectory_utils ${kdl_parser_LIBRARIES} ${orocos_kdl_LIBRARIES}
            ${Boost_LIBRARIES} ${rviz_visual_tools_LIBRARIES} ${interactive_markers_LIBRARIES})


       catkin_package(
              INCLUDE_DIRS include
              LIBRARIES trajectory_utils
              CATKIN_DEPENDS roscpp message_runtime geometry_msgs std_msgs
              DEPENDS system_lib
              )
     
       install(TARGETS trajectory_designer
              ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
              LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
              RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
              )
    endif()
endif()
endif()
########################################################################
# use YCM to export idynutils so that it can be found using find_package #
########################################################################

set(VARS_PREFIX "TRAJECTORY_UTILS")

set(TRAJECTORY_UTILS_MAJOR_VERSION 0)
set(TRAJECTORY_UTILS_MINOR_VERSION 0)
set(TRAJECTORY_UTILS_PATCH_VERSION 1)
set(TRAJECTORY_UTILS_VERSION ${TRAJECTORY_UTILS_MAJOR_VERSION}.${TRAJECTORY_UTILS_MINOR_VERSION}.${TRAJECTORY_UTILS_PATCH_VERSION})

find_package(YCM REQUIRED)
include(YCMDefaultDirs)
ycm_default_dirs(${VARS_PREFIX})

target_include_directories(trajectory_utils PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
                                            "$<INSTALL_INTERFACE:${${VARS_PREFIX}_INSTALL_INCLUDEDIR}>")

set_target_properties(trajectory_utils PROPERTIES VERSION ${${VARS_PREFIX}_VERSION}
                                SOVERSION ${${VARS_PREFIX}_VERSION})

target_link_libraries(trajectory_utils LINK_PUBLIC )

install(DIRECTORY include/ 
        DESTINATION "${${VARS_PREFIX}_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h*")

install(TARGETS trajectory_utils  
        EXPORT trajectory_utils
        ARCHIVE DESTINATION "${${VARS_PREFIX}_INSTALL_BINDIR}" COMPONENT lib
        RUNTIME DESTINATION "${${VARS_PREFIX}_INSTALL_BINDIR}" COMPONENT bin
        LIBRARY DESTINATION "${${VARS_PREFIX}_INSTALL_LIBDIR}" COMPONENT shlib)
        
#enabling it will add all idynutils dependencies as dependencies for third party users
set_property(GLOBAL APPEND PROPERTY ${VARS_PREFIX}_TARGETS trajectory_utils)


include(InstallBasicPackageFiles)
install_basic_package_files(trajectory_utils   VARS_PREFIX ${VARS_PREFIX}
                                        VERSION ${${VARS_PREFIX}_VERSION}
                                        COMPATIBILITY SameMajorVersion
                                        TARGETS_PROPERTY ${VARS_PREFIX}_TARGETS
                                        NO_CHECK_REQUIRED_COMPONENTS_MACRO)

include(AddUninstallTarget)

#######################
# Add Testing target  #
#######################
if(${TRAJECTORY_UTILS_COMPILE_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()
