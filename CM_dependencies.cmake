# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


FIND_PACKAGE ( GTest			QUIET		)
FIND_PACKAGE ( Threads			REQUIRED	)
FIND_PACKAGE ( OpenCV	CONFIG	REQUIRED	) # sudo apt-get install libopencv-dev
FIND_LIBRARY ( DARKNET			darknet		) # https://github.com/stephanecharette/DarkHelp#building-darknet-linux
FIND_LIBRARY ( DARKHELP			darkhelp	) # https://github.com/stephanecharette/DarkHelp#building-darkhelp-linux
FIND_LIBRARY ( LIBMAGIC			magic		) # sudo apt-get install libmagic-dev
FIND_LIBRARY ( POPPLERCPP		poppler-cpp	) # sudo apt-get install libpoppler-cpp-dev

SET ( DM_LIBRARIES Threads::Threads ${DARKHELP} ${DARKNET} ${OpenCV_LIBS} ${LIBMAGIC} ${POPPLERCPP} )

INCLUDE_DIRECTORIES ( ${OpenCV_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${Darknet_INCLUDE_DIR} )
