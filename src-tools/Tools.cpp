// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


void dm::find_files(File dir, VStr & image_filenames, VStr & json_filenames, VStr & images_without_json, std::atomic<bool> & done)
{
	image_filenames.clear();
	json_filenames.clear();

	Log("finding all images and markup files in " + dir.getFullPathName().toStdString());

	const std::regex image_filename_regex(cfg().get_str("image_regex"), std::regex::icase | std::regex::nosubs | std::regex::optimize | std::regex::ECMAScript);

	for (auto dir_entry : RangedDirectoryIterator(dir, true))
	{
		if (done)
		{
			break;
		}

		File f = dir_entry.getFile();
		const std::string filename = f.getFullPathName().toStdString();
		if (std::regex_match(filename, image_filename_regex))
		{
			// Why is it that sometimes darknet creates a file named "chart.png", and other times it gets complicated
			// and instead creates the file as "chart_<project>_yolov3[-tiny].png"?  Either way, ignore those chart*.png
			// files when running DarkMark.

			if (filename.find("/chart.png") != std::string::npos)
			{
				continue;
			}

			if (filename.find("/chart_"	) != std::string::npos and
				filename.find(".png"	) != std::string::npos)
			{
				continue;
			}

			if (filename.find("/darkmark_image_cache/") != std::string::npos)
			{
				continue;
			}

			image_filenames.push_back(filename);

			File json_file = f.withFileExtension(".json");
			if (json_file.existsAsFile())
			{
				json_filenames.push_back(json_file.getFullPathName().toStdString());
			}
			else
			{
				File txt_file = f.withFileExtension(".txt");
				if (txt_file.existsAsFile())
				{
					images_without_json.push_back(filename);
				}
			}
		}
	}

	return;
}
