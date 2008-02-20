PROJECT( urg_nz_example )

INCLUDE_DIRECTORIES( /usr/local )

ADD_EXECUTABLE( urg_nz_example example.cpp )
TARGET_LINK_LIBRARIES( urg_nz_example urg_nz )
SET_TARGET_PROPERTIES( urg_nz_example PROPERTIES
                       LINK_FLAGS "-L/usr/local/lib/gearbox"
                       INSTALL_RPATH "${INSTALL_RPATH};/usr/local/lib/gearbox"
                       BUILD_WITH_INSTALL_RPATH TRUE )
