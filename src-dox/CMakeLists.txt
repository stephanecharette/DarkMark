# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FIND_PACKAGE ( Doxygen QUIET )
CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )

IF ( Doxygen_FOUND )

	ADD_CUSTOM_TARGET (
		doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating documentation with Doxygen" VERBATIM )

ENDIF ( Doxygen_FOUND )
