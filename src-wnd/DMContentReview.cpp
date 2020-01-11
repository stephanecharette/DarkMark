/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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
}


void dm::DMContentReview::run()
{
	DarkMarkApplication::setup_signal_handling();

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	MMReviewInfo m;

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
			Log("failed to read image " + fn + " or parse json " + f.getFullPathName().toStdString());
			continue;
		}

		if (root["mark"].empty())
		{
			// nothing we can do with this file we don't have any marks defined
			continue;
		}

		for (auto mark : root["mark"])
		{
			if (threadShouldExit())
			{
				break;
			}

			const size_t class_idx = mark["class_idx"].get<size_t>();
			const int x = std::round(mat.cols * mark["rect"]["x"].get<double>());
			const int y = std::round(mat.rows * mark["rect"]["y"].get<double>());
			const int w = std::round(mat.cols * mark["rect"]["w"].get<double>());
			const int h = std::round(mat.rows * mark["rect"]["h"].get<double>());
			const cv::Rect r(x, y, w, h);

			// Check to see if the file type looks sane.  Especially when working with 3rd-party data sets, I've seen plenty of images
			// which are saved with .jpg extension, but which are actually .bmp, .gif, or .png.  (Though I'm not certain if this causes
			// problems when darknet uses opencv to load images...?)
			const std::string msg = magic_file(magic_cookie, fn.c_str());

			ReviewInfo review_info;
			review_info.class_idx = class_idx;
			review_info.filename = fn;
			review_info.mat = mat(r).clone();
			review_info.msg = msg;
			const size_t idx = m[class_idx].size();
			m[class_idx][idx] = review_info;
		}
	}

	magic_close(magic_cookie);

	if (threadShouldExit() == false)
	{
		Log("review map entries (classes found): " + std::to_string(m.size()));
		for (const auto iter : m)
		{
			const auto & class_idx = iter.first;
			const auto & mri = iter.second;
			Log("review map entries for class #" + std::to_string(class_idx) + ": " + std::to_string(mri.size()));
		}
	}


	if (not dmapp().review_wnd)
	{
		dmapp().review_wnd.reset(new DMReviewWnd(content));
	}
	dmapp().review_wnd->m.swap(m);
	dmapp().review_wnd->rebuild_notebook();
	dmapp().review_wnd->toFront(true);

	return;
}
