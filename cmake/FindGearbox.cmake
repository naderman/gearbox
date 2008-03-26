# Locate Gearbox installation

# This module defines
# GEARBOX_HOME where to find include, lib, bin, etc.
# GEARBOX_FOUND, If set to 0, don't try to use Gearbox libraries, scripts, etc.
#
# We look for the cmake manifest file generated by Gearbox.

# start with 'not found'
SET( GEARBOX_FOUND 0 CACHE BOOL "Do we have Gearbox?" )

FIND_PATH( GEARBOX_HOME gearbox_manifest.cmake
  $ENV{GEARBOX_HOME}
  # Test standard installation points
  /usr/local
  /opt/gearbox
  C:/gearbox
  )
# MESSAGE( STATUS "DEBUG: manifest.cmake is apparently found in : ${GEARBOX_HOME}" )

# NOTE: if GEARBOX_HOME is set to *-NOTFOUND it will evaluate to FALSE
IF( GEARBOX_HOME )
    SET( GEARBOX_FOUND 1 CACHE BOOL "Do we have Gearbox?" FORCE )
ENDIF( GEARBOX_HOME )
