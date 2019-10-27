# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# $Id$


# get rid of the directories and files automatically created by Introjucer since we're using our own CMake build files
FILE ( REMOVE_RECURSE Builds Source )

# build JUCE prior to enabling Wall Wextra Werror
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/modules )
ADD_SUBDIRECTORY ( src-juce )

ADD_DEFINITIONS ( "-Wall -Wextra -Werror -Wno-unused-parameter" )

INCLUDE_DIRECTORIES ( BEFORE src-main	)
INCLUDE_DIRECTORIES ( BEFORE src-tools	)
INCLUDE_DIRECTORIES ( BEFORE src-wnd	)

ADD_SUBDIRECTORY ( src-tools	)
ADD_SUBDIRECTORY ( src-wnd		)
ADD_SUBDIRECTORY ( src-main		)
ADD_SUBDIRECTORY ( src-dox		)
ADD_SUBDIRECTORY ( src-ubuntu	)
