set( lib_name GbxSickAcfr )
GBX_ADD_LICENSE( LGPL )

set( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

add_subdirectory( gbxiceutilacfr )

set( dep_libs GbxUtilAcfr GbxIceUtilAcfr GbxSerialAcfr )
GBX_REQUIRE_LIBS( build LIB ${lib_name} ${dep_libs} )

# these are built internally
set( dep_libs ${dep_libs} GbxSerialDeviceAcfr )

if( build )

    add_subdirectory( gbxserialdeviceacfr )
    
    include( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    include( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    file( GLOB hdrs *.h )
    file( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    target_link_libraries( ${lib_name} ${dep_libs} )
    GBX_ADD_PKGCONFIG( ${lib_name} "Drives SICK hardware, directly connected to the computer." proj_libs dep_libs "" "" )

    GBX_ADD_HEADERS( gbxsickacfr ${hdrs} )

    if( GBX_BUILD_TESTS )
        add_subdirectory( test )
    endif( GBX_BUILD_TESTS )

endif( build )
