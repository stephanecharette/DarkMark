# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


# Get the kernel name.  This should give us a string such as "Linux".
EXECUTE_PROCESS (
	COMMAND uname -s
	OUTPUT_VARIABLE DM_UNAME_S
	OUTPUT_STRIP_TRAILING_WHITESPACE )

# Get the machine hardware name.  This should give us a string such as "x86_64" or "aarch64" (Jetson Nano for example).
EXECUTE_PROCESS (
	COMMAND uname -m
	OUTPUT_VARIABLE DM_UNAME_M
	OUTPUT_STRIP_TRAILING_WHITESPACE )

# Get the name "Ubuntu".
EXECUTE_PROCESS (
	COMMAND lsb_release --id
	COMMAND cut -d\t -f2
	OUTPUT_VARIABLE DM_LSB_ID
	OUTPUT_STRIP_TRAILING_WHITESPACE )

# Get the version number "20.04".
EXECUTE_PROCESS (
	COMMAND lsb_release --release
	COMMAND cut -d\t -f2
	OUTPUT_VARIABLE DM_LSB_REL
	OUTPUT_STRIP_TRAILING_WHITESPACE )

SET ( CPACK_PACKAGE_VENDOR				"Stephane Charette"				)
SET ( CPACK_PACKAGE_CONTACT				"stephanecharette@gmail.com"	)
SET ( CPACK_PACKAGE_VERSION				${DM_VERSION}					)
SET ( CPACK_PACKAGE_VERSION_MAJOR		${DM_VER_MAJOR}					)
SET ( CPACK_PACKAGE_VERSION_MINOR		${DM_VER_MINOR}					)
SET ( CPACK_PACKAGE_VERSION_PATCH		${DM_VER_PATCH}					)
#SET ( CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}/license.txt )
SET ( CPACK_PACKAGE_NAME				"darkmark"		)
SET ( CPACK_PACKAGE_FILE_NAME			"darkmark-${DM_VERSION}-${DM_UNAME_S}-${DM_UNAME_M}-${DM_LSB_ID}-${DM_LSB_REL}" )
SET ( CPACK_PACKAGE_DESCRIPTION_SUMMARY	"darkmark"		)
SET ( CPACK_PACKAGE_DESCRIPTION			"darkmark"		)

INCLUDE ( CM_package_ubuntu.cmake )

SET ( CPACK_SOURCE_IGNORE_FILES	".svn" ".kdev4" "build/" "build_and_upload" )
SET ( CPACK_SOURCE_GENERATOR	"TGZ;ZIP" )

INCLUDE( CPack )
