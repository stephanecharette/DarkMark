// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"

#include <magic.h>

#include "json.hpp"
using json = nlohmann::json;


dm::DMContentReview::DMContentReview(dm::DMContent & c) :
	ThreadWithProgressWindow("Building image index...", true, true),
	content(c)
{
	return;
}


dm::DMContentReview::~DMContentReview()
{
	return;
}


void dm::DMContentReview::run()
{
	DarkMarkApplication::setup_signal_handling();

	const bool resize_thumbnails = cfg().get_bool("review_resize_thumbnails");
	const int row_height = cfg().get_int("review_table_row_height");
	const cv::Size desired_size(9999, row_height);

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	MMReviewInfo m;
	MStrSize md5s;

	// last index in the names vector will be to store "errors"
	const size_t error_index = content.names.size();

	magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic_cookie, nullptr);

	for (const auto & fn : content.image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		File f = File(fn).withFileExtension(".json");
		if (f.existsAsFile() == false)
		{
			// nothing we can do with this file since we don't have a corresponding .json
			continue;
		}

		json root;
		cv::Mat mat;
		try
		{
			root = json::parse(f.loadFileAsString().toStdString());
			mat = cv::imread(fn);
		}
		catch(const std::exception & e)
		{
			Log("failed to read image " + fn + " or parse json " + f.getFullPathName().toStdString() + ": " + e.what());
			const auto class_idx = error_index;
			ReviewInfo review_info;
			review_info.overlap_sum	= 0.0;
			review_info.class_idx	= class_idx;
			review_info.filename	= fn;
			review_info.mat			= cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 255)); // use a red square to indicate a problem
			review_info.errors.push_back(e.what()); // default error msg, but then see if we can provide something more specific
			if (root.empty())
			{
				review_info.errors.push_back("error reading json file " + f.getFullPathName().toStdString());
			}
			else if (mat.empty())
			{
				review_info.errors.push_back("error reading image file " + fn);
			}
			const size_t idx = m[class_idx].size();
			m[class_idx][idx] = review_info;
			continue;
		}

		if (mat.empty())
		{
			Log("failed to load image " + fn);
			const auto class_idx = error_index;
			ReviewInfo review_info;
			review_info.overlap_sum	= 0.0;
			review_info.class_idx	= class_idx;
			review_info.filename	= fn;
			review_info.mat			= cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 255)); // use a red square to indicate a problem
			review_info.errors.push_back("failed to load image");
			const size_t idx		= m[class_idx].size();
			m[class_idx][idx]		= review_info;
			continue;
		}

		const auto md5 = MD5(mat.data, mat.step[0] * mat.rows).toHexString().toStdString();
		md5s[md5] ++;

		if (root["mark"].empty() and root.value("completely_empty", false))
		{
			const auto class_idx = content.empty_image_name_index;
			ReviewInfo review_info;
			review_info.overlap_sum = 0.0;
			review_info.class_idx = class_idx;
			review_info.filename = fn;
			review_info.mime_type = magic_file(magic_cookie, fn.c_str());
			review_info.r = cv::Rect(0, 0, mat.cols, mat.rows);
			review_info.md5 = md5;

			// full-size images are always resized
			review_info.mat = DarkHelp::resize_keeping_aspect_ratio(mat, desired_size);

			const size_t idx = m[class_idx].size();
			m[class_idx][idx] = review_info;
			continue;
		}

		if (root["mark"].empty())
		{
			// nothing we can do with this file we don't have any marks defined
			Log("no marks defined, yet image is not marked as empty: " + fn);
			const auto class_idx = error_index;
			ReviewInfo review_info;
			review_info.overlap_sum = 0.0;
			review_info.class_idx = class_idx;
			review_info.filename = fn;
			review_info.md5 = md5;
			review_info.mat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 255)); // use a red square to indicate a problem
			review_info.errors.push_back("no marks defined, yet image is not marked as empty");
			const size_t idx = m[class_idx].size();
			m[class_idx][idx] = review_info;
			continue;
		}

		// first we need to get all the rectangles (marks) and make a list of them so we can eventually calculate the overlapping regions
		std::vector<cv::Rect> all_rectangles;
		for (auto mark : root["mark"])
		{
			if (threadShouldExit())
			{
				break;
			}

			const int x = std::round(mat.cols * mark["rect"]["x"].get<double>());
			const int y = std::round(mat.rows * mark["rect"]["y"].get<double>());
			const int w = std::round(mat.cols * mark["rect"]["w"].get<double>());
			const int h = std::round(mat.rows * mark["rect"]["h"].get<double>());
			const cv::Rect r(x, y, w, h);

			all_rectangles.push_back(r);
		}

		// This image may need to be resized for the neural network.  Figure out the exact factor by which the image
		// will be resized so we can determine if individual marks will be too small.
		const double network_width	= content.project_info.image_width;
		const double network_height	= content.project_info.image_height;
		const double image_width	= mat.cols;
		const double image_height	= mat.rows;
		const double scale_x		= network_width / image_width;
		const double scale_y		= network_height / image_height;

		// now go through all the marks *again*
		for (auto mark : root["mark"])
		{
			if (threadShouldExit())
			{
				break;
			}

			size_t class_idx = mark["class_idx"].get<size_t>();
			const int x = std::round(mat.cols * mark["rect"]["x"].get<double>());
			const int y = std::round(mat.rows * mark["rect"]["y"].get<double>());
			const int w = std::round(mat.cols * mark["rect"]["w"].get<double>());
			const int h = std::round(mat.rows * mark["rect"]["h"].get<double>());
			const cv::Rect r1(x, y, w, h);

			ReviewInfo review_info;
			review_info.r = r1;
			review_info.overlap_sum = 0.0;
			review_info.class_idx = class_idx;
			review_info.filename = fn;
			review_info.md5 = md5;

			// Check to see if the file type looks sane.  Especially when working with 3rd-party data sets, I've seen plenty of images
			// which are saved with .jpg extension, but which are actually .bmp, .gif, or .png.  (Though I'm not certain if this causes
			// problems when darknet uses opencv to load images...?)
			review_info.mime_type = magic_file(magic_cookie, fn.c_str());
			if (review_info.mime_type != "image/jpeg" and review_info.mime_type != "image/png")
			{
				// looks like something about this image is different than what we'd normally expect
				if (review_info.mime_type.find("image/") == std::string::npos)
				{
					review_info.errors.push_back("not an image");
				}
				else
				{
					review_info.warnings.push_back("unusual image type");
				}
			}

			try
			{
				cv::Mat roi = mat(r1);
				if (resize_thumbnails or roi.rows > row_height)
				{
					review_info.mat = DarkHelp::resize_keeping_aspect_ratio(roi, desired_size).clone();
				}
				else
				{
					review_info.mat = roi.clone();
				}
			}
			catch (...)
			{
				Log(content.names[class_idx] + ": encountered a problem trying to get the ROI from " + fn);
				review_info.mat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 255)); // use a red square to indicate a problem
				review_info.errors.push_back("error reading image or region of interest; maybe try to delete and re-create the mark?");
				class_idx = error_index;
			}

			// now compare this rectangle against all other rectangles in this image to see if there is any overlap
			for (const auto & r2 : all_rectangles)
			{
				// so now we have r1 and r2, and since we're looping over "all_rectangles" at some
				// point r1 == r2 which we'll need to take into account when we calculate the sum

				// see: https://stackoverflow.com/questions/9324339/how-much-do-two-rectangles-overlap/9325084
				const auto tl1 = r1.tl();	// blue_triangle
				const auto tl2 = r2.tl();	// orange_triangle
				const auto br1 = r1.br();	// blue_circle
				const auto br2 = r2.br();	// orange_circle

				const double intersecting_area = std::max(0, std::min(br2.x, br1.x) - std::max(tl2.x, tl1.x)) * std::max(0, std::min(br2.y, br1.y) - std::max(tl2.y, tl1.y));
				const double a1 = r1.area();
				const double a2 = r2.area();
				const double intersection_over_union = intersecting_area / std::max(1.0, (a1 + std::min(a2, intersecting_area) - intersecting_area));
				review_info.overlap_sum += intersection_over_union;
//				Log(fn + ": intersection_over_union=" + std::to_string(intersection_over_union));
			}

			if (review_info.overlap_sum >= 1.0)
			{
				// we don't care about the overlap we have with ourself (which is exactly 1.0) so subtract that from the total
				review_info.overlap_sum -= 1.0;

				if (review_info.overlap_sum >= 0.1) // meaning >= 10%
				{
					review_info.warnings.push_back("overlap (intersection over union) seems high");
//					Log(fn + ": overlap (intersection over union) is " + std::to_string(review_info.overlap_sum));
				}
			}

			const double scaled_width = scale_x * w;
			const double scaled_height = scale_y * h;
			if (scaled_width < 16.0 or scaled_height < 16.0)
			{
				review_info.warnings.push_back("scaled mark measuring " + std::to_string((int)scaled_width) + "x" + std::to_string((int)scaled_height) + " may be too small to detect");
//				Log(fn + ": scaled mark measures " + std::to_string((int)scaled_width) + "x" + std::to_string((int)scaled_height));
			}

			const size_t idx = m[class_idx].size();
			m[class_idx][idx] = review_info;
		}
	}

	magic_close(magic_cookie);

	if (threadShouldExit() == false)
	{
		Log("review map entries (classes found): " + std::to_string(m.size()));
		for (const auto & iter : m)
		{
			const auto & class_idx = iter.first;
			const auto & mri = iter.second;
			Log("review map entries for class #" + std::to_string(class_idx) + ": " + std::to_string(mri.size()));
		}
	}


	if (not dmapp().review_wnd)
	{
		dmapp().review_wnd.reset(new DMReviewWnd(content));

		// start with JUCE 7, setting the always-on-top flag before the window has been fully created and displayed
		// seems to cause a segfault deep inside JUCE and X
//		dmapp().review_wnd->setAlwaysOnTop(true);
	}
	dmapp().review_wnd->m.swap(m);
	dmapp().review_wnd->md5s.swap(md5s);
	dmapp().review_wnd->rebuild_notebook();
	dmapp().review_wnd->toFront(true);

	return;
}
