/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMContentRotateImages::DMContentRotateImages(dm::DMContent & c, const bool all_images) :
	ThreadWithProgressWindow("Rotate images 90, 180, and 270 degrees...", true, true),
	content(c),
	apply_to_all_images(all_images)
{
	return;
}


dm::DMContentRotateImages::~DMContentRotateImages()
{
	return;
}


void dm::DMContentRotateImages::run()
{
	DarkMarkApplication::setup_signal_handling();

	// briefly pause here so the window can properly redraw itself (hack?)
	getAlertWindow()->repaint();
	sleep(250); // milliseconds

	// make a set of all filenames so we can quickly look up if an image already exists
	SStr all_filenames;
	for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
	{
		if (threadShouldExit())
		{
			break;
		}

		all_filenames.insert(File(content.image_filenames.at(idx)).getFileNameWithoutExtension().toStdString());
	}

	// disable all predictions so the images load faster
	const auto previous_predictions = content.show_predictions;
	content.show_predictions = EToggle::kOff;

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	size_t images_added = 0;
	size_t images_skipped = 0;

	// we're going to be growing the vector of filenames with new images pushed to the back of the vector, but we don't
	// want to re-process those new images, so we must remember the max index
	const size_t original_max_idx = content.image_filenames.size();

	for (size_t idx = 0; idx < original_max_idx; idx ++)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		File original_file(content.image_filenames.at(idx));
		const String original_fn = original_file.getFileNameWithoutExtension();
		if (original_fn.endsWith("_r090") or
			original_fn.endsWith("_r180") or
			original_fn.endsWith("_r270"))
		{
			// this file is the result of a previous rotation, so skip it
			images_skipped ++;
			continue;
		}

		// load the given image so we can get access to the cv::Mat
		content.load_image(idx);
		if (content.original_image.empty() or
			content.original_image.cols < 1 or
			content.original_image.rows < 1)
		{
			// something is wrong with this image
			images_skipped ++;
			continue;
		}

		// remember the image and the markup for this image, because we're going to have to rotate all the points
		const auto original_marks = content.marks;
		cv::Mat original_mat = content.original_image;

		if (apply_to_all_images == false and (content.image_is_completely_empty == false and original_marks.empty()))
		{
			// image is not marked up, so don't bother to rotate it
			images_skipped ++;
			continue;
		}

		for (const auto rotation : {cv::ROTATE_90_CLOCKWISE, cv::ROTATE_180, cv::ROTATE_90_COUNTERCLOCKWISE})
		{
			if (threadShouldExit())
			{
				break;
			}

			const auto new_fn = original_fn + "_r" + (rotation == cv::ROTATE_90_CLOCKWISE ? "090" : rotation == cv::ROTATE_180 ? "180" : "270");
			if (all_filenames.count(new_fn.toStdString()) > 0)
			{
				// this rotated image already exists
				images_skipped ++;
				continue;
			}

			// figure out the full path of the new image
			const std::string new_full_fn = original_file.getSiblingFile(new_fn + ".jpg").getFullPathName().toStdString();
			Log("creating new rotated image " + new_full_fn);

			// rotate and save the image to disk
			cv::Mat dst;
			cv::rotate(original_mat, dst, rotation);
			cv::imwrite(new_full_fn, dst, {cv::IMWRITE_JPEG_QUALITY, 75, cv::IMWRITE_JPEG_OPTIMIZE, 1});

			// note: images will no longer be sorted correctly!
			content.image_filenames.push_back(new_full_fn);
			images_added ++;

			if (original_marks.empty() == false or content.image_is_completely_empty)
			{
				// re-create the markup for this newly rotated image by saving a simple .txt file,
				// and then we'll get DarkMark to reload this image which should cause the .json output

				const double degrees	= (rotation == cv::ROTATE_90_CLOCKWISE ? 90.0 : rotation == cv::ROTATE_180 ? 180.0 : 270.0);
				const double rads		= degrees * M_PI / 180.0;
				const double s			= std::sin(rads);
				const double c			= std::cos(rads);

				auto txt_file = File(new_full_fn).withFileExtension(".txt");
				std::ofstream fs(txt_file.getFullPathName().toStdString());

				for (const auto & m : original_marks)
				{
					if (m.is_prediction)
					{
						continue;
					}

					const cv::Rect2d r	= m.get_normalized_bounding_rect();
					const double w		= r.width;
					const double h		= r.height;
					const double x		= r.x + r.width / 2.0 - 0.5; // -0.5 to bring it back to the origin
					const double y		= r.y + r.height / 2.0 - 0.5; // -0.5 to bring it back to the origin

					const double new_w		= (degrees == 180.0 ? w : h);
					const double new_h		= (degrees == 180.0 ? h : w);
					const double new_x		= x * c - y * s + 0.5;
					const double new_y		= x * s + y * c + 0.5;

					fs << std::fixed << std::setprecision(10) << m.class_idx << " " << new_x << " " << new_y << " " << new_w << " " << new_h << std::endl;
				}

				// load the new images to force DarkMark to create the .json file from the .txt file
				content.load_image(content.image_filenames.size() - 1);
			}
		}
	}

	content.show_predictions = previous_predictions;

	if (threadShouldExit() == false)
	{
		content.load_image(0);

		std::stringstream ss;
		ss << "Rotation tool ";
		if (images_skipped > 0)
		{
			ss << "skipped " << images_skipped << " existing image" << (images_skipped == 1 ? "" : "s") << " and ";
		}
		ss << "added " << images_added << " new image" << (images_added == 1 ? "" : "s") << ".";
		if (images_added)
		{
			ss << std::endl << std::endl << "Additions are inserted at the end of the list of images.  You may want to re-apply the filename sort.";
		}

		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
	}

	return;
}
