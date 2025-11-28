# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


SET ( CMAKE_CXX_STANDARD 17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED ON )

IF (WIN32)
	# number of sections exceeded object file format limit: compile with /bigobj
	# /bigobj can be completely removed/commented out if only release is being built
	ADD_COMPILE_OPTIONS ( /bigobj )

	ADD_COMPILE_OPTIONS ( -D__PRETTY_FUNCTION__=__FUNCSIG__ )
	ADD_COMPILE_OPTIONS ( -Dpclose=_pclose )
	ADD_COMPILE_OPTIONS ( -Dpopen=_popen )
	ADD_COMPILE_OPTIONS ( -D_USE_MATH_DEFINES )
ELSE ()
	# cannot add the other flags until JUCE has been built
	ADD_DEFINITIONS ( "-Wall" ) # -Wextra -Werror -Wno-unused-parameter" )
ENDIF ()

# equivalent to -rdynamic (to get the backtrace when something goes wrong)
SET ( CMAKE_ENABLE_EXPORTS TRUE )

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
# ADD_DEFINITIONS ( -DJUCE_DISPLAY_SPLASH_SCREEN=1 )
# ADD_DEFINITIONS ( -DJUCE_USE_DARK_SPLASH_SCREEN=0 ) 
ADD_DEFINITIONS ( -DJUCE_USE_FONTCONFIG=0)
ADD_DEFINITIONS ( -DJUCE_CATCH_UNHANDLED_EXCEPTIONS=1 )
ADD_DEFINITIONS ( -DJUCE_MODAL_LOOPS_PERMITTED=1 )
ADD_DEFINITIONS ( -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 )
