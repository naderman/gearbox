#
# GBX_ASSERT( TEST COMMENT_FAIL [COMMENT_PASS=''] [IS_FATAL=FALSE] )
#
MACRO( GBX_ASSERT TEST COMMENT_FAIL )

    IF( ${TEST} )
#         MESSAGE( STATUS "DEBUG: assertion passed : ${TEST}" )

        # ARG2 holds COMMENT_PASS
        IF( ${ARGC} GREATER 2 )
            MESSAGE( STATUS ${ARGV2} )
        ENDIF( ${ARGC} GREATER 2 )

    ELSE ( ${TEST} )
#         MESSAGE( STATUS "DEBUG: assertion failed : ${TEST}" )

        SET( IS_FATAL 0 )
        IF( ${ARGC} GREATER 3 )
            SET( IS_FATAL ${ARGV3} )
        ENDIF( ${ARGC} GREATER 3 )

        IF( ${IS_FATAL} )
#             MESSAGE( STATUS "DEBUG: failure is fatal : ${IS_FATAL}" )
            MESSAGE( FATAL_ERROR ${COMMENT_FAIL} )
        ELSE ( ${IS_FATAL} )
#             MESSAGE( STATUS "DEBUG: failure is NOT fatal : ${IS_FATAL}" )
            MESSAGE( STATUS ${COMMENT_FAIL} )
        ENDIF( ${IS_FATAL} )

    ENDIF( ${TEST} )

ENDMACRO( GBX_ASSERT )
