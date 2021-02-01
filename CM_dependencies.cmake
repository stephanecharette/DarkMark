# DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>


FIND_PACKAGE ( Threads			REQUIRED	)
FIND_PACKAGE ( OpenCV	CONFIG	REQUIRED	)
FIND_LIBRARY ( DARKNET			darknet		)
FIND_LIBRARY ( DARKHELP			darkhelp	)
FIND_LIBRARY ( LIBMAGIC			magic		) # sudo apt-get install libmagic-dev
FIND_PACKAGE ( GTest			QUIET		)

SET ( DM_LIBRARIES Threads::Threads ${DARKHELP} ${DARKNET} ${OpenCV_LIBS} ${LIBMAGIC} )

INCLUDE_DIRECTORIES ( ${OpenCV_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${Darknet_INCLUDE_DIR} )

