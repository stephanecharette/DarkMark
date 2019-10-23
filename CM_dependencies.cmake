# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FIND_PACKAGE ( Threads			REQUIRED		)
FIND_PACKAGE ( OpenCV			REQUIRED		)
FIND_LIBRARY ( XEXT			Xext			)
FIND_LIBRARY ( DARKHELP			darkhelp		)
FIND_LIBRARY ( DARKNET			darknet			)
FIND_PACKAGE ( GTest			QUIET			)

SET ( DM_LIBRARIES ${DARKHELP} ${DARKNET} ${XEXT} ${OpenCV_LIBS} )
