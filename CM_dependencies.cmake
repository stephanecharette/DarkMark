# DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>


FIND_PACKAGE ( Threads			REQUIRED	)
FIND_PACKAGE ( OpenCV	CONFIG	REQUIRED	)

IF (WIN32)
	# Assume that vcpkg was used on Windows
	FIND_PACKAGE (Darknet REQUIRED)
	INCLUDE_DIRECTORIES (${Darknet_INCLUDE_DIR})
	SET (Darknet Darknet::dark)
	FIND_LIBRARY ( DARKHELP darkhelp REQUIRED HINTS  ../DarkHelp/build/src-lib/Release )
	INCLUDE_DIRECTORIES ( ../DarkHelp/src-lib )
	SET (LIBMAGIC "")
ELSE ()
	FIND_LIBRARY ( DARKNET			darknet		)
	FIND_LIBRARY ( DARKHELP			darkhelp	)
	FIND_LIBRARY ( LIBMAGIC			magic		) # sudo apt-get install libmagic-dev
	FIND_PACKAGE ( GTest			QUIET		)
ENDIF ()


SET ( DM_LIBRARIES Threads::Threads ${DARKHELP} ${DARKNET} ${OpenCV_LIBS} ${LIBMAGIC} )

INCLUDE_DIRECTORIES ( ${OpenCV_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${Darknet_INCLUDE_DIR} )

