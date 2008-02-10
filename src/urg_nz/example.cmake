PROJECT( urg_nz_example )

INCLUDE_DIRECTORIES( @CMAKE_INSTALL_PREFIX@ )

ADD_EXECUTABLE( urg_nz_example example.cpp )
TARGET_LINK_LIBRARIES( urg_nz_example urg_nz )
SET_TARGET_PROPERTIES( urg_nz_example PROPERTIES
                       LINK_FLAGS "-L@CMAKE_INSTALL_PREFIX@/lib/gearbox"
                       INSTALL_RPATH "${INSTALL_RPATH};@CMAKE_INSTALL_PREFIX@/lib/gearbox"
                       BUILD_WITH_INSTALL_RPATH TRUE )