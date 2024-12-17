# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DARKMARK_CLASSID_SOURCE *.cpp	)
LIST ( SORT DARKMARK_CLASSID_SOURCE			)

ADD_LIBRARY ( dm_classid OBJECT ${DARKMARK_CLASSID_SOURCE} )
