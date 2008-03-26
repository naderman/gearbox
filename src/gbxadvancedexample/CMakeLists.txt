SET( lib_name GbxAdvancedExample )
GBX_ADD_LICENSE( GPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} OFF )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

SET( dep_libs basicexample )
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${dep_libs} )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxadvancedexample ${hdrs} )

ENDIF( build )
