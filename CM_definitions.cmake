# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


SET ( CMAKE_CXX_STANDARD 17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED ON )

# cannot add the other flags until JUCE has been built
ADD_DEFINITIONS ( "-Wall" ) # -Wextra -Werror -Wno-unused-parameter" )
