# DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
# $Id$


SET ( CMAKE_CXX_STANDARD 17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED ON )

# cannot add the other flags until JUCE has been built
ADD_DEFINITIONS ( "-Wall" ) # -Wextra -Werror -Wno-unused-parameter" )


# *******************************************************************************************************************
# TLDR: *** Unless you are specifically working on CSRT I suggest that this option remain disabled (set to zero). ***
# *******************************************************************************************************************
#
# In June 2020 I added the ability to use OpenCV's CSRT object tracker in DarkMark.  This means that prior to a neural
# network having been trained, OpenCV could be used to track an object between video frames, more quickly marking up
# images, and getting a proper trained network up and running.
#
# While this worked to a certain degree, it doesn't work well enough to really facilitate marking up video frames.  Over
# the span of 2-3 frames, the CSRT RoI rectangle grows too much, and tracking is trivial at best.  If the video frame is
# a single object moving, then the results are better.  But then with a single object, it woudln't be a burden to mark up
# images anyway.
#
# Originally, I was hoping to use CSRT object tracking to help mark up downtown city traffic images from a dash cam with
# lots of vehicles, pedestrians, bicycles, etc.  But for certain it isn't good enough to help in that scenario.
#
# This test was done on Ubuntu 20.04, with OpenCV v4.2.0.  While the code works, it is incomplete and has limitations.
# I've left the code in DarkMark for future use, tests, and for me to keep working on it.  But I've commented it out
# since I don't feel it is ready for use.
#
ADD_DEFINITIONS(DARKMARK_ENABLE_OPENCV_CSRT_TRACKER=0)
