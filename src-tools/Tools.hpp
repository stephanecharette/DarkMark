// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// Get all of the image and .json markup files (recursively) for the given directory.
	void find_files(File dir, VStr & image_filenames, VStr & json_filenames, std::atomic<bool> & done);
}
