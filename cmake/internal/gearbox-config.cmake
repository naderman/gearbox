# Find Gearbox includes and library.
#
#

set( GEARBOX_FOUND 1 )

# this is the installed location of <package>-config.cmake file
get_filename_component( _found_dir "${CMAKE_CURRENT_LIST_FILE}" PATH )

# load all exported Gearbox targets
include( ${_found_dir}/gearbox-targets.cmake )

# assume that gearbox-config.cmake was installed into
# <install-root>/lib/gearbox/
set( _install_dir "${_found_dir}/../../" )

get_filename_component(
    GEARBOX_INCLUDE_DIR
    "${_install_dir}/include/gearbox"
    ABSOLUTE )

get_filename_component(
    GEARBOX_CMAKE_DIR
    "${_install_dir}/share/cmake/Modules"
    ABSOLUTE )

set( GEARBOX_USE_FILE "${GEARBOX_CMAKE_DIR}/gearbox-use-file.cmake" )

get_filename_component(
    GEARBOX_LINK_DIR
    "${_install_dir}/lib/gearbox"
    ABSOLUTE )

set( _found_dir )
set( _install_dir )
