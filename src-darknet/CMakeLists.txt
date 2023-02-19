# DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_DARKNET_SOURCE *.cpp	)
LIST ( SORT DM_DARKNET_SOURCE		)

ADD_LIBRARY ( dm_darknet OBJECT ${DM_DARKNET_SOURCE} )
