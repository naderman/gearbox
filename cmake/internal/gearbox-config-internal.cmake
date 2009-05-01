set( GEARBOX_FOUND 1 )   

# This is potentially problematic: installed directory structure is different from in-source one.
set( GEARBOX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gearbox/src" )

set( GEARBOX_CMAKE_DIR "${CMAKE_SOURCE_DIR}/gearbox/cmake" )

set( GEARBOX_USE_FILE "${GEARBOX_CMAKE_DIR}/gearbox-use-file.cmake" )

# this is where Gearbox libs will be installed 
set( GEARBOX_LINK_DIR ${CMAKE_INSTALL_PREFIX}/lib/gearbox )
