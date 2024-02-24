# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_CLASSID_SOURCE *.cpp	)
LIST ( SORT DM_CLASSID_SOURCE		)

ADD_LIBRARY ( dm_classid OBJECT ${DM_CLASSID_SOURCE} )
