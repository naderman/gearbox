#
# Give feedback on custom entries
#
MESSAGE( STATUS "Setting project name to ${PROJECT_NAME}" )
MESSAGE( STATUS "Setting project version to ${GBX_PROJECT_VERSION}" )



SET( GBX_CMAKE_DIR ${${PROJECT_NAME}_SOURCE_DIR}/cmake CACHE PATH "Location of CMake scripts" )

#
# Process version number
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/SetupVersion.cmake )

#
# Project directories
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/SetupDirectories.cmake )

#
# Determine OS, and make os-specefic choices
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/SetupOs.cmake )

#
# Set the build type (affects debugging symbols and optimization)
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/SetupBuildType.cmake )


#
# Include internal macro definitions
#
INCLUDE( ${GBX_CMAKE_DIR}/Assert.cmake )
INCLUDE( ${GBX_CMAKE_DIR}/internal/TargetUtils.cmake )
INCLUDE( ${GBX_CMAKE_DIR}/internal/DependencyUtils.cmake )

#
# Defaults for big source code switches
# (these are defaults. after the user modifies these in GUI they stay in cache)
#
OPTION( GBX_BUILD_LICENSE  "Enables writing LICENCE file. For admins only." OFF )

#
# check compiler type and version
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/CheckCompiler.cmake )

#
# Defaults for big source code switches
# (these are defaults. after the user modifies these in GUI they stay in cache)
#
OPTION( GBX_BUILD_TESTS    "Enables compilation of all tests" ON  )

#
# Look for low-level C headers, write defines to config.h
#
INCLUDE( ${GBX_CMAKE_DIR}/internal/WriteConfigH.cmake )

#
# Installation preferences
#
# CMake default is FALSE
# SET( CMAKE_SKIP_BUILD_RPATH TRUE )
# CMake default is FALSE
# SET( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
SET( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib )

# Enable testing by including the Dart module
# (must be done *before* entering source directories )
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)
ENABLE_TESTING()

#
# Enter the source tree
#
ADD_SUBDIRECTORY ( src )
ADD_SUBDIRECTORY ( submitted )
# ADD_SUBDIRECTORY ( retired )

# Some cmake and shell scripts need to be installed
ADD_SUBDIRECTORY ( cmake )

#
# Print results of CMake activity
#
GBX_WRITE_MANIFEST()

#
# Print license information to a file
#
IF ( GBX_BUILD_LICENSE )
    GBX_WRITE_LICENSE()
ENDIF ( GBX_BUILD_LICENSE )

GBX_CONFIG_REPORT( "Nothing special" )

#
# House-keeping, clear lists of targets, licenses, etc.
#
GBX_RESET_ALL_LISTS()
