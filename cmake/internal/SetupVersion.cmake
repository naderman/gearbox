#
# define the project version so we can have access to it from the code
#

# alexm: for gcc need to produce this in the Makefile: -DGEARBOX_VERSION=\"X.Y.Z\", 
#        without escaping the quotes the compiler will strip them off.
# alexb: it seems that you also need to escape the quotes for windoze??

ADD_DEFINITIONS( "-DGEARBOX_VERSION=\\\"${GBX_PROJECT_VERSION}\\\"" )
# ADD_DEFINITIONS( "-DCMAKE_INSTALL_PREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"" )
