# DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>


Please see the most recent documentation for DarkMark:  https://github.com/stephanecharette/DarkMark


# ------------
# PLEASE NOTE!
# ------------

Some preliminary work was done to port DarkMark to Windows in late May and
early June 2021, but this was never completed.  The following instructions
in this document will **NOT** result in a successful build.


# -------------------------
# SETTING UP PRE-REQUISITES
# -------------------------

You'll need to setup DarkHelp, Darknet, and OpenCV.
Please complete the build instructions here first:
https://github.com/stephanecharette/DarkHelp


# -----------------
# BUILDING DARKMARK
# -----------------

Assuming you already followed the steps for DarkHelp and Darknet which included installing vcpkg, run the following commands:

	cd c:\src\vcpkg
	vcpkg.exe install freetype:x64-windows
	cd c:\src
	git clone https://github.com/stephanecharette/DarkHelp.git
	cd darkhelp
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake ..
	cmake --build .
