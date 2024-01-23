# Install script for directory: /home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/structures.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/utils.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/param_setting.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/semaphore.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/camera_model.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/imagebuffer.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/base_thread.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/capture_thread.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/process_thread.hpp;/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include/bird_view.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/include" TYPE FILE FILES
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/structures.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/utils.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/param_setting.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/semaphore.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/camera_model.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/imagebuffer.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/base_thread.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/capture_thread.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/process_thread.hpp"
    "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/src/bird_view.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so"
         RPATH "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib" TYPE SHARED_LIBRARY FILES "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/build/src/libsvs.so")
  if(EXISTS "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so"
         OLD_RPATH ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ubuntu/Documents/WZ_dev/PERCEPTION/AVP_SLAM/SVS/lib/libsvs.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

