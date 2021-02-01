# DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>


SET ( CPACK_DEBIAN_PACKAGE_SECTION			"other" )
SET ( CPACK_DEBIAN_PACKAGE_PRIORITY			"optional" )
SET ( CPACK_DEBIAN_PACKAGE_DEPENDS			"darkhelp" )
SET ( CPACK_DEBIAN_PACKAGE_MAINTAINER		"Stephane Charette <stephanecharette@gmail.com>" )
SET ( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 	"${CMAKE_CURRENT_SOURCE_DIR}/src-ubuntu/postinst;${CMAKE_CURRENT_SOURCE_DIR}/src-ubuntu/postrm" )
SET ( CPACK_GENERATOR						"DEB" )
