# DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>


FILE ( GLOB DM_LAUNCHER_SOURCE *.cpp	)
LIST ( SORT DM_LAUNCHER_SOURCE			)

ADD_LIBRARY ( dm_launcher OBJECT ${DM_LAUNCHER_SOURCE} )
