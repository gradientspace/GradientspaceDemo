cmake_minimum_required(VERSION 3.22)

project(gradientspace_demo)

# this gets rid of MinSizeRel and RelWithDebInfo configurations...even though they are still listed in CMake GUI...
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# this sets CMAKE_INSTALL_PREFIX automatically, otherwise it defaults to ProgramFiles(x86)...
#IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#  SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "install folder is not used?" FORCE)
#ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#use C++20
set (CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE DEMO_SOURCE_FILES DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo_source/*.*")
set(MODULE_FILES ${DEMO_SOURCE_FILES})

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/submodules/GradientspaceCore ${CMAKE_CURRENT_BINARY_DIR}/gscore_bin )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/submodules/GradientspaceIO ${CMAKE_CURRENT_BINARY_DIR}/gsio_bin )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/submodules/GradientspaceGrid ${CMAKE_CURRENT_BINARY_DIR}/gsgrid_bin )

add_executable(gradientspace_demo ${MODULE_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gradientspace_demo)

target_link_libraries(gradientspace_demo gradientspace_core)
target_link_libraries(gradientspace_demo gradientspace_io)
target_link_libraries(gradientspace_demo gradientspace_grid)


# from https://stackoverflow.com/questions/14089284/copy-all-dlls-that-an-executable-links-to-to-the-executable-directory
# this copies all the dependent DLLs to the folder the exe was generaed in
# however it doesn't copy PDBs...

# The following variable is defined only on DLL systems
if (CMAKE_IMPORT_LIBRARY_SUFFIX)
  add_custom_command(
    TARGET gradientspace_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:gradientspace_demo> $<TARGET_FILE_DIR:gradientspace_demo>
    COMMAND_EXPAND_LISTS
  )
endif ()

