# INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

LINK_LIBRARIES( GbxUtilAcfr )

ADD_EXECUTABLE( mathtest mathtest.cpp )
GBX_ADD_TEST( GbxUtilAcfr_MathTest mathtest )

ADD_EXECUTABLE( tokenisetest tokenisetest.cpp )
GBX_ADD_TEST( GbxUtilAcfr_TokeniseTest tokenisetest )

ADD_EXECUTABLE( trivialtracertest trivialtracertest.cpp )
GBX_ADD_TEST( GbxUtilAcfr_TrivialTracerTest trivialtracertest )

ADD_EXECUTABLE( trivialstatustest trivialstatustest.cpp )
GBX_ADD_TEST( GbxUtilAcfr_TrivialStatusTest trivialstatustest )
