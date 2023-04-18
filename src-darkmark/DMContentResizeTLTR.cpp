// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentResizeTLTR::DMContentResizeTLTR(dm::DMContent & c) :
	ThreadWithProgressWindow("Resize TL and TR labels...", true, true),
	content(c)
{
	return;
}


dm::DMContentResizeTLTR::~DMContentResizeTLTR()
{
	return;
}


void dm::DMContentResizeTLTR::run()
{
	DarkMarkApplication::setup_signal_handling();

	const cv::Size preferred_size(45, 45);

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

		File f = File(content.image_filenames[idx]).withFileExtension(".txt");
		if (not f.existsAsFile())
		{
			continue;
		}
		if (f.getSize() == 0)
		{
			continue;
		}

		content.load_image(idx);

		// see if this image has TL or TR annotations
		for (auto & mark : content.marks)
		{
			if (mark.is_prediction)
			{
				continue;
			}

			if (mark.class_idx != static_cast<size_t>(content.tl_name_index) and
				mark.class_idx != static_cast<size_t>(content.tr_name_index))
			{
				continue;
			}

			// if we get here, then we have a TL/TR mark

			const auto old_r = mark.get_bounding_rect(content.original_image.size());
			auto new_r = old_r;

			if (mark.class_idx == static_cast<size_t>(content.tr_name_index))
			{
				// for TR we need to move the left border, meaning the original X position
				new_r.x += new_r.width - preferred_size.width;
			}

			new_r.width		= preferred_size.width;
			new_r.height	= preferred_size.height;

			if (old_r != new_r)
			{
				std::stringstream ss;
				ss << content.short_filename << ": resizing " << mark.name << " from " << old_r << " to " << new_r;
				Log(ss.str());
				mark.set(new_r);
				content.need_to_save = true;
			}
		}
	}

	content.scrollfield_width = previous_scrollfield_width;
	content.show_predictions = previous_predictions;
	content.load_image(0);
	content.scrollfield.rebuild_entire_field_on_thread();

	return;
}
