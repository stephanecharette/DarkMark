# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$


SET ( CPACK_PACKAGE_VENDOR				"Stephane Charette"				)
SET ( CPACK_PACKAGE_CONTACT				"stephanecharette@gmail.com"	)
SET ( CPACK_PACKAGE_VERSION				${DM_VERSION}					)
SET ( CPACK_PACKAGE_VERSION_MAJOR		${DM_VER_MAJOR}					)
SET ( CPACK_PACKAGE_VERSION_MINOR		${DM_VER_MINOR}					)
SET ( CPACK_PACKAGE_VERSION_PATCH		${DM_VER_PATCH}					)
#SET ( CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}/license.txt )
SET ( CPACK_PACKAGE_NAME				"darkmark"		)
SET ( CPACK_PACKAGE_DESCRIPTION_SUMMARY	"darkmark"		)
SET ( CPACK_PACKAGE_DESCRIPTION			"darkmark"		)

INCLUDE ( CM_package_ubuntu.cmake )

SET ( CPACK_SOURCE_IGNORE_FILES	".svn" ".kdev4" "build" )
SET ( CPACK_SOURCE_GENERATOR	"TGZ;ZIP" )

INCLUDE( CPack )
