# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_SOURCE *.cpp )
LIST ( SORT DM_SOURCE )

SET (APP_WIN32_ICON "")
IF (WIN32)
	SET (APP_WIN32_ICON "${CMAKE_SOURCE_DIR}/src-windows/appicon.rc")
ENDIF ()

ADD_EXECUTABLE ( DarkMark
			WIN32
			${GUI_TYPE}
			${DM_SOURCE}
			$<TARGET_OBJECTS:dm_tools>
			$<TARGET_OBJECTS:dm_darknet>
			$<TARGET_OBJECTS:dm_darkmark>
			$<TARGET_OBJECTS:dm_launcher>
			$<TARGET_OBJECTS:dm_classid>
			$<TARGET_OBJECTS:dm_wnd>
			${APP_WIN32_ICON}
			)

TARGET_LINK_LIBRARIES ( DarkMark PRIVATE dm_juce ${DM_LIBRARIES} )

INSTALL ( TARGETS DarkMark RUNTIME DESTINATION bin )
