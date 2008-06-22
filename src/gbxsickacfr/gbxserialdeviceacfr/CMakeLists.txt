SET( lib_name GbxSerialDeviceAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
# don't give user an option
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

# this was already tested in the dir above
SET( int_libs GbxSerialAcfr GbxIceUtilAcfr )
# GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${proj_libs} )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    INCLUDE( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${int_libs} )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxsickacfr/gbxserialdeviceacfr ${hdrs} )

ENDIF( build )
