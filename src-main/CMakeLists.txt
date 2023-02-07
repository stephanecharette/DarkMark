# DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_SOURCE *.cpp )
LIST ( SORT DM_SOURCE )

IF(!WIN32)
ADD_EXECUTABLE ( DarkMark
			${GUI_TYPE}
			${DM_SOURCE}
			$<TARGET_OBJECTS:dm_tools>
			$<TARGET_OBJECTS:dm_darknet>
			$<TARGET_OBJECTS:dm_darkmark>
			$<TARGET_OBJECTS:dm_launcher>
			$<TARGET_OBJECTS:dm_wnd>
			)
ELSE()
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/src-windows/appicon.rc")

ADD_EXECUTABLE ( DarkMark
			WIN32
			${GUI_TYPE}
			${DM_SOURCE}
			$<TARGET_OBJECTS:dm_tools>
			$<TARGET_OBJECTS:dm_darknet>
			$<TARGET_OBJECTS:dm_darkmark>
			$<TARGET_OBJECTS:dm_launcher>
			$<TARGET_OBJECTS:dm_wnd>
			${APP_ICON_RESOURCE_WINDOWS}
			)
ENDIF()

TARGET_LINK_LIBRARIES ( DarkMark PRIVATE dm_juce ${DM_LIBRARIES} )

INSTALL ( TARGETS DarkMark RUNTIME DESTINATION bin )
