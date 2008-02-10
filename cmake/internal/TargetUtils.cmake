#
# Components should add themselves by calling 'GBX_ADD_EXECUTABLE'
# instead of 'ADD_EXECUTABLE' in CMakeLists.txt.
# Usage: GBX_ADD_EXECUTABLE( name src1 src2 src3 )
#
MACRO( GBX_ADD_EXECUTABLE name )
  ADD_EXECUTABLE( ${name} ${ARGN} )
  INSTALL( TARGETS ${name} RUNTIME DESTINATION bin )
  SET( templist ${COMPONENT_LIST} )
  LIST ( APPEND templist ${name} )
#   MESSAGE ( STATUS "DEBUG: ${templist}" )
  SET( COMPONENT_LIST ${templist} CACHE INTERNAL "Global list of components to build" FORCE )
  MESSAGE( STATUS "Planning to Build Executable: ${name}" )
ENDMACRO( GBX_ADD_EXECUTABLE name )

#
# Components should add themselves by calling 'GBX_ADD_EXECUTABLE'
# instead of 'ADD_LIBRARY' in CMakeLists.txt.
# Usage: GBX_ADD_LIBRARY( name src1 src2 src3 )
#
MACRO( GBX_ADD_LIBRARY name )
    ADD_LIBRARY( ${name} ${ARGN} )
    INSTALL( TARGETS ${name} LIBRARY DESTINATION lib/gearbox )
    SET( templist ${LIBRARY_LIST} )
    LIST ( APPEND templist ${name} )
    SET( LIBRARY_LIST ${templist} CACHE INTERNAL "Global list of libraries to build" FORCE )
    MESSAGE( STATUS "Planning to Build Library   : ${name}" )
ENDMACRO( GBX_ADD_LIBRARY name )

#
# GBX_ADD_HEADERS( install_subdir FILE0 [FILE1 FILE2 ...] )
#
# Specialization of INSTALL(FILES ...) for GearBox project.
# All files are installed into PREFIX/include/gearbox/${install_subdir}
#
MACRO( GBX_ADD_HEADERS install_subdir )
    INSTALL( FILES ${ARGN} DESTINATION include/gearbox/${install_subdir} )
ENDMACRO( GBX_ADD_HEADERS install_subdir )

#
# GBX_ADD_EXAMPLE( install_subdir makefile [FILE0 FILE1 FILE2 ...] )
#
# Specialisation of INSTALL(FILES ...) for GearBox project to to install examples.
# All files are installed into PREFIX/share/gearbox/${install_subdir}.
# makefile is passed through CONFIGURE_FILE to add in correct include and library
# paths based on the install prefix.
#
MACRO( GBX_ADD_EXAMPLE install_subdir makefile )
    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/${makefile} ${CMAKE_CURRENT_BINARY_DIR}/${makefile} @ONLY)
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/${makefile} DESTINATION share/gearbox/${install_subdir} RENAME CMakeLists.txt )
    INSTALL( FILES ${ARGN} DESTINATION share/gearbox/${install_subdir} )
ENDMACRO( GBX_ADD_EXAMPLE install_subdir makefile )


#
# This is a mechanism to register special items which are not
# components or libraries. This function only records the name of
# the item to display it at the end of the cmake run and to submit
# to the Dashboard.
# Usage: GBX_ADD_ITEM( name )
#
MACRO( GBX_ADD_ITEM name )
    SET( templist ${ITEM_LIST} )
    LIST ( APPEND templist ${name} )
    SET( ITEM_LIST ${templist} CACHE INTERNAL "Global list of special items to build" FORCE )
    MESSAGE( STATUS "Planning to Build Item      : ${name}" )
ENDMACRO( GBX_ADD_ITEM name )

#
# This is a mechanism to specify a license for the current source directory.
# Usage: GBX_ADD_LICENSE( license )
#
MACRO( GBX_ADD_LICENSE license )
    SET( templist ${LICENSE_LIST} )

    # get relative path to the current source dir
    STRING ( LENGTH ${GBX_PROJECT_SOURCE_DIR} proj_src_dir_length )
    STRING ( LENGTH ${CMAKE_CURRENT_SOURCE_DIR} current_src_dir_length )
    MATH ( EXPR relative_path_length "${current_src_dir_length} - ${proj_src_dir_length} - 1" )
    MATH ( EXPR relative_path_start "${proj_src_dir_length} + 1" )
    STRING ( SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR}
        ${relative_path_start} ${relative_path_length} current_src_dir_relative )

    # format the string to line up properly
    SET( spaces "A                                                                                       Z" )
    STRING ( LENGTH ${current_src_dir_relative} current_src_dir_relative_length )
    MATH ( EXPR white_space_length "60 - ${current_src_dir_relative_length}" )
    STRING ( SUBSTRING ${spaces} 1 ${white_space_length} white_space )

    SET( line_item "${current_src_dir_relative}${white_space}${license}" )
    LIST ( APPEND templist ${line_item} )
    SET( LICENSE_LIST ${templist} CACHE INTERNAL "Global list of directories and their licenses" FORCE )
#     MESSAGE( STATUS ${line_item} )
ENDMACRO( GBX_ADD_LICENSE license )

#
# Usage: GBX_ADD_TEST( testname Exename arg1 arg2 ... )
# Example:  GBX_ADD_TEST( IntegerTest inttest --verbose )
#
MACRO( GBX_ADD_TEST name EXE )
    ADD_TEST( ${name} ${EXE} ${ARGN} )
    SET( templist ${TEST_LIST} )
    LIST ( APPEND templist ${name} )
    SET( TEST_LIST ${templist} CACHE INTERNAL "Global list of (CTest) tests to build" FORCE )
#     MESSAGE( STATUS "Planning to Build Test      : ${name}" )
ENDMACRO( GBX_ADD_TEST name EXE )

#
# Usage: GBX_NOT_ADD_EXECUTABLE( name reason )
#
MACRO( GBX_NOT_ADD_EXECUTABLE name reason )
  SET( templist ${COMPONENT_NOT_LIST} )
  LIST ( APPEND templist ${name} )
#  MESSAGE ( STATUS "DEBUG: ${templist}" )
  SET( COMPONENT_NOT_LIST ${templist} CACHE INTERNAL "Global list of components NOT to build" FORCE )
  MESSAGE( STATUS "Not planning to Build Executable : ${name} because ${reason}" )
ENDMACRO( GBX_NOT_ADD_EXECUTABLE name reason )

#
# Usage: GBX_NOT_ADD_LIBRARY( name reason )
#
MACRO( GBX_NOT_ADD_LIBRARY name reason )
  SET( templist ${LIBRARY_NOT_LIST} )
  LIST ( APPEND templist ${name} )
#  MESSAGE ( STATUS "DEBUG: ${templist}" )
  SET( LIBRARY_NOT_LIST ${templist} CACHE INTERNAL "Global list of libraries NOT to build" FORCE )
  MESSAGE( STATUS "Not planning to Build Library   : ${name} because ${reason}" )
ENDMACRO( GBX_NOT_ADD_LIBRARY name reason )

#
# Prints out list information: size, and items.
# Prints nothing if list is empty.
# Example: LIST_REPORT( COMPONENT_LIST "component(s)" )
#
# Tricky list stuff.
# see http://www.cmake.org/Wiki/CMakeMacroMerge for an example
#
MACRO ( LIST_REPORT ACTION ITEM_NAME note L )
    SET( templist ${L} )
    LIST ( LENGTH templist templist_length )
    SET( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )

    IF ( templist_length GREATER 0 )
        LIST ( SORT templist )

        MESSAGE ( STATUS "${ACTION} ${templist_length} ${ITEM_NAME} ${note}:" )
        MESSAGE ( STATUS "    ${templist}" )

        WRITE_FILE ( ${report_file} "${ACTION} ${templist_length} ${ITEM_NAME}:" APPEND )
        WRITE_FILE ( ${report_file} "    ${templist}" APPEND )
    ENDIF ( templist_length GREATER 0 )
ENDMACRO ( LIST_REPORT ACTION ITEM_NAME note L )

#
# Puts messages on the screen.
# Writes to a text file.
#
MACRO( GBX_CONFIG_REPORT )

    MESSAGE( STATUS "== SUMMARY ==" )

    # write configuration results to file (this line clears existing contents)
    SET( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )
    WRITE_FILE ( ${report_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
#     WRITE_FILE ( ${report_file} "Using Ice version ${ICE_VERSION}" )

    #
    # Print some results
    #
    MESSAGE ( STATUS "Project name      ${PROJECT_NAME}")
    MESSAGE ( STATUS "Project version   ${GBX_PROJECT_VERSION}")
    # would be nice to print out Orca version for satellite projects
    # for this we need an executable which is guaranteed to be installed.
    # then we can run it with --version flag.
    # IF ( NOT ORCA_MOTHERSHIP )
    #     MESSAGE ( STATUS "Using Orca version ${ORCA_VERSION}")
    # ENDIF ( NOT ORCA_MOTHERSHIP )
    MESSAGE ( STATUS "Platform          ${CMAKE_SYSTEM}")
    MESSAGE ( STATUS "CMake version     ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}-patch ${CMAKE_PATCH_VERSION}")
    MESSAGE ( STATUS "Install dir       ${CMAKE_INSTALL_PREFIX}")

    SET( note " " )
    LIST_REPORT ( "Will build" "executables" ${note} "${COMPONENT_LIST}" )
    LIST_REPORT ( "Will build" "libraries" ${note} "${LIBRARY_LIST}" )
    LIST_REPORT ( "Will build" "CTest tests" ${note} "${TEST_LIST}" )
    LIST_REPORT ( "Will build" "special items" ${note} "${ITEM_LIST}" )

    SET( note "(see above for reasons)" )
    LIST_REPORT ( "Will NOT build" "executables" ${note} "${COMPONENT_NOT_LIST}" )
    LIST_REPORT ( "Will NOT build" "libraries" ${note} "${LIBRARY_NOT_LIST}" )

ENDMACRO( GBX_CONFIG_REPORT )

MACRO ( GBX_WRITE_MANIFEST )
    SET( manifest_file ${GBX_PROJECT_BINARY_DIR}/${PROJECT_NAME}_manifest.cmake )
    WRITE_FILE ( ${manifest_file} "\# Autogenerated by CMake for ${PROJECT_NAME} project" )

    FOREACH( A ${LIBRARY_LIST} )
        STRING ( TOUPPER ${A} UPPERA )
        WRITE_FILE ( ${manifest_file} "SET( ${UPPERA}_INSTALLED 1)" APPEND )
    ENDFOREACH( A ${LIBRARY_LIST} )

    FOREACH( A ${LIBRARY_NOT_LIST} )
        STRING ( TOUPPER ${A} UPPERA )
        WRITE_FILE ( ${manifest_file} "SET( ${UPPERA}_INSTALLED 0)" APPEND )
    ENDFOREACH( A ${LIBRARY_NOT_LIST} )

    WRITE_FILE ( ${manifest_file} " " APPEND )

    STRING ( TOUPPER ${PROJECT_NAME} upper_project_name )
    WRITE_FILE ( ${manifest_file} "SET( ${upper_project_name}_MANIFEST_LOADED 1)" APPEND )

    INSTALL( FILES ${manifest_file} DESTINATION . )
ENDMACRO ( GBX_WRITE_MANIFEST )

MACRO ( GBX_WRITE_LICENSE )
    SET( license_file ${GBX_PROJECT_SOURCE_DIR}/LICENSE )
    WRITE_FILE ( ${license_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
    WRITE_FILE ( ${license_file} "----------------------------------------------------------------------" APPEND )
    WRITE_FILE ( ${license_file} "DIRECTORY                                                   license" APPEND )
    WRITE_FILE ( ${license_file} "----------------------------------------------------------------------" APPEND )

    FOREACH( A ${LICENSE_LIST} )
        WRITE_FILE ( ${license_file} ${A} APPEND )
    ENDFOREACH( A ${LICENSE_LIST} )

ENDMACRO ( GBX_WRITE_LICENSE )

#
# Reset global lists of components, libraries, etc.
#
MACRO ( GBX_RESET_ALL_LISTS )
    # MESSAGE ( STATUS "DEBUG: Resetting global component and library lists" )
    SET( COMPONENT_LIST    "" CACHE INTERNAL "Global list of components to build" FORCE )
    SET( LIBRARY_LIST      "" CACHE INTERNAL "Global list of libraries to build" FORCE )
    SET( TEST_LIST         "" CACHE INTERNAL "Global list of CTest tests to build" FORCE )
    SET( ITEM_LIST         "" CACHE INTERNAL "Global list of special items to build" FORCE )

    SET( COMPONENT_NOT_LIST  "" CACHE INTERNAL "Global list of components NOT to build" FORCE )
    SET( LIBRARY_NOT_LIST    "" CACHE INTERNAL "Global list of libraries NOT to build" FORCE )

    SET( LICENSE_LIST      "" CACHE INTERNAL "Global list of directories and their licenses" FORCE )
ENDMACRO ( GBX_RESET_ALL_LISTS )
