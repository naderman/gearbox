SET (libName urg_nz)
set (libDesc "Hokuyo URG laser scanner driver")
GBX_ADD_LICENSE (GPL)

SET (build TRUE)
GBX_REQUIRE_OPTION (build LIB ${libName} ON)

SET (reqLibs flexiport)
GBX_REQUIRE_TARGETS (build LIB ${libName} ${reqLibs})

IF (build)
	INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

	SET (hdrs urg_nz.h)
    SET (srcs urg_nz.cpp)

	IF (WIN32)
		ADD_DEFINITIONS (-DURG_NZ_EXPORTS)
	ENDIF (WIN32)
	GBX_ADD_LIBRARY (${libName} SHARED ${srcs})
	TARGET_LINK_LIBRARIES (${libName} ${reqLibs})
	GBX_ADD_PKGCONFIG (${libName} ${libDesc} ${reqLibs} "" "" "")

	GBX_ADD_HEADERS (${libName} ${hdrs})

	IF (GBX_BUILD_TESTS)
		ADD_SUBDIRECTORY (test)
	ENDIF (GBX_BUILD_TESTS)
ENDIF( build )
