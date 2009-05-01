set( lib_name GbxSerialDeviceAcfr )
GBX_ADD_LICENSE( LGPL )

set( build TRUE )
# don't give user an option
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

# this was already tested in the dir above
set( dep_libs GbxSerialAcfr GbxIceUtilAcfr )
# GBX_REQUIRE_LIBS( build LIB ${lib_name} ${proj_libs} )

if( build )
    
    include( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    include( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    file( GLOB hdrs *.h )
    file( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    target_link_libraries( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxsickacfr/gbxserialdeviceacfr ${hdrs} )

endif( build )
