cmake_minimum_required(VERSION 3.5)
project(ethercat_erob)

set(CMAKE_BUILD_TYPE Debug)  # or Release, RelWithDebInfo, etc.

# EtherLab
set(ETHERLAB_DIR /usr/local/etherlab)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_library(ETHERCAT_LIB ethercat HINTS ${ETHERLAB_DIR}/lib)

include_directories(
  ${ETHERLAB_DIR}/include
)

add_executable(
  EC_PROJECT
  src/main.cpp
  src/EtherCATMaster.cpp
  src/Joint.cpp
  src/RealtimeManager.cpp
  src/JointConfig.cpp
)

target_include_directories(
  EC_PROJECT
  PRIVATE
  ${ETHERLAB_DIR}/include
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(EC_PROJECT ${ETHERCAT_LIB})
