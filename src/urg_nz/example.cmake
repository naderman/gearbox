PROJECT( urg_example )

INCLUDE_DIRECTORIES( @CMAKE_INSTALL_PREFIX@ )

ADD_EXECUTABLE( urg_example example.cpp )
TARGET_LINK_LIBRARIES( urg_example urglaser )
SET_TARGET_PROPERTIES( urg_example PROPERTIES
                       LINK_FLAGS "-L@CMAKE_INSTALL_PREFIX@/lib/gearbox"
                       INSTALL_RPATH "${INSTALL_RPATH};@CMAKE_INSTALL_PREFIX@/lib/gearbox"
                       BUILD_WITH_INSTALL_RPATH TRUE )