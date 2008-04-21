#
# Test CMake version
#
CMAKE_MINIMUM_REQUIRED( VERSION 2.4 FATAL_ERROR )

#
# project name
#
PROJECT( gearbox )

#
# project version string
#
SET( GBX_PROJECT_VERSION            1.0.0        CACHE STRING "Version of GearBox distribution" )

#
# The rest is done by a script
#
INCLUDE( cmake/internal/Setup.cmake )
