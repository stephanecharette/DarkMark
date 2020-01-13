/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


void dm::find_files(File dir, VStr & image_filenames, VStr & json_filenames, std::atomic<bool> & done)
{
	image_filenames.clear();
	json_filenames.clear();

	Log("finding all images and markup files in " + dir.getFullPathName().toStdString());

	const std::regex image_filename_regex(cfg().get_str("image_regex"), std::regex::icase | std::regex::nosubs | std::regex::optimize | std::regex::ECMAScript);

	DirectoryIterator iter(dir, true, "*", File::findFiles + File::ignoreHiddenFiles);
	while (iter.next() && done == false)
	{
		File f = iter.getFile();
		const std::string filename = f.getFullPathName().toStdString();
		if (std::regex_match(filename, image_filename_regex))
		{
			// Why is it that sometimes darknet creates a file named "chart.png", and other times it gets complicated
			// and instead creates the file as "chart_<project>_yolov3[-tiny].png"?  Either way, ignore those chart*.png
			// files when running DarkMark.

			if (filename.find("chart.png") != std::string::npos)
			{
				continue;
			}

			if (filename.find("chart_"	) != std::string::npos and
				filename.find(".png"	) != std::string::npos)
			{
				continue;
			}

			image_filenames.push_back(filename);

			File json_file = f.withFileExtension(".json");
			if (json_file.existsAsFile())
			{
				json_filenames.push_back(json_file.getFullPathName().toStdString());
			}
		}
	}

	return;
}
