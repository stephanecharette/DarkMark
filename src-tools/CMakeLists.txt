# DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DARKMARK_TOOLS_SOURCE *.cpp	)
LIST ( SORT DARKMARK_TOOLS_SOURCE		)

ADD_LIBRARY ( dm_tools OBJECT ${DARKMARK_TOOLS_SOURCE} )
