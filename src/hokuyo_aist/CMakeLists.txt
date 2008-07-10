SET (libName hokuyo_aist)
set (libDesc "Hokuyo laser scanner driver")
GBX_ADD_LICENSE (GPL)

SET (build TRUE)
GBX_REQUIRE_OPTION (build LIB ${libName} ON)

SET (reqLibs flexiport)
GBX_REQUIRE_TARGETS (build LIB ${libName} ${reqLibs})

IF (build)
	INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

	SET (hdrs hokuyo_aist.h)
	SET (srcs hokuyo_aist.cpp)

	IF (WIN32)
		ADD_DEFINITIONS (-DHOKUYO_AIST_EXPORTS)
	ENDIF (WIN32)
	GBX_ADD_LIBRARY (${libName} SHARED ${srcs})
	TARGET_LINK_LIBRARIES (${libName} ${reqLibs})
	GBX_ADD_PKGCONFIG (${libName} ${libDesc} ${reqLibs} "" "" "")

	GBX_ADD_HEADERS (${libName} ${hdrs})

	IF (GBX_BUILD_TESTS)
		ADD_SUBDIRECTORY (test)
	ENDIF (GBX_BUILD_TESTS)
ENDIF( build )
