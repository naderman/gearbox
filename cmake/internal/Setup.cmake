#
# Give feedback on custom entries
#
MESSAGE( STATUS "Setting project name to ${PROJECT_NAME}" )
MESSAGE( STATUS "Setting project version to ${GBX_PROJECT_VERSION}" )



SET( GBX_CMAKE_DIR ${${PROJECT_NAME}_SOURCE_DIR}/cmake CACHE INTERNAL "Location of CMake scripts" )

#
# Process version number
#
INCLUDE( ${GBX_CMAKE_DIR}/SetupVersion.cmake )

#
# Determine OS, and make os-specific choices
#
INCLUDE( ${GBX_CMAKE_DIR}/SetupOs.cmake )

#
# Project directories, including installation
#
INCLUDE( ${GBX_CMAKE_DIR}/SetupDirectories.cmake )

#
# Set the build type (affects debugging symbols and optimization)
#
INCLUDE( ${GBX_CMAKE_DIR}/SetupBuildType.cmake )


#
# Include internal macro definitions
#
INCLUDE( ${GBX_CMAKE_DIR}/Assert.cmake )
INCLUDE( ${GBX_CMAKE_DIR}/TargetUtils.cmake )
INCLUDE( ${GBX_CMAKE_DIR}/DependencyUtils.cmake )

#
# Defaults for big source code switches
# (these are defaults. after the user modifies these in GUI they stay in cache)
#
OPTION( GBX_BUILD_LICENSE  "Enables writing LICENCE file. For admins only." OFF )
MARK_AS_ADVANCED( GBX_BUILD_LICENSE )

#
# check compiler type and version
#
INCLUDE( ${GBX_CMAKE_DIR}/CheckCompiler.cmake )

#
# Defaults for big source code switches
# (these are defaults. after the user modifies these in GUI they stay in cache)
#
OPTION( GBX_BUILD_TESTS    "Enables compilation of all tests" ON  )

#
# Look for low-level C headers, write defines to config.h
#
INCLUDE( ${GBX_CMAKE_DIR}/WriteConfigH.cmake )

#
# Installation preferences
#
# CMake defaults
# see: \http://www.cmake.org/Wiki/CMake_RPATH_handling
#
# use, i.e. don't skip the full RPATH for the build tree
# SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
# SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing
SET( CMAKE_INSTALL_RPATH ${GBX_LIB_INSTALL_DIR} )

# Enable testing by including the Dart module
# (must be done *before* entering source directories )
INCLUDE(${CMAKE_ROOT}/Modules/Dart.cmake)
ENABLE_TESTING()

#
# Enter the source tree
#
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( submitted )
ADD_SUBDIRECTORY( retired )

# Some cmake and shell scripts need to be installed
ADD_SUBDIRECTORY( cmake )

#
# Write results of CMake activity to file
#
GBX_WRITE_MANIFEST()
GBX_WRITE_OPTIONS()

#
# Print license information to a file
#
IF( GBX_BUILD_LICENSE )
    GBX_WRITE_LICENSE()
ENDIF( GBX_BUILD_LICENSE )

#
# Print results of CMake activity
#
GBX_CONFIG_REPORT( "Nothing special" )

#
# House-keeping, clear lists of targets, licenses, options, etc.
#
GBX_RESET_ALL_TARGET_LISTS()
GBX_RESET_ALL_DEPENDENCY_LISTS()
