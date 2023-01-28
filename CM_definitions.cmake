# DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>


SET ( CMAKE_CXX_STANDARD 17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED ON )

# cannot add the other flags until JUCE has been built
IF (!WIN32)
	ADD_DEFINITIONS ( "-Wall" ) # -Wextra -Werror -Wno-unused-parameter" )
ENDIF()

SET ( CMAKE_ENABLE_EXPORTS TRUE )		# equivalent to -rdynamic (to get the backtrace when something goes wrong)

IF (FALSE)
	# This is part of a sponsored change to provide a simplified "darknet" window
	# with less options than normal.  Usually, this "IF" block should remain disabled.
	ADD_DEFINITIONS ( -DDARKNET_GEN_SIMPLIFIED=1 )
ENDIF ()

ADD_DEFINITIONS ( -DJUCE_USE_CURL=0 )
ADD_DEFINITIONS ( -DJUCE_WEB_BROWSER=0 )
ADD_DEFINITIONS ( -DJUCE_ENABLE_LIVE_CONSTANT_EDITOR=0 )
ADD_DEFINITIONS ( -DJUCE_STANDALONE_APPLICATION=1 )
ADD_DEFINITIONS ( -DJUCE_REPORT_APP_USAGE=1 )
ADD_DEFINITIONS ( -DJUCE_DISPLAY_SPLASH_SCREEN=1 )
ADD_DEFINITIONS ( -DJUCE_USE_DARK_SPLASH_SCREEN=0 )
ADD_DEFINITIONS ( -DJUCE_CATCH_UNHANDLED_EXCEPTIONS=1 )
ADD_DEFINITIONS ( -DJUCE_MODAL_LOOPS_PERMITTED=1 )
