LINK_LIBRARIES( GbxNovatelUtilAcfr )

ADD_EXECUTABLE( crc32test crc32test.cpp )
GBX_ADD_TEST( GbxNovatelUtilAcfr_Crc32Test crc32test )
