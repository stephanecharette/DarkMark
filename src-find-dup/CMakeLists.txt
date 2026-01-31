# DarkMark (C) 2019-2026 Stephane Charette <stephanecharette@gmail.com>

FILE ( GLOB FIND_DUP_SOURCE *.cpp )
LIST ( SORT FIND_DUP_SOURCE )

ADD_EXECUTABLE ( DarkMark_find_duplicates ${FIND_DUP_SOURCE} )

TARGET_LINK_LIBRARIES ( DarkMark_find_duplicates PRIVATE dm_juce ${DM_LIBRARIES} )

IF (APPLE)
	# Mac bundle details and rpath policy
	SET_TARGET_PROPERTIES (DarkMark_find_duplicates PROPERTIES
		SKIP_BUILD_RPATH			OFF
		BUILD_WITH_INSTALL_RPATH	ON
		# search brew and usr-local at runtime
		INSTALL_RPATH				"/opt/homebrew/lib;/usr/local/lib"
	)

	SET_TARGET_PROPERTIES (DarkMark_find_duplicates PROPERTIES
		MACOSX_BUNDLE_BUNDLE_NAME			"DarkMark"
		MACOSX_BUNDLE_GUI_IDENTIFIER		"ca.ccoderun.DarkMark"
		MACOSX_BUNDLE_ICON_FILE				"darkmark.icns"
		MACOSX_BUNDLE_SHORT_VERSION_STRING	"${DM_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT				"Copyright (c) 2026 Stephane Charette"
	)

	INSTALL ( TARGETS DarkMark_find_duplicates
		BUNDLE DESTINATION	.	COMPONENT Runtime
		RUNTIME DESTINATION	bin	COMPONENT Runtime
	)
ELSE ()
	INSTALL ( TARGETS DarkMark_find_duplicates RUNTIME DESTINATION bin )
ENDIF ()
