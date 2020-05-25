# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FILE ( GLOB DM_LAUNCHER_SOURCE *.cpp	)
LIST ( SORT DM_LAUNCHER_SOURCE			)

ADD_LIBRARY ( dm_launcher OBJECT ${DM_LAUNCHER_SOURCE} )
