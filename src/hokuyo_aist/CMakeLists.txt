SET (libName hokuyo_aist)
set (libDesc "Hokuyo laser scanner driver")
GBX_ADD_LICENSE (LGPL)

SET (build TRUE)
GBX_REQUIRE_OPTION (build LIB ${libName} ON)

SET (reqLibs flexiport)
GBX_REQUIRE_TARGETS (build LIB ${libName} ${reqLibs})

if(build)
	INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

	SET (hdrs hokuyo_aist.h)
	SET (srcs hokuyo_aist.cpp)

	if(WIN32)
		if(GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
			ADD_DEFINITIONS (-DHOKUYO_AIST_EXPORTS)
		else(GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
			ADD_DEFINITIONS (-DHOKUYO_AIST_STATIC -DFLEXIPORT_STATIC)
		endif(GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
	endif(WIN32)
	GBX_ADD_LIBRARY(${libName} DEFAULT ${srcs})
	TARGET_LINK_LIBRARIES (${libName} ${reqLibs})
	GBX_ADD_PKGCONFIG (${libName} ${libDesc} ${reqLibs} "" "" "")

	GBX_ADD_HEADERS (${libName} ${hdrs})

	if(GBX_BUILD_TESTS)
		ADD_SUBDIRECTORY (test)
	endif(GBX_BUILD_TESTS)

	OPTION (HOKUYO_AIST_BUILD_BINDINGS "Build the Python bindings for Hokuyo_aist" ON)
	if(HOKUYO_AIST_BUILD_BINDINGS)
		ADD_SUBDIRECTORY (python)
	endif(HOKUYO_AIST_BUILD_BINDINGS)
endif(build)
