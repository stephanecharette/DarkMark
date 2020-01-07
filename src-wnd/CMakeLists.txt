# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FILE ( GLOB DARKMARK_WND_SOURCE *.cpp	)
LIST ( SORT DARKMARK_WND_SOURCE			)

ADD_LIBRARY ( dm_wnd OBJECT ${DARKMARK_WND_SOURCE} )
