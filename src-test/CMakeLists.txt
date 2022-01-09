# DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>


#FILE ( GLOB TEST_SOURCE *.cpp		)
#LIST ( SORT TEST_SOURCE				)
#
#ADD_EXECUTABLE ( darkmark_tests ${TEST_SOURCE}
#			$<TARGET_OBJECTS:dm_tools>
#			$<TARGET_OBJECTS:dm_darknet>
#			$<TARGET_OBJECTS:dm_launcher>
#			$<TARGET_OBJECTS:dm_wnd>
#			)
#TARGET_LINK_LIBRARIES ( darkmark_tests dm_juce ${CMAKE_THREAD_LIBS_INIT} ${GTEST_LIBRARIES} ${DM_LIBRARIES} pthread )
#
#ADD_TEST ( NAME darkmark_tests COMMAND darkmark_tests )
