# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

FILE ( GLOB DM_SOURCE *.cpp )
LIST ( SORT DM_SOURCE )

SET (EXTRA_SRC_FILES "")
SET (OS_BUNDLE_ARG "")

IF (WIN32)
    SET (EXTRA_SRC_FILES "${CMAKE_SOURCE_DIR}/src-windows/appicon.rc")
    SET (OS_BUNDLE_ARG "WIN32")
ELSEIF (APPLE)
    SET (EXTRA_SRC_FILES "${CMAKE_SOURCE_DIR}/src-macos/darkmark.icns")
    SET (OS_BUNDLE_ARG "MACOSX_BUNDLE")
    SET_SOURCE_FILES_PROPERTIES(${EXTRA_SRC_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF ()

ADD_EXECUTABLE ( DarkMark
            ${OS_BUNDLE_ARG}
            ${GUI_TYPE}
            ${DM_SOURCE}
            $<TARGET_OBJECTS:dm_tools>
            $<TARGET_OBJECTS:dm_darknet>
            $<TARGET_OBJECTS:dm_darkmark>
            $<TARGET_OBJECTS:dm_launcher>
            $<TARGET_OBJECTS:dm_classid>
            $<TARGET_OBJECTS:dm_wnd>
            ${EXTRA_SRC_FILES}
            )

TARGET_LINK_LIBRARIES ( DarkMark PRIVATE dm_juce ${DM_LIBRARIES} )

IF (APPLE)
    # Mac bundle details and rpath policy
    SET_TARGET_PROPERTIES (DarkMark PROPERTIES
        SKIP_BUILD_RPATH         OFF
        BUILD_WITH_INSTALL_RPATH ON
        # search brew and usr-local at runtime
        INSTALL_RPATH            "/opt/homebrew/lib;/usr/local/lib"
    )

    SET_TARGET_PROPERTIES (DarkMark PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME         	"DarkMark"
        MACOSX_BUNDLE_GUI_IDENTIFIER      	"ca.ccoderun.DarkMark"
        MACOSX_BUNDLE_ICON_FILE           	"darkmark.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING 	"${DM_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT           	"Copyright (c) 2025 Stephane Charette"
    )
ENDIF ()

IF (APPLE)
	INSTALL ( TARGETS DarkMark
		BUNDLE DESTINATION .   COMPONENT Runtime
		RUNTIME DESTINATION bin COMPONENT Runtime
	)
ELSE ()
	INSTALL ( TARGETS DarkMark RUNTIME DESTINATION bin )
ENDIF ()
