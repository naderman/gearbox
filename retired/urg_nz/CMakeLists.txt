SET( lib_name urg_nz )
set ( lib_desc "Hokuyo URG laser scanner driver" )
GBX_ADD_LICENSE( GPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "Only Linux OS is supported" )

IF( build )

    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    SET( hdrs urg_nz.h )
    SET( srcs urg_nz.cpp )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    GBX_ADD_PKGCONFIG( ${lib_name} ${lib_desc} "" "" "" "" )

    GBX_ADD_HEADERS( ${lib_name} ${hdrs} )

    GBX_ADD_EXAMPLE( ${lib_name} example.cmake.in example.cmake example.cpp example.readme )

ENDIF( build )
