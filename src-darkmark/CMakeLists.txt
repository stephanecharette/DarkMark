# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_DARKMARK_SOURCE *.cpp	)
LIST ( SORT DM_DARKMARK_SOURCE			)

ADD_LIBRARY ( dm_darkmark OBJECT ${DM_DARKMARK_SOURCE} )

IF (APPLE)
	INCLUDE_DIRECTORIES ( "/opt/homebrew/include" )
ENDIF ()
