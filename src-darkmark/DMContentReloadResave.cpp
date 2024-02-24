// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentReloadResave::DMContentReloadResave(dm::DMContent & c) :
	ThreadWithProgressWindow("Re-saving the .json and .txt output files...", true, true),
	content(c)
{
	return;
}


dm::DMContentReloadResave::~DMContentReloadResave()
{
	return;
}


void dm::DMContentReloadResave::run()
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

	// disable all predictions
	const auto previous_predictions = content.show_predictions;
	content.show_predictions = EToggle::kOff;

	for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		const std::string fn = content.image_filenames.at(idx);
		File f1 = File(fn).withFileExtension(".json");
		File f2 = f1.withFileExtension(".txt");
		if (f1.existsAsFile() or f2.existsAsFile())
		{
			content.load_image(idx);
			content.need_to_save = true;
			// give the window time to repaint itself
			Thread::yield();
		}
	}

	content.scrollfield_width = previous_scrollfield_width;
	content.show_predictions = previous_predictions;
	content.load_image(0);
	content.scrollfield.rebuild_entire_field_on_thread();

	return;
}
