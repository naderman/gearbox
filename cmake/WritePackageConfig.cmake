
set( _input_dir ${PROJECT_SOURCE_DIR}/cmake/internal )
set( _output_dir ${PROJECT_BINARY_DIR} )
set( _destination lib/${PROJECT_NAME} )

# set( _input_file config-internal.cmake.in )
# set( _output_file ${PROJECT_NAME}-config-internal.cmake )
# configure_file( 
#     ${_input_dir}/${_input_file}
#     ${_output_dir}/${_output_file}
#     @ONLY )

# set( _input_file config-external.cmake.in )
set( _output_file ${PROJECT_NAME}-config.cmake )
# configure_file( 
#     ${_input_dir}/${_input_file}
#     ${_output_dir}/${_output_file}
#     @ONLY )
install( 
    FILES ${_input_dir}/${_output_file}
    DESTINATION ${_destination} )

set( _input_file ${PROJECT_NAME}-config-version.cmake.in )
set( _output_file ${PROJECT_NAME}-config-version.cmake )
configure_file( 
    ${_input_dir}/${_input_file}
    ${_output_dir}/${_output_file}
    @ONLY )
install( 
    FILES ${_output_dir}/${_output_file}
    DESTINATION ${_destination} )

# export targets
install( 
    EXPORT ${PROJECT_NAME}-targets 
#     NAMESPACE import_ 
    DESTINATION ${_destination} )

set( _input_dir )
set( _output_dir )
set( _input_file )
set( _output_file )
set( _destination )
