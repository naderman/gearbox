# Locate IceUtil home
#
# This module defines the following variables:
# ICEUTIL_FOUND : 1 if Ice is found, 0 otherwise
# ICEUTIL_HOME  : path where to find include, lib, bin, etc.

# start with 'not found'
SET( ICEUTIL_FOUND 0 CACHE BOOL "Do we have libIceUtil?" )

FIND_PATH( ICEUTIL_HOME_INCLUDE_ICEUTIL IceUtil.h
  # rational for this search order:
  #    source install w/env.var -> source install
  #    package -> package
  #    package + source install w/env.var -> source install
  #    package + source install w/out env.var -> package 
  #
  # installation selected by user
  $ENV{ICE_HOME}/include/IceUtil
  $ENV{ICEUTIL_HOME}/include/IceUtil
  # debian package installs Ice here
  /usr/include/IceUtil
  # Test standard installation points: newer versions first
  /opt/Ice-3.2.1/include/IceUtil
  /opt/Ice-3.2.0/include/IceUtil
  # some people may manually choose to install Ice here
  /usr/local/include/IceUtil
  # windows
  C:/Ice-3.2.0-VC80/include/IceUtil
  C:/Ice-3.2.0/include/IceUtil
  )
# MESSAGE( STATUS "DEBUG: Ice.h is apparently found in : ${ICEUTIL_HOME_INCLUDE_ICE}" )

# NOTE: if ICEUTIL_HOME_INCLUDE_ICE is set to *-NOTFOUND it will evaluate to FALSE
IF ( ICEUTIL_HOME_INCLUDE_ICEUTIL )

    SET( ICEUTIL_FOUND 1 CACHE BOOL "Do we have Ice?" FORCE )

    # strip 'file' twice to get rid off 'include/IceUtil'
#     MESSAGE( STATUS "DEBUG: ICEUTIL_HOME_INCLUDE_ICE=" ${ICEUTIL_HOME_INCLUDE_ICE} )
    GET_FILENAME_COMPONENT( ICEUTIL_HOME_INCLUDE ${ICEUTIL_HOME_INCLUDE_ICEUTIL} PATH )
#     MESSAGE( STATUS "DEBUG: ICEUTIL_HOME_INCLUDE=" ${ICEUTIL_HOME_INCLUDE} )
    GET_FILENAME_COMPONENT( ICEUTIL_HOME ${ICEUTIL_HOME_INCLUDE} PATH CACHE )
#     MESSAGE( STATUS "Setting ICEUTIL_HOME to ${ICEUTIL_HOME}" )

ENDIF ( ICEUTIL_HOME_INCLUDE_ICEUTIL )

IF ( ICEUTIL_FOUND )
    MESSAGE( STATUS "Looking for libIceUtil - found in ${ICEUTIL_HOME}")
ELSE ( ICEUTIL_FOUND )
    MESSAGE( STATUS "Looking for libIceUtil - not found")
ENDIF ( ICEUTIL_FOUND )
