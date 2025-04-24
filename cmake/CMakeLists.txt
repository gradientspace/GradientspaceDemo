cmake_minimum_required(VERSION 3.10)

project(gradientspace_demo)

# this gets rid of MinSizeRel and RelWithDebInfo configurations...even though they are still listed in CMake GUI...
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# this sets CMAKE_INSTALL_PREFIX automatically, otherwise it defaults to ProgramFiles(x86)...
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "install folder is not used?" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#use C++20
set (CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE DEMO_SOURCE_FILES DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../demo_source/*.*")
set(MODULE_FILES ${DEMO_SOURCE_FILES})


add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/GradientspaceCore/Build/cmake ${CMAKE_CURRENT_BINARY_DIR}/gscore_bin )

add_executable(gradientspace_demo ${MODULE_FILES})

target_link_libraries(gradientspace_demo gradientspace_core)
