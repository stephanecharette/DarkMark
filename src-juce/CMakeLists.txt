# DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>


IF (WIN32 OR APPLE)
	SET ( RT "" )
	SET ( DL "" )
	SET ( X11 "" )
ELSE ()
	FIND_LIBRARY	( RT		rt			)
	FIND_LIBRARY	( DL		dl			)
	FIND_PACKAGE	( X11		REQUIRED	)
ENDIF ()
FIND_PACKAGE	( Threads	REQUIRED	)
FIND_PACKAGE	( Freetype	REQUIRED	)

# Adopt this if JUCE_USE_FONTCONFIG=1 for UNIX needed:
# FIND_PACKAGE(PkgConfig REQUIRED)
# PKG_CHECK_MODULES(FONTCONFIG REQUIRED fontconfig)
# LINK_DIRECTORIES(${FONTCONFIG_LIBRARY_DIRS})

INCLUDE_DIRECTORIES ( AFTER ${FREETYPE_INCLUDE_DIRS} )

# Adopt this if JUCE_USE_FONTCONFIG=1 for UNIX needed:
# SET ( JUCE_LIBRARIES ${RT} ${DL} ${X11_LIBRARIES} ${FREETYPE_LIBRARIES} ${GTK3_LIBRARIES} ${FONTCONFIG_LIBRARIES} )
SET ( JUCE_LIBRARIES ${RT} ${DL} ${X11_LIBRARIES} ${FREETYPE_LIBRARIES} ${GTK3_LIBRARIES} )

# JUCE C++ 
FILE ( GLOB JUCE_CPP_SOURCE ${CMAKE_HOME_DIRECTORY}/JuceLibraryCode/*.cpp )
LIST (SORT JUCE_CPP_SOURCE)

# SheenBidi C sources
FILE ( GLOB JUCE_SHEENBIDI_SOURCE ${CMAKE_HOME_DIRECTORY}/JuceLibraryCode/modules/juce_graphics/unicode/sheenbidi/Source/*.c )
LIST (SORT JUCE_SHEENBIDI_SOURCE)

if (APPLE)
    # JUCE Objective-C++ module files
    FILE ( GLOB JUCE_MM_SOURCE ${CMAKE_HOME_DIRECTORY}/JuceLibraryCode/*.mm )
    LIST (SORT JUCE_MM_SOURCE)

    SET_SOURCE_FILES_PROPERTIES ( ${JUCE_MM_SOURCE} ${JUCE_CPP_SOURCE} PROPERTIES LANGUAGE OBJCXX )
ENDIF ()

ADD_LIBRARY( dm_juce STATIC ${JUCE_MM_SOURCE} ${JUCE_CPP_SOURCE} ${JUCE_SHEENBIDI_SOURCE} )

# SILENCE SHEENBIDI WARNINGS
IF (NOT MSVC AND NOT APPLE)
    # TARGET_COMPILE_OPTIONS( dm_juce PRIVATE
    #     -Wno-stringop-overflow
    #     -Wno-maybe-uninitialized
    #     -Wno-unused-variable
    # )
    SET_SOURCE_FILES_PROPERTIES ( ${JUCE_SHEENBIDI_SOURCE} PROPERTIES
        COMPILE_OPTIONS "-Wno-stringop-overflow;-Wno-maybe-uninitialized;-Wno-unused-variable"
    )
ENDIF ()

TARGET_LINK_LIBRARIES ( dm_juce ${CMAKE_THREAD_LIBS_INIT} ${JUCE_LIBRARIES} )

IF (APPLE)
    TARGET_LINK_LIBRARIES ( dm_juce
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreText"
        "-framework Foundation"
        "-framework AppKit"
        "-framework IOKit"
        "-framework QuartzCore"
        "-framework ApplicationServices"
        "-framework Security"
    )
ENDIF ()