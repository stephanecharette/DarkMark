# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FIND_PACKAGE ( Threads			REQUIRED	)
FIND_PACKAGE ( OpenCV	CONFIG	REQUIRED	)
FIND_PACKAGE ( Darknet	CONFIG	REQUIRED	)
FIND_LIBRARY ( DARKHELP			darkhelp	)
FIND_LIBRARY ( LIBMAGIC			magic		) # sudo apt-get install libmagic-dev
FIND_PACKAGE ( GTest			QUIET		)

SET ( DM_LIBRARIES Threads::Threads ${DARKHELP} Darknet::dark ${OpenCV_LIBS} ${LIBMAGIC} )

INCLUDE_DIRECTORIES ( ${OpenCV_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${Darknet_INCLUDE_DIR} )

