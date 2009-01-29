# INCLUDE( ${HYDRO_CMAKE_DIR}/UseBasicRules.cmake )
# INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/../UseHydroIceUtil.cmake )
LINK_LIBRARIES( GbxIceUtilAcfr )

ADD_EXECUTABLE( buffertest buffertest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfr_BufferTest buffertest )

ADD_EXECUTABLE( storetest storetest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfr_StoreTest storetest )

ADD_EXECUTABLE( notifytest notifytest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfr_NotifyTest notifytest )

ADD_EXECUTABLE( threadtest threadtest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfr_ThreadTest threadtest )

ADD_EXECUTABLE( safethreadtest safethreadtest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfr_SafeThreadTest safethreadtest )
