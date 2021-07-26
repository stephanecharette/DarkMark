// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentDeleteRotateAndFlipImages::DMContentDeleteRotateAndFlipImages(dm::DMContent & c) :
	ThreadWithProgressWindow("Delete rotate and flip images...", true, true),
	content(c)
{
	return;
}


dm::DMContentDeleteRotateAndFlipImages::~DMContentDeleteRotateAndFlipImages()
{
	return;
}


void dm::DMContentDeleteRotateAndFlipImages::run()
{
	DarkMarkApplication::setup_signal_handling();

	const auto previous_scrollfield_width = content.scrollfield_width;
	if (previous_scrollfield_width > 0)
	{
		content.scrollfield_width = 0;
		content.resized();
	}

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	VStr keep_filenames;

	size_t image_file_deleted	= 0;
	size_t json_file_deleted	= 0;
	size_t txt_file_deleted		= 0;

	for (const auto fn : content.image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		File f(fn);
		const String str = f.getFileNameWithoutExtension();
		if (str.endsWith("_r090") or
			str.endsWith("_r180") or
			str.endsWith("_r270") or
			str.endsWith("_fh") or
			str.endsWith("_fv"))
		{
			f.moveToTrash();
			image_file_deleted ++;

			f = f.withFileExtension(".txt");
			if (f.existsAsFile())
			{
				f.moveToTrash();
				txt_file_deleted ++;
			}

			f = f.withFileExtension(".json");
			if (f.existsAsFile())
			{
				f.moveToTrash();
				json_file_deleted ++;
			}
		}
		else
		{
			keep_filenames.push_back(fn);
		}
	}

	setStatusMessage("Sorting...");
	setProgress(1.1);
	std::sort(keep_filenames.begin(), keep_filenames.end());

	content.image_filenames.swap(keep_filenames);
	content.load_image(0);
	content.set_sort_order(ESort::kAlphabetical);

	content.scrollfield_width = previous_scrollfield_width;
	content.scrollfield.rebuild_entire_field_on_thread();

	if (keep_filenames.size() == content.image_filenames.size())
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", "No suitable images were found to delete.");
	}
	else
	{
		AlertWindow::showMessageBox(
			AlertWindow::AlertIconType::InfoIcon,
			"DarkMark",
			"Summary of file deletion:\n"
			"\n"
			"- Number of images deleted: "		+ std::to_string(image_file_deleted	) + "\n"
			"- Number of .txt files deleted: "	+ std::to_string(txt_file_deleted	) + "\n"
			"- Number of .json files deleted: "	+ std::to_string(json_file_deleted	));
	}

	return;
}
