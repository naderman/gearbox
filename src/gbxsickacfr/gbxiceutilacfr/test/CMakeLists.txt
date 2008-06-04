# INCLUDE( ${HYDRO_CMAKE_DIR}/UseBasicRules.cmake )
# INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/../UseHydroIceUtil.cmake )
LINK_LIBRARIES( GbxIceUtilAcfr )

ADD_EXECUTABLE( buffertest buffertest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfrBufferTest buffertest )

ADD_EXECUTABLE( storetest storetest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfrStoreTest storetest )

ADD_EXECUTABLE( notifytest notifytest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfrNotifyTest notifytest )

ADD_EXECUTABLE( threadtest threadtest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfrThreadTest threadtest )

ADD_EXECUTABLE( safethreadtest safethreadtest.cpp )
GBX_ADD_TEST( GbxIceUtilAcfrSafeThreadTest safethreadtest )
