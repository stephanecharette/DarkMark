# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


IF (WIN32)
	MESSAGE("Packaging for Windows requires the NSIS tools to be installed.")
ELSE ()
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
ENDIF ()

SET (CPACK_PACKAGE_VENDOR					"Stephane Charette")
SET (CPACK_PACKAGE_CONTACT					"stephanecharette@gmail.com")
SET (CPACK_PACKAGE_VERSION					${DM_VERSION})
SET (CPACK_PACKAGE_VERSION_MAJOR			${DM_VER_MAJOR})
SET (CPACK_PACKAGE_VERSION_MINOR			${DM_VER_MINOR})
SET (CPACK_PACKAGE_VERSION_PATCH			${DM_VER_PATCH})
SET (CPACK_PACKAGE_NAME						"darkmark")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY		"darkmark")
SET (CPACK_PACKAGE_DESCRIPTION				"darkmark")

IF (WIN32)
	SET (CPACK_PACKAGE_FILE_NAME					"darkmark-${DM_VERSION}-Windows-64")
	SET (CPACK_NSIS_PACKAGE_NAME					"DarkMark")
	SET (CPACK_NSIS_DISPLAY_NAME					"DarkMark v${DM_VERSION}")
	SET (CPACK_NSIS_MUI_ICON						"${CMAKE_CURRENT_SOURCE_DIR}\\\\src-windows\\\\darkmark_logo.ico")
	SET (CPACK_NSIS_MUI_UNIICON						"${CMAKE_CURRENT_SOURCE_DIR}\\\\src-windows\\\\darkmark_logo.ico")
	SET (CPACK_PACKAGE_ICON							"${CMAKE_CURRENT_SOURCE_DIR}\\\\src-windows\\\\darkmark_logo.ico")
	SET (CPACK_NSIS_CONTACT							"stephanecharette@gmail.com")
	SET (CPACK_NSIS_URL_INFO_ABOUT					"https://www.ccoderun.ca/DarkMark/")
	SET (CPACK_NSIS_HELP_LINK						"https://www.ccoderun.ca/DarkMark/")
	SET (CPACK_NSIS_MODIFY_PATH						"ON")
	SET (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL	"ON")
	SET (CPACK_NSIS_INSTALLED_ICON_NAME				"bin\\\\darkmark_logo.ico")
	SET (CPACK_GENERATOR							"NSIS")
ENDIF ()

IF (UNIX)
	SET (CPACK_PACKAGE_FILE_NAME					"darkmark-${DM_VERSION}-${DM_UNAME_S}-${DM_UNAME_M}-${DM_LSB_ID}-${DM_LSB_REL}")
	SET (CPACK_DEBIAN_PACKAGE_SECTION				"other")
	SET (CPACK_DEBIAN_PACKAGE_PRIORITY				"optional")
	SET (CPACK_DEBIAN_PACKAGE_DEPENDS				"darkhelp")
	SET (CPACK_DEBIAN_PACKAGE_MAINTAINER			"Stephane Charette <stephanecharette@gmail.com>")
	SET (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 		"${CMAKE_CURRENT_SOURCE_DIR}/src-ubuntu/postinst;${CMAKE_CURRENT_SOURCE_DIR}/src-ubuntu/postrm")
	SET (CPACK_GENERATOR							"DEB")
ENDIF ()

IF (APPLE)
	SET (CPACK_PACKAGE_FILE_NAME					"darkmark-${DM_VERSION}-macOS-${CMAKE_SYSTEM_PROCESSOR}")
	SET (CPACK_GENERATOR							"DragNDrop")
	SET (CPACK_DMG_VOLUME_NAME						"DarkMark ${DM_VERSION}")
	SET (CPACK_DMG_FORMAT							"UDZO")
	SET (CPACK_PACKAGE_ICON							"${CMAKE_CURRENT_SOURCE_DIR}/src-macos/darkmark.icns")
	SET (CPACK_RESOURCE_FILE_LICENSE				"${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
ENDIF ()


SET (CPACK_SOURCE_IGNORE_FILES	".git" ".kdev4" "build/" "build_and_upload")
SET (CPACK_SOURCE_GENERATOR	"TGZ;ZIP")

INCLUDE (CPack)
