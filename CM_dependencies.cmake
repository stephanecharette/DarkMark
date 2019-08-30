# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$


FIND_PACKAGE ( Threads			REQUIRED		)
FIND_PACKAGE ( OpenCV			REQUIRED		)
FIND_LIBRARY ( VZIMAGINATION	vzimagination	)
FIND_LIBRARY ( DARKHELP			darkhelp		)
FIND_LIBRARY ( DARKNET			darknet			)

SET ( DM_LIBRARIES ${VZIMAGINATION} ${DARKHELP} ${DARKNET} ${OpenCV_LIBS} )
