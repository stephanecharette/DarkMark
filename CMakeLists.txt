# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

CMAKE_MINIMUM_REQUIRED ( VERSION 3.10 )

PROJECT ( DarkMark LANGUAGES C CXX )

IF (APPLE)
	ENABLE_LANGUAGE( OBJC )
    ENABLE_LANGUAGE( OBJCXX )
	# ADD_DEFINITIONS( -DJUCE_DISABLE_CORETEXT_SB_SHAPER=1 )
ENDIF ()

IF ( NOT CMAKE_BUILD_TYPE )
	SET ( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
ENDIF ()


INCLUDE ( CM_version.cmake		)	# get the version number
INCLUDE ( CM_definitions.cmake	)	# compiler definitions
INCLUDE ( CM_dependencies.cmake	)	# find all required dependencies
INCLUDE ( CM_source.cmake		)	# source code that needs to be built
INCLUDE ( CM_package.cmake		)	# create .exe or .deb packages

IF ( GTest_FOUND )
	ENABLE_TESTING ()
	ADD_SUBDIRECTORY ( src-test )
ENDIF ()
