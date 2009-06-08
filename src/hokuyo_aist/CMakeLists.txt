set (libName hokuyo_aist)
set (libDesc "Hokuyo laser scanner driver")
set (libVersion 1.0.0)
GBX_ADD_LICENSE (LGPL)

set (build TRUE)
GBX_REQUIRE_OPTION (build LIB ${libName} ON)

set (reqLibs flexiport)
GBX_REQUIRE_LIBS (build LIB ${libName} ${reqLibs})

if (build)
	include (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

	set (hdrs hokuyo_aist.h)
	set (srcs hokuyo_aist.cpp)

	if (WIN32)
		if (GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
			add_definitions (-DHOKUYO_AIST_EXPORTS)
		else (GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
			add_definitions (-DHOKUYO_AIST_STATIC -DFLEXIPORT_STATIC)
		endif (GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
	endif (WIN32)
	GBX_ADD_LIBRARY (${libName} DEFAULT ${libVersion} ${srcs})
	target_link_libraries (${libName} ${reqLibs})
	GBX_ADD_PKGCONFIG (${libName} ${libDesc} ${reqLibs} "" "" "" ${libVersion})

	GBX_ADD_HEADERS (${libName} ${hdrs})

	if (GBX_BUILD_TESTS)
		add_subdirectory (test)
	endif (GBX_BUILD_TESTS)

	option (HOKUYO_AIST_BUILD_BINDINGS "Build the Python bindings for Hokuyo_aist" ON)
	if (HOKUYO_AIST_BUILD_BINDINGS)
		add_subdirectory (python)
	endif (HOKUYO_AIST_BUILD_BINDINGS)
endif (build)
