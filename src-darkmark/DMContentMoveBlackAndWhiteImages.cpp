// DarkMark (C) 2019-2026 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentMoveBlackAndWhiteImages::DMContentMoveBlackAndWhiteImages(dm::DMContent & c) :
		ThreadWithProgressWindow("Moving black-and-white images...", true, true),
		content(c)
{
	return;
}


dm::DMContentMoveBlackAndWhiteImages::~DMContentMoveBlackAndWhiteImages()
{
	return;
}


void dm::DMContentMoveBlackAndWhiteImages::run()
{
	DarkMarkApplication::setup_signal_handling();

	const auto previous_scrollfield_width = content.scrollfield_width;
	if (previous_scrollfield_width > 0)
	{
		content.scrollfield_width = 0;
		content.resized();
	}

	File dir = File(content.project_info.project_dir).getChildFile("black-and-white_images");
	dir.createDirectory();

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		std::string & fn = content.image_filenames.at(idx);
		File f1 = File(fn);

		if (f1.isAChildOf(dir))
		{
			// this file is already in the "black-and-white images" folder
			continue;
		}

		cv::Mat mat = cv::imread(f1.getFullPathName().toStdString());

		// if we're dealing with a BGRA image, first thing we must do is convert to BGR
		if (mat.channels() == 4)
		{
			// the real question is, why are you using an image with a transparency layer to train a neural network!?
			cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
		}

		bool must_move_file = false;
		if (mat.empty())
		{
			must_move_file = true;
		}
		else if (mat.channels() == 1)
		{
			must_move_file = true;
		}
		else if (mat.channels() == 3)
		{
			// use a portion from the center of the image, not the entire thing,
			// in case the image has a border, frame, or some sort of colour bar on the outside

			cv::Rect r;
			r.width		= mat.cols / 2;
			r.height	= mat.rows / 2;
			r.x			= mat.cols / 4;
			r.y			= mat.rows / 4;

			cv::Mat channels[3];
			cv::split(mat(r), channels);

			cv::Mat bg_diff;
			cv::Mat br_diff;
			cv::absdiff(channels[0], channels[1], bg_diff);
			cv::absdiff(channels[0], channels[2], br_diff);

			const double mean_bg = cv::mean(bg_diff)[0];
			const double mean_br = cv::mean(br_diff)[0];

			if (mean_bg <= 3.0 and
				mean_br <= 3.0)
			{
				// all channels are the same (or nearly the same)
				must_move_file = true;
			}

			Log(f1.getFullPathName().toStdString() +
				": move=" + (must_move_file ? "TRUE" : "false") +
				" m1="	+ std::to_string(mean_bg	) +
				" m2="	+ std::to_string(mean_br	) +
				" x="	+ std::to_string(r.x		) +
				" y="	+ std::to_string(r.y		) +
				" w="	+ std::to_string(r.width	) +
				" h="	+ std::to_string(r.height)	);
		}
		else
		{
			// don't know how many channels we're dealing with, but it isn't a number that we were expecting
		}

		if (must_move_file)
		{
			const std::vector<juce::File> v =
			{
				f1.withFileExtension(".json"),
				f1.withFileExtension(".txt"	),
				f1.getFullPathName()
			};

			std::string new_image_filename;
			for (auto & src : v)
			{
				if (not src.existsAsFile())
				{
					continue;
				}

				File dst = dir.getChildFile(src.getFileName());

				Log("moving " + src.getFullPathName().toStdString() + " to " + dst.getFullPathName().toStdString());

				src.moveFileTo(dst);

				new_image_filename = dst.getFullPathName().toStdString();
			}

			if (not new_image_filename.empty())
			{
				fn = new_image_filename;
			}
		}
	}

	content.scrollfield_width = previous_scrollfield_width;
	content.scrollfield.rebuild_entire_field_on_thread();

	return;
}
