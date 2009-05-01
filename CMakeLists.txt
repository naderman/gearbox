#
# Test CMake version
#
CMAKE_MINIMUM_REQUIRED( VERSION 2.4 FATAL_ERROR )

#
# project name
#
project( gearbox )

#
# project version
#
set( GBX_PROJECT_VERSION_MAJOR "9" )
set( GBX_PROJECT_VERSION_MINOR "02" )
set( GBX_PROJECT_VERSION_PATCH "+" )

set( GBX_PROJECT_VERSION 
    ${GBX_PROJECT_VERSION_MAJOR}.${GBX_PROJECT_VERSION_MINOR}.${GBX_PROJECT_VERSION_PATCH} )

#
# The rest is done by a script
#
include( cmake/internal/Setup.cmake )
