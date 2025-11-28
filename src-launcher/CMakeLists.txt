# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_LAUNCHER_SOURCE *.cpp	)
LIST ( SORT DM_LAUNCHER_SOURCE			)

ADD_LIBRARY ( dm_launcher OBJECT ${DM_LAUNCHER_SOURCE} )

IF (APPLE)
	INCLUDE_DIRECTORIES ( "/opt/homebrew/include" )
ENDIF ()
