# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FILE ( GLOB DM_SOURCE *.cpp )
LIST ( SORT DM_SOURCE )

ADD_EXECUTABLE ( DarkMark
			${GUI_TYPE}
			${DM_SOURCE}
			$<TARGET_OBJECTS:dm_tools>
			$<TARGET_OBJECTS:dm_wnd>
			)

TARGET_LINK_LIBRARIES ( DarkMark dm_juce ${CMAKE_THREAD_LIBS_INIT} ${DM_LIBRARIES} )

INSTALL ( TARGETS DarkMark RUNTIME DESTINATION bin )
