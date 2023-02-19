// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// Get all of the image and .json markup files (recursively) for the given directory.
	void find_files(File dir, VStr & image_filenames, VStr & json_filenames, VStr & images_without_json, std::atomic<bool> & done);

	/// Used to generate random numbers.
	std::default_random_engine & get_random_engine();
}
