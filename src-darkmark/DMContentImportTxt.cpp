// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentImportTxt::DMContentImportTxt(dm::DMContent & c, const VStr & v) :
	ThreadWithProgressWindow(getText("TITLE2"), true, true, 10000, getText("Cancel")),
	content(c),
	image_filenames(v)
{
	return;
}


dm::DMContentImportTxt::~DMContentImportTxt()
{
	return;
}


void dm::DMContentImportTxt::run()
{
	DarkMarkApplication::setup_signal_handling();

	const auto previous_scrollfield_width = content.scrollfield_width;
	if (previous_scrollfield_width > 0)
	{
		content.scrollfield_width = 0;
		content.resized();
	}

	double max_work = image_filenames.size();
	double work_completed = 0.0;

	// for quick lookup, create a set with the images we need to find
	setStatusMessage(getText("Building image list..."));
	std::set<std::string> s;
	for (const auto & fn : image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;
		s.insert(fn);
	}
	image_filenames.clear();

	// disable all predictions so loading images goes faster
	const auto previous_predictions = content.show_predictions;
	content.show_predictions = EToggle::kOff;

	max_work = content.image_filenames.size();
	work_completed = 0.0;
	setStatusMessage(getText("Looking for images and annotations..."));
	for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
	{
		if (s.empty() or threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		const auto & fn = content.image_filenames[idx];
		if (s.count(fn) == 1)
		{
			// we found a match -- load this image to force the creation of the json file
			Log("importing annotations for " + fn);
			try
			{
				content.load_image(idx);
				content.need_to_save = true;
			}
			catch (const std::exception & e)
			{
				Log("exception caught while loading " + fn + ": " + e.what());
			}
		}
		s.erase(fn);
	}

	content.scrollfield_width = previous_scrollfield_width;
	content.show_predictions = previous_predictions;
	content.load_image(0);
	content.scrollfield.rebuild_entire_field_on_thread();

	return;
}
