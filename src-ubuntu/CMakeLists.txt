# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$

CONFIGURE_FILE ( ccoderun-darkmark.desktop.in ccoderun-darkmark.desktop)

INSTALL ( FILES ${CMAKE_CURRENT_BINARY_DIR}/ccoderun-darkmark.desktop	DESTINATION /usr/share/applications/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_256.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_128.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_064.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_048.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_032.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_022.png		DESTINATION /usr/share/doc/DarkMark/ )
INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkmark_logo_016.png		DESTINATION /usr/share/doc/DarkMark/ )
