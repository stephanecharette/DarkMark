# DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>


SET ( CMAKE_CXX_STANDARD 17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED ON )

IF (WIN32)
	ADD_COMPILE_OPTIONS ( /W4 )				# warning level (high)
	ADD_COMPILE_OPTIONS ( /WX )				# treat warnings as errors
	ADD_COMPILE_OPTIONS ( /permissive- )	# stick to C++ standards (turn off Microsoft-specific extensions)
	ADD_COMPILE_OPTIONS ( /wd4100 )			# disable "unreferenced formal parameter"
	ADD_COMPILE_DEFINITIONS ( _CRT_SECURE_NO_WARNINGS )	# don't complain about localtime()
	SET ( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )
ELSE ()
	# cannot add the other flags until JUCE has been built
	ADD_DEFINITIONS ( "-Wall" ) # -Wextra -Werror -Wno-unused-parameter" )
ENDIF ()
