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
set( GBX_PROJECT_VERSION 9.02+ )

#
# The rest is done by a script
#
include( cmake/internal/Setup.cmake )
