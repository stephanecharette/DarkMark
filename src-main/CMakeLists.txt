# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FILE ( GLOB DM_SOURCE *.cpp )
LIST ( SORT DM_SOURCE )

ADD_EXECUTABLE ( DarkMark
			${GUI_TYPE}
			${DM_SOURCE}
			$<TARGET_OBJECTS:dm_tools>
			$<TARGET_OBJECTS:dm_darknet>
			$<TARGET_OBJECTS:dm_darkmark>
			$<TARGET_OBJECTS:dm_launcher>
			$<TARGET_OBJECTS:dm_wnd>
			)

TARGET_LINK_LIBRARIES ( DarkMark PRIVATE dm_juce ${DM_LIBRARIES} )

INSTALL ( TARGETS DarkMark RUNTIME DESTINATION bin )
