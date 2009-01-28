# A place for well-tested and well-documented libraries.

MESSAGE( STATUS "== SRC ==" )

# When adding new directories, please maintain order of inter-dependencies.
# Otherwise, maintain alphabetical order.

ADD_SUBDIRECTORY( basicexample )
ADD_SUBDIRECTORY( flexiport )
ADD_SUBDIRECTORY( gbxadvancedexample )
ADD_SUBDIRECTORY( gbxserialacfr )
ADD_SUBDIRECTORY( gbxutilacfr )
ADD_SUBDIRECTORY( gbxgarminacfr )
ADD_SUBDIRECTORY( gbxnovatelacfr )
ADD_SUBDIRECTORY( gbxsickacfr )
ADD_SUBDIRECTORY( gbxsmartbatteryacfr )
ADD_SUBDIRECTORY( hokuyo_aist )

